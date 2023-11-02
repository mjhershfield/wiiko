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
#include <ogc/lwp_watchdog.h>
#include <vector>
#include <fat.h>
#include <dirent.h>
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

	settime(0);

	// TODO: SEED RNG WITH SOMETHING ACTUALLY RANDOM LIKE gettime(). SYS_Time returns 0.
	srand(SYS_Time());

	xfb = initialize_gfx();

	std::cout << "Initializing filesystem" << std::endl;
	if (!(ret = fatInitDefault()))
	{
		std::cout << "error initializing filesystem: " << ret << std::endl;
		while (1);
	}

	std::cout << "Filesystem initialized!" << std::endl;

	std::cout << "Clearing temp folder" << std::endl;
	DIR* tmp_dir = opendir("sd://apps/wiiko/tmp/");
	if (tmp_dir == NULL)
	{
		std::cout << "could not open tmp dir" << std::endl;
		while(1);
	}

	while (1)
	{
		dirent* tmp_ent = readdir(tmp_dir);
		if (tmp_ent == NULL) break;

		if(strcmp(".", tmp_ent->d_name) != 0 && strcmp("..", tmp_ent->d_name) != 0)
		{
			char pathbuf[300];
			sprintf(pathbuf, "sd://apps/wiiko/tmp/%s", tmp_ent->d_name);
			if (tmp_ent->d_type == DT_REG)
			{
				if (!remove(pathbuf))
					printf("Deleted %s\n", pathbuf);
				else
					printf("Failed to delete %s\n", pathbuf);
			}
			else if (tmp_ent->d_type == DT_DIR)
			{
				printf("HOMIE IS A DIRECTORY ^^\n");
			}
		}
	}

	closedir(tmp_dir);


	std::cout << "Configuring network ..." << std::endl;

	// Configure the network interface
	ret = -1;
	while (ret < 0) {
        net_deinit();
        while ((ret = net_init()) == -EAGAIN);
        if (ret < 0) printf("net_init() failed: [%i] %s, retrying...\n", ret, strerror(-ret));
	}

	in_addr localip;
	localip.s_addr = net_gethostip();
	
	std::cout << "network configured, ip: " << inet_ntoa(localip) << std::endl;

	LWP_CreateThread(&httd_handle, /* thread handle */
						Server::server_entrypoint,		   /* code */
						(void*) &game,	   /* arg pointer for thread */
						NULL,		   /* stack base */
						128 * 1024,	   /* stack size */
						80 /* thread priority */);

	while (1)
	{

		VIDEO_WaitVSync();
		WPAD_ScanPads();

		uint32_t buttonsDown = WPAD_ButtonsDown(0);

		if (buttonsDown & WPAD_BUTTON_HOME)
		{
			exit(0);
		}

		// if(LWP_MutexTryLock(game.mutex))
		// {
			// TODO: Be able to get time left on graphics thread
			game.check_next_state_timeout();
		// 	LWP_MutexUnlock(game.mutex);
		// }

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
