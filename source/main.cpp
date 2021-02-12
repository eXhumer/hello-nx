#if defined(__SWITCH__)
#include "application.hpp"

#if defined(DEBUG_NXLINK)
static int nxlink_sock = -1;
#endif /* DEBUG_NXLINK */

static const AudioRendererConfig ar_config =
{
	.output_rate     = AudioRendererOutputRate_48kHz,
	.num_voices      = 24,
	.num_effects     = 0,
	.num_sinks       = 1,
	.num_mix_objs    = 1,
	.num_mix_buffers = 2,
};

extern "C" void userAppInit(void)
{
	Result res = romfsInit();
	if (R_FAILED(res))
		diagAbortWithResult(res);

	if (R_FAILED(res = plInitialize(PlServiceType_User)))
		diagAbortWithResult(res);

	if (R_FAILED(res = audrenInitialize(&ar_config)))
		diagAbortWithResult(res);

#if defined(DEBUG_NXLINK)
	if (R_FAILED(res = socketInitializeDefault()))
		diagAbortWithResult(res);
	nxlink_sock = nxlinkStdioForDebug();
	if (nxlink_sock > 0)
		printf("Connected with NXLink Client!%s");
	else
		; // NXLink failed
#endif /* DEBUG_NXLINK */
}

extern "C" void userAppExit(void)
{
#if defined(DEBUG_NXLINK)
	if (nxlink_sock != -1)
		close(nxlink_sock);
	socketExit();
#endif /* DEBUG_NXLINK */

	audrenExit();
	plExit();
	romfsExit();
}

// Main entrypoint
int main(int argc, char* argv[])
{
	HelloWorldDkApp app(&ar_config);
	app.run();
	return 0;
}
#endif /* __SWITCH__ */
