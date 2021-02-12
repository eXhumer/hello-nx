#if defined(__SWITCH__)
#include "application.hpp"

#if defined(DEBUG_NXLINK)
static int nxlink_sock = -1;
#endif /* DEBUG_NXLINK */

static const AudioRendererConfig audren_config =
{
	.output_rate     = AudioRendererOutputRate_48kHz,
	.num_voices      = 24,
	.num_effects     = 0,
	.num_sinks       = 1,
	.num_mix_objs    = 1,
	.num_mix_buffers = 2,
};

static const SocketInitConfig socket_config =
{
    .bsdsockets_version = 1,
    .tcp_tx_buf_size = 0x8000,
    .tcp_rx_buf_size = 0x10000,
    .tcp_tx_buf_max_size = 0x40000,
    .tcp_rx_buf_max_size = 0x40000,
    .udp_tx_buf_size = 0x2400,
    .udp_rx_buf_size = 0xA500,
    .sb_efficiency = 4,
    .num_bsd_sessions = 3,
    .bsd_service_type = BsdServiceType_User,
};

extern "C" void userAppInit(void)
{
	Result res = romfsInit();
	if (R_FAILED(res))
		diagAbortWithResult(res);

	if (R_FAILED(res = plInitialize(PlServiceType_User)))
		diagAbortWithResult(res);

	if (R_FAILED(res = audrenInitialize(&audren_config)))
		diagAbortWithResult(res);

	if (R_FAILED(res = socketInitialize(&socket_config)))
		diagAbortWithResult(res);

#if defined(DEBUG_NXLINK)
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
#endif /* DEBUG_NXLINK */

	socketExit();
	audrenExit();
	plExit();
	romfsExit();
}

int main(int argc, char* argv[])
{
	HelloWorldDkApp app(&audren_config);
	app.run();
	return 0;
}
#endif /* __SWITCH__ */
