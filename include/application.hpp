#if defined(__SWITCH__)
// Deko3D Framework Headers
#include <nanovg/framework/CApplication.h>
#include <nanovg/framework/CMemPool.h>
#include <nanovg_dk.h>

#include "hello.h"
#include "perf.h"

namespace
{
	static constexpr unsigned NumFramebuffers = 2;
	static constexpr unsigned StaticCmdSize = 0x1000;
}

class HelloWorldDkApp final : public CApplication
{
public:
	HelloWorldDkApp(const AudioRendererConfig *ar_config);
	~HelloWorldDkApp();

private:
	static constexpr uint32_t FramebufferWidth = 1280;
	static constexpr uint32_t FramebufferHeight = 720;

	dk::UniqueDevice m_device;
	dk::UniqueQueue m_queue;
	dk::UniqueSwapchain m_swapchain;

	std::optional<CMemPool> m_pool_images;
	std::optional<CMemPool> m_pool_audio;
	std::optional<CMemPool> m_pool_code;
	std::optional<CMemPool> m_pool_data;

	dk::UniqueCmdBuf m_cmdbuf;
	DkCmdList m_render_cmdlist;

	dk::Image m_depthBuffer;
	CMemPool::Handle m_depthBuffer_mem;
	dk::Image m_framebuffers[NumFramebuffers];
	CMemPool::Handle m_framebuffers_mem[NumFramebuffers];
	DkCmdList m_framebuffer_cmdlists[NumFramebuffers];
	CMemPool::Handle m_audioBuffer;

	AudioDriverWaveBuf wavebuf;
	AudioDriver drv;

	std::optional<nvg::DkRenderer> m_renderer;
	NVGcontext* m_vg;

	HelloWorldScreenData m_data;
	PerfGraph m_fps;
	float m_prevTime;
	int m_standard_font;
	PadState m_pad;

	void createFramebufferResources();
	void destroyFramebufferResources();
	void recordStaticCommands();
	void render(u64 ns, int blowup);

protected:
	bool onFrame(u64 ns) override;
};
#endif /* __SWITCH__ */