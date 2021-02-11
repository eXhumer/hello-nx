// Deko3D Framework Headers
#include <nanovg/framework/CApplication.h>
#include <nanovg/framework/CMemPool.h>
// C++ Standard Library Headers
#include <array>
#include <optional>
#include <unistd.h>
// nanovg Headers
#include <nanovg.h>
#include <nanovg_dk.h>

// #include "demo.h"
#include "perf.h"
#include "hello.h"

#if !defined(USE_OPENGL_GLFW) && !defined(USE_OPENGL_EGL)

//#define DEBUG_NXLINK

#ifdef DEBUG_NXLINK
static int nxlink_sock = -1;
#endif

extern "C" void userAppInit(void)
{
	Result res = romfsInit();
	if (R_FAILED(res))
		diagAbortWithResult(res);

	if (R_FAILED(res = plInitialize(PlServiceType_User)))
		diagAbortWithResult(res);


#ifdef DEBUG_NXLINK
	socketInitializeDefault();
	nxlink_sock = nxlinkStdioForDebug();
#endif
}

extern "C" void userAppExit(void)
{
#ifdef DEBUG_NXLINK
	if (nxlink_sock != -1)
		close(nxlink_sock);
	socketExit();
#endif

	plExit();
	romfsExit();
}

void OutputDkDebug(void* userData, const char* context, DkResult result, const char* message) 
{
	printf("Context: %s\nResult: %d\nMessage: %s\n", context, result, message);
}

namespace
{
	static constexpr unsigned NumFramebuffers = 2;
	static constexpr unsigned StaticCmdSize = 0x1000;
}

class HelloWorldApp final : public CApplication
{
private:
	static constexpr uint32_t FramebufferWidth = 1280;
	static constexpr uint32_t FramebufferHeight = 720;

	dk::UniqueDevice m_device;
	dk::UniqueQueue m_queue;
	dk::UniqueSwapchain m_swapchain;

	std::optional<CMemPool> m_pool_images;
	std::optional<CMemPool> m_pool_code;
	std::optional<CMemPool> m_pool_data;

	dk::UniqueCmdBuf m_cmdbuf;
	DkCmdList m_render_cmdlist;

	dk::Image m_depthBuffer;
	CMemPool::Handle m_depthBuffer_mem;
	dk::Image m_framebuffers[NumFramebuffers];
	CMemPool::Handle m_framebuffers_mem[NumFramebuffers];
	DkCmdList m_framebuffer_cmdlists[NumFramebuffers];

	std::optional<nvg::DkRenderer> m_renderer;
	NVGcontext* m_vg;

	int m_standard_font;
	HelloWorldScreenData m_data;
	PerfGraph m_fps;
	float m_prevTime;
	PadState m_pad;

public:
	HelloWorldApp()
	{
		// Create the deko3d device
		this->m_device = dk::DeviceMaker{}.setCbDebug(OutputDkDebug).create();

		// Create the main queue
		this->m_queue = dk::QueueMaker{this->m_device}.setFlags(DkQueueFlags_Graphics).create();

		// Create the memory pools
		this->m_pool_images.emplace(this->m_device, DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image, 16*1024*1024);
		this->m_pool_code.emplace(this->m_device, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached | DkMemBlockFlags_Code, 128*1024);
		this->m_pool_data.emplace(this->m_device, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached, 1*1024*1024);

		// Create the static command buffer and feed it freshly allocated memory
		this->m_cmdbuf = dk::CmdBufMaker{this->m_device}.create();
		CMemPool::Handle cmdmem = this->m_pool_data->allocate(StaticCmdSize);
		this->m_cmdbuf.addMemory(cmdmem.getMemBlock(), cmdmem.getOffset(), cmdmem.getSize());

		// Create the framebuffer resources
		this->createFramebufferResources();

		this->m_renderer.emplace(FramebufferWidth, FramebufferHeight, this->m_device, this->m_queue, *this->m_pool_images, *this->m_pool_code, *this->m_pool_data);
		this->m_vg = nvgCreateDk(&*this->m_renderer, NVG_ANTIALIAS | NVG_STENCIL_STROKES);

		initGraph(&m_fps, GRAPH_RENDER_FPS, "Frame Time");
		initHelloWorld(&this->m_data);

		padConfigureInput(1, HidNpadStyleSet_NpadStandard);
		padInitializeDefault(&this->m_pad);

		Result rc;
		PlFontData font;
		if (R_FAILED(rc = plGetSharedFontByType(&font, PlSharedFontType_Standard)))
			diagAbortWithResult(rc);

		this->m_standard_font = nvgCreateFontMem(this->m_vg, "switch-standard", static_cast<u8*>(font.address), font.size, 0);
	}

	~HelloWorldApp()
	{
		// Destroy the framebuffer resources. This should be done first.
		this->destroyFramebufferResources();

		// Cleanup vg. This needs to be done first as it relies on the renderer.
		nvgDeleteDk(this->m_vg);

		// Destroy the renderer
		this->m_renderer.reset();
	}

	void createFramebufferResources()
	{
	  // Create layout for the depth buffer
		dk::ImageLayout layout_depthbuffer;
		dk::ImageLayoutMaker{this->m_device}
			.setFlags(DkImageFlags_UsageRender | DkImageFlags_HwCompression)
			.setFormat(DkImageFormat_S8)
			.setDimensions(FramebufferWidth, FramebufferHeight)
			.initialize(layout_depthbuffer);

		// Create the depth buffer
		this->m_depthBuffer_mem = this->m_pool_images->allocate(layout_depthbuffer.getSize(), layout_depthbuffer.getAlignment());
		this->m_depthBuffer.initialize(layout_depthbuffer, this->m_depthBuffer_mem.getMemBlock(), this->m_depthBuffer_mem.getOffset());

		// Create layout for the framebuffers
		dk::ImageLayout layout_framebuffer;
		dk::ImageLayoutMaker{this->m_device}
			.setFlags(DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_HwCompression)
			.setFormat(DkImageFormat_RGBA8_Unorm)
			.setDimensions(FramebufferWidth, FramebufferHeight)
			.initialize(layout_framebuffer);

		// Create the framebuffers
		std::array<DkImage const*, NumFramebuffers> fb_array;
		uint64_t fb_size  = layout_framebuffer.getSize();
		uint32_t fb_align = layout_framebuffer.getAlignment();
		for (unsigned i = 0; i < NumFramebuffers; i ++)
		{
			// Allocate a framebuffer
			this->m_framebuffers_mem[i] = this->m_pool_images->allocate(fb_size, fb_align);
			this->m_framebuffers[i].initialize(layout_framebuffer, this->m_framebuffers_mem[i].getMemBlock(), this->m_framebuffers_mem[i].getOffset());

			// Generate a command list that binds it
			dk::ImageView colorTarget{ this->m_framebuffers[i] }, depthTarget{ this->m_depthBuffer };
			this->m_cmdbuf.bindRenderTargets(&colorTarget, &depthTarget);
			this->m_framebuffer_cmdlists[i] = this->m_cmdbuf.finishList();

			// Fill in the array for use later by the swapchain creation code
			fb_array[i] = &this->m_framebuffers[i];
		}

		// Create the swapchain using the framebuffers
		this->m_swapchain = dk::SwapchainMaker{this->m_device, nwindowGetDefault(), fb_array}.create();

		// Generate the main rendering cmdlist
		this->recordStaticCommands();
	}

	void destroyFramebufferResources()
	{
		// Return early if we have nothing to destroy
		if (!this->m_swapchain) return;

		// Make sure the queue is idle before destroying anything
		this->m_queue.waitIdle();

		// Clear the static cmdbuf, destroying the static cmdlists in the process
		this->m_cmdbuf.clear();

		// Destroy the swapchain
		this->m_swapchain.destroy();

		// Destroy the framebuffers
		for (unsigned i = 0; i < NumFramebuffers; i ++)
			this->m_framebuffers_mem[i].destroy();

		// Destroy the depth buffer
		this->m_depthBuffer_mem.destroy();
	}

	void recordStaticCommands()
	{
		// Initialize state structs with deko3d defaults
		dk::RasterizerState rasterizerState;
		dk::ColorState colorState;
		dk::ColorWriteState colorWriteState;
		dk::BlendState blendState;

		// Configure the viewport and scissor
		this->m_cmdbuf.setViewports(0, { { 0.0f, 0.0f, FramebufferWidth, FramebufferHeight, 0.0f, 1.0f } });
		this->m_cmdbuf.setScissors(0, { { 0, 0, FramebufferWidth, FramebufferHeight } });

		// Clear the color and depth buffers
		this->m_cmdbuf.clearColor(0, DkColorMask_RGBA, 0.2f, 0.3f, 0.3f, 1.0f);
		this->m_cmdbuf.clearDepthStencil(true, 1.0f, 0xFF, 0);

		// Bind required state
		this->m_cmdbuf.bindRasterizerState(rasterizerState);
		this->m_cmdbuf.bindColorState(colorState);
		this->m_cmdbuf.bindColorWriteState(colorWriteState);

		this->m_render_cmdlist = this->m_cmdbuf.finishList();
	}

	void render(u64 ns, int blowup)
	{
		float time = ns / 1000000000.0;
		float dt = time - this->m_prevTime;
		this->m_prevTime = time;

		// Acquire a framebuffer from the swapchain (and wait for it to be available)
		int slot = this->m_queue.acquireImage(this->m_swapchain);

		// Run the command list that attaches said framebuffer to the queue
		this->m_queue.submitCommands(this->m_framebuffer_cmdlists[slot]);

		// Run the main rendering command list
		this->m_queue.submitCommands(this->m_render_cmdlist);

		updateGraph(&this->m_fps, dt);

		nvgBeginFrame(this->m_vg, FramebufferWidth, FramebufferHeight, 1.0f);
		{
			// Render stuff!
			renderHelloWorld(this->m_vg, FramebufferWidth, FramebufferHeight, 0, 0, &this->m_data);
			renderGraph(this->m_vg, 5,5, &this->m_fps);
		}
		nvgEndFrame(this->m_vg);

		// Now that we are done rendering, present it to the screen
		this->m_queue.presentImage(this->m_swapchain, slot);
	}

	bool onFrame(u64 ns) override
	{
		padUpdate(&this->m_pad);
		u64 kDown = padGetButtonsDown(&this->m_pad);
		if (kDown & KEY_PLUS)
			return false;

		// hidKeysHeld alternate not provided with libnx v4.0.0 +
		// using kDown instead. Renders for a single frame when pressed
		render(ns, kDown & KEY_MINUS);
		return true;
	}
};

// Main entrypoint
int main(int argc, char* argv[])
{
	HelloWorldApp app;
	app.run();
	return 0;
}
#endif