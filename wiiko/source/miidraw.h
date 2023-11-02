/*===========================================
    Wii Double Down Mii interface
    Code     : mdbrim

    miidraw.h
============================================*/
#ifndef MIIDRAW_H
#define MIIDRAW_H

#include <mii.h>
#include "mii_config.h"

extern GRRLIB_texImg* img_staticmii[MAX_MIIS];
extern int bodyadjust[MAX_MIIS];
extern char miinames[MAX_MIIS][11];

int MakeMiiHappy(int bodypart);
int MakeMiiSad(int bodypart);
int MakeMiiShocked(int bodypart);
int MakeMiiMad(int bodypart);
void GRRLIB_MiisInit();
void GRRLIB_BuildMii(Mii mii, int miiid, int miiblinks, int miieyes, int miimouth, GRRLIB_texImg* tex);
void GRRLIB_CreateStaticMiis();

#endif