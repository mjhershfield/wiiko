#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <debug.h>
#include <errno.h>
#include <network.h>
#include <wiiuse/wpad.h>
#include <ogc/system.h>
#include <vector>
#include "Game.h"
#include "Server.h"

void *xfb = NULL;
GXRModeObj *rmode = NULL;

void *initialize_gfx();

lwp_t httd_handle = (lwp_t)NULL;
Game game;

int main(int argc, char **argv)
{
	s32 ret;

	srand(SYS_Time());

	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};

	xfb = initialize_gfx();

	std::cout << "Configuring network ..." << std::endl;

	// Configure the network interface
	ret = if_config(localip, netmask, gateway, TRUE, 20);
	if (ret >= 0)
	{
		std::cout << "network configured, ip: " << localip << std::endl;

		LWP_CreateThread(&httd_handle, /* thread handle */
						 Server::server_entrypoint,		   /* code */
						 (void*) &game,	   /* arg pointer for thread */
						 NULL,		   /* stack base */
						 16 * 1024,	   /* stack size */
						 50 /* thread priority */);
	}
	else
	{
		std::cout << "network configuration failed!" << std::endl;
	}

	while (1)
	{

		VIDEO_WaitVSync();
		WPAD_ScanPads();

		uint32_t buttonsDown = WPAD_ButtonsDown(0);

		if (buttonsDown & WPAD_BUTTON_HOME)
		{
			exit(0);
		}

		while(LWP_MutexLock(game.mutex));
		// TODO: Be able to get time left on graphics thread
		game.check_next_state_timeout();
		LWP_MutexUnlock(game.mutex);

	}

	return 0;
}

void *initialize_gfx()
{
	void *framebuffer;

	VIDEO_Init();
	WPAD_Init();

	rmode = VIDEO_GetPreferredMode(NULL);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	return framebuffer;
}
