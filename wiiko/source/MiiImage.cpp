#include <stdlib.h>
#include <mii.h>
#include <grrlib.h>
#include <math.h>
#include "MiiImage.h"
#include "default_miis.h"

#include "mii_heads_png.h"
#include "mii_hairs1_png.h"
#include "mii_hairs2_png.h"
#include "mii_eyebrows_png.h"
#include "mii_eyes1_png.h"
#include "mii_eyes2_png.h"
#include "mii_eyes3_png.h"
#include "mii_noses_png.h"
#include "mii_features_png.h"
#include "mii_lips_png.h"
#include "mii_mole_png.h"
#include "mii_beards_png.h"
#include "mii_mustache_png.h"
#include "mii_glasses_png.h"
#include "AzeretMonoBold_ttf.h"

// ************************ DEFINE CONSTANTS ********************
const unsigned int hairbg[72] = {56,56,56,56,56,56,56,56,56,56,56,56,16,56,56,56,56,56,17,18,56,19,20,56,56,56,21,56,56,56,56,56,56,56,56,56,22,23,56,56,24,25,56,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,56,56,48,49,50,51,52,53,56};
const unsigned int hairfg[72] = {59,42,65,49,40,44,52,47,45,63,51,54,36,37,48,70,61,56,64,43,53,58,50,27,69,41,39,46,66,71,33,11,12,0,35,57,30,14,25,4,1,31,26,24,3,6,62,13,15,7,19,2,17,67,29,20,9,34,18,8,22,60,23,55,21,32,16,28,10,38,5,68};
const unsigned int haircol[8] = {0x111111FF, 0x332222FF, 0x441111FF, 0xBB6644FF, 0x8888AAFF, 0x443322FF, 0x996644FF, 0xDDBB99FF};
const unsigned int skincol[6] = {0xECCFBDFF, 0xF7BC7dFF, 0xD78A48FF, 0xF5B189FF, 0x995122FF, 0x563010FF};
const unsigned int eyecol[6] = {0x000000FF, 0x778887FF, 0x7E6355FF, 0x888940FF, 0x6A84D0FF, 0x409B5AFF};
const unsigned int lipcol[3] = {0xC76C46FF, 0xE44E3AFF, 0xD88789FF};
const unsigned int glassescol[6] = {0x626D6CFF, 0x85703AFF, 0xAB4E37FF, 0x426996FF, 0xB97F27FF, 0xBDBFB9FF};
const unsigned int eyebrows[24] = {1,3,14,15,11,10,0,6,8,4,13,12,2,19,16,18,22,9,21,5,17,7,20,23};
const unsigned int eyes[48] = {2,6,0,42,1,24,29,36,3,16,45,13,17,26,46,9,8,5,33,14,11,20,44,18,30,21,7,10,34,41,31,32,15,12,19,23,27,28,38,4,22,25,39,43,37,40,35,47};
const unsigned int noses[12] = {5,0,2,3,7,6,4,10,8,9,1,11};
const unsigned int lips[24] = {6,1,14,16,17,5,10,12,7,13,8,19,23,11,22,18,9,15,21,2,20,3,4,0};
// ***************************************************************
static GRRLIB_texImg* img_heads;
static GRRLIB_texImg* img_hairs1;
static GRRLIB_texImg* img_hairs2;
static GRRLIB_texImg* img_eyebrows;
static GRRLIB_texImg* img_eyes1;
static GRRLIB_texImg* img_eyes2;
static GRRLIB_texImg* img_eyes3;
static GRRLIB_texImg* img_noses;
static GRRLIB_texImg* img_features;
static GRRLIB_texImg* img_lips;
static GRRLIB_texImg* img_mole;
static GRRLIB_texImg* img_beards;
static GRRLIB_texImg* img_mustache;
static GRRLIB_texImg* img_glasses;
static GRRLIB_ttfFont* mii_name_font;

void MiiImage::default_expression()
{
	this->mouth_override = 0;
	this->eye_override = 0;
	this->needs_updating = true;
}

void MiiImage::make_happy()
{
    int happy_mouths[8] = {3, 4, 5, 8, 14, 16, 19, 23};
	this->mouth_override = (happy_mouths[rand() % 8]);
	this->eye_override = 0;
	this->needs_updating = true;
}

void MiiImage::make_sad() {
	int sad_mouths[3] = {9, 22, 24};
	this->mouth_override = (sad_mouths[rand() % 3]);
	int sad_eyes[3]={6, 33, 39};
	this->eye_override = (sad_eyes[rand() % 3]);
	this->needs_updating = true;
}

void MiiImage::make_shocked() {
	int shocked_mouths[2] = {15, 11};
	this->mouth_override = (shocked_mouths[rand() % 2]);
	int shocked_eyes[5] = {10, 13, 36, 37, 43};
	this->eye_override = (shocked_eyes[rand() % 5]);
	this->needs_updating = true;
}

void MiiImage::make_mad() {
	int mad_mouths[3] = {1, 21, 24};
	this->mouth_override = (mad_mouths[rand() % 3]);
	int mad_eyes[2] = {44, 44};
	this->eye_override = (mad_eyes[rand() % 2]);
	this->needs_updating = true;
}

void MiiImage::set_character(Character new_character)
{
	if (this->character != new_character) {
		this->character = new_character;
		this->needs_updating = true;
	}
}

void MiiImage::set_location(f32 x, f32 y)
{
	this->pos_x = x;
	this->pos_y = y;
}

void MiiImage::set_blinking_enabled(bool enable)
{
	this->blink_enabled = enable;
}

void MiiImage::set_bobbing_enabled(bool enable)
{
	this->bobbing_enabled = enable;
}

void MiiImage::set_name_card_enabled(bool enable)
{
	this->show_name_card = enable;
}

// Initialize tilesets used for rending Miis
void MiiImage::initialize(GRRLIB_ttfFont* font)
{
    img_heads = GRRLIB_LoadTexture(mii_heads_png);
	img_hairs1 = GRRLIB_LoadTexture(mii_hairs1_png);
	img_hairs2 = GRRLIB_LoadTexture(mii_hairs2_png);
	img_eyebrows = GRRLIB_LoadTexture(mii_eyebrows_png);
	img_eyes1 = GRRLIB_LoadTexture(mii_eyes1_png);
	img_eyes2 = GRRLIB_LoadTexture(mii_eyes2_png);
	img_eyes3 = GRRLIB_LoadTexture(mii_eyes3_png);
	img_noses = GRRLIB_LoadTexture(mii_noses_png);
	img_features = GRRLIB_LoadTexture(mii_features_png);
	img_lips = GRRLIB_LoadTexture(mii_lips_png);
	img_mole = GRRLIB_LoadTexture(mii_mole_png);
	img_beards = GRRLIB_LoadTexture(mii_beards_png);
	img_mustache = GRRLIB_LoadTexture(mii_mustache_png);
	img_glasses = GRRLIB_LoadTexture(mii_glasses_png);

	mii_name_font = font;
	
	GRRLIB_InitTileSet(img_heads, 120, 120, 0);
	GRRLIB_InitTileSet(img_hairs1, 120, 120, 0);
	GRRLIB_InitTileSet(img_hairs2, 120, 120, 0);
	GRRLIB_InitTileSet(img_eyebrows, 36, 54, 0);
	GRRLIB_InitTileSet(img_eyes1, 54, 54, 0);
	GRRLIB_InitTileSet(img_eyes2, 54, 54, 0);
	GRRLIB_InitTileSet(img_eyes3, 54, 54, 0);
	GRRLIB_InitTileSet(img_noses, 50, 50, 0);
	GRRLIB_InitTileSet(img_features, 120, 120, 0);
	GRRLIB_InitTileSet(img_lips, 60, 60, 0);
	GRRLIB_InitTileSet(img_beards, 120, 140, 0);
	GRRLIB_InitTileSet(img_mustache, 60, 60, 0);
	GRRLIB_InitTileSet(img_glasses, 180, 72, 0);
}

void MiiImage::composite() { // The heart and sole, builds up our mii with requested blink, eye, and mouth settings. 0s will build the Mii unaltered)
	
	if (!this->needs_updating)
		return;
	
	GRRLIB_CompoStart();

	const Mii& mii = default_miis[this->character];

	GRRLIB_SetHandle(img_hairs1, 60,0);
	GRRLIB_SetHandle(img_hairs2, 60,0);
	GRRLIB_DrawTile(230, 210, img_hairs2, 0, -1*(2*mii.hairPart-1), 1, haircol[mii.hairColor], hairbg[hairfg[mii.hairType]]);
	GRRLIB_DrawTile(230, 200, img_heads, 0, 1, 1, skincol[mii.skinColor], mii.faceShape);
	
	int feature = 39;
	
	switch (mii.facialFeature)
	{
		case 0 : feature = 39; break;
		case 1 : feature = mii.faceShape; break;
		case 2 : feature = mii.faceShape; break;
		case 3 : feature = 33; break;
		case 4 : feature = 34; break;
		case 5 : feature = 35; break;
		case 6 : feature = 36; break;
		case 7 : feature = 40 + mii.faceShape; break;
		case 8 : feature = 38; break;
		case 9 : feature = 8 + mii.faceShape; break;
		case 10: feature = 16 + mii.faceShape; break;
		case 11: feature = 24 + mii.faceShape; break;
		default: break;
	}
	
	if(mii.facialFeature==2) {
		GRRLIB_DrawTile(230, 200, img_features, 0, 1, 1, skincol[mii.skinColor], 32);
		GRRLIB_DrawTile(230, 200, img_features, 0, 1, 1, skincol[mii.skinColor], feature);
	}
	else GRRLIB_DrawTile(230, 200, img_features, 0, 1, 1, skincol[mii.skinColor], feature);
	
	if(mii.beardType>0) GRRLIB_DrawTile(230, 200, img_beards, 0, 1, 1, haircol[mii.facialHairColor], ((mii.beardType-1)*8)+mii.faceShape);
	
	GRRLIB_SetHandle(img_mole, 6, 6);
	if(mii.mole==1) GRRLIB_DrawImg(252+(4*mii.moleHorizPos), 212+(2.8*mii.moleVertPos), img_mole, 0, .2+(mii.moleSize*0.1), .2+(mii.moleSize*0.1), 0xFFFFFFFF);
	
	if(this->blinking) { //if blinking display closed eye
		GRRLIB_SetHandle(img_eyes1, 18,36);
		GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 46); 
		GRRLIB_SetHandle(img_eyes2, 18,36);
		GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], 46); 
		GRRLIB_SetHandle(img_eyes3, 18,36);
		GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 46); 
		GRRLIB_SetHandle(img_eyes1, 36,36);
		GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-46); 
		GRRLIB_SetHandle(img_eyes2, 36,36);
		GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], 95-46); 
		GRRLIB_SetHandle(img_eyes3, 36,36);
		GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-46);
	}
	else { // display open eye
		if(eye_override==0) { // if eye_override is 0 draw default open eye.
			GRRLIB_SetHandle(img_eyes1, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, eyes[mii.eyeType]); 
			GRRLIB_SetHandle(img_eyes2, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], eyes[mii.eyeType]); 
			GRRLIB_SetHandle(img_eyes3, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, eyes[mii.eyeType]); 
			GRRLIB_SetHandle(img_eyes1, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-eyes[mii.eyeType]); 
			GRRLIB_SetHandle(img_eyes2, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], 95-eyes[mii.eyeType]); 
			GRRLIB_SetHandle(img_eyes3, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-eyes[mii.eyeType]);
		}
		else { // draw custom open eye.
			GRRLIB_SetHandle(img_eyes1, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, eye_override-1); 
			GRRLIB_SetHandle(img_eyes2, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], eye_override-1); 
			GRRLIB_SetHandle(img_eyes3, 18,36);
			GRRLIB_DrawTile(272+(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, 11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, eye_override-1); 
			GRRLIB_SetHandle(img_eyes1, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes1, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-(eye_override-1)); 
			GRRLIB_SetHandle(img_eyes2, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes2, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), eyecol[mii.eyeColor], 95-(eye_override-1)); 
			GRRLIB_SetHandle(img_eyes3, 36,36);
			GRRLIB_DrawTile(254-(2.6*mii.eyeHorizSpacing), 184+(2.8*(mii.eyeVertPos)), img_eyes3, -11.25 * (7-mii.eyeRotation), .3+(mii.eyeSize*0.1), .3+(mii.eyeSize*0.1), 0xFFFFFFFF, 95-(eye_override-1));
		}
	}
	
	GRRLIB_SetHandle(img_eyebrows, 0, 54);
	GRRLIB_DrawTile(288+(2.6*mii.eyebrowHorizSpacing), 166+(2.8*(mii.eyebrowVertPos-3)), img_eyebrows, 11.25 * (11-mii.eyebrowRotation), .3+(mii.eyebrowSize*0.1), .3+(mii.eyebrowSize*0.1), haircol[mii.eyebrowColor], eyebrows[mii.eyebrowType]);
	GRRLIB_SetHandle(img_eyebrows, 36, 54);
	GRRLIB_DrawTile(256-(2.6*mii.eyebrowHorizSpacing), 166+(2.8*(mii.eyebrowVertPos-3)), img_eyebrows, -11.25 * (11-mii.eyebrowRotation), .3+(mii.eyebrowSize*0.1), .3+(mii.eyebrowSize*0.1), haircol[mii.eyebrowColor], 53-eyebrows[mii.eyebrowType]);
	
	GRRLIB_SetHandle(img_lips, 30, 30);
	if(mouth_override==0) {
		if(lips[mii.lipType]==1 || lips[mii.lipType]==6 || lips[mii.lipType]==11 || lips[mii.lipType]==17 || lips[mii.lipType]==19) {
			GRRLIB_DrawTile(260, 220+(2.6*mii.lipVertPos), img_lips, 0, .2+(mii.lipSize*0.1), .2+(mii.lipSize*0.1), lipcol[mii.lipColor], lips[mii.lipType]);
		}
		else {
			GRRLIB_DrawTile(260, 220+(2.6*mii.lipVertPos), img_lips, 0, .2+(mii.lipSize*0.1), .2+(mii.lipSize*0.1), 0XFFFFFFFF, lips[mii.lipType]);
		}
	}
	else {
		if(mouth_override-1==1 || mouth_override-1==6 || mouth_override-1==11 || mouth_override-1==17 || mouth_override-1==19) {
			GRRLIB_DrawTile(260, 220+(2.6*mii.lipVertPos), img_lips, 0, .2+(mii.lipSize*0.1), .2+(mii.lipSize*0.1), 0xC76C46FF, mouth_override-1);
		}
		else {
			GRRLIB_DrawTile(260, 220+(2.6*mii.lipVertPos), img_lips, 0, .2+(mii.lipSize*0.1), .2+(mii.lipSize*0.1), 0XFFFFFFFF, mouth_override-1);
		}
	}
	
	GRRLIB_SetHandle(img_mustache, 30, 10);
	if(mii.mustacheType>0) GRRLIB_DrawTile(260, 244+(2.9*mii.mustacheVertPos), img_mustache, 0, .2+(mii.mustacheSize*0.1), .2+(mii.mustacheSize*0.1), haircol[mii.facialHairColor], mii.mustacheType-1);
	
	GRRLIB_SetHandle(img_noses, 25, 30);
	GRRLIB_DrawTile(265, 220+(2.6*mii.noseVertPos), img_noses, 0, .2+(mii.noseSize*0.1), .2+(mii.noseSize*0.1), skincol[mii.skinColor], noses[mii.noseType]);
		
	if(hairfg[mii.hairType]<56) {
		GRRLIB_DrawTile(230, 190, img_hairs1, 0, -1*(2*mii.hairPart-1), 1, haircol[mii.hairColor], hairfg[mii.hairType]);
	}
	else {
		GRRLIB_DrawTile(230, 190, img_hairs2, 0, -1*(2*mii.hairPart-1), 1, haircol[mii.hairColor], hairfg[mii.hairType]-56);
	}

	if(mii.glassesType > 0) {
		GRRLIB_SetHandle(img_glasses, 90, 32);
		if(mii.glassesType < 6) {
			GRRLIB_DrawTile(200, 195+(2.6*(mii.glassesVertPos)), img_glasses, 0, .1+(mii.glassesSize*0.1), .1+(mii.glassesSize*0.1), glassescol[mii.glassesColor], mii.glassesType-1);
		}
		else {
			GRRLIB_DrawTile(200, 195+(2.6*(mii.glassesVertPos)), img_glasses, 0, .1+(mii.glassesSize*0.1), .1+(mii.glassesSize*0.1), glassescol[mii.glassesColor], mii.glassesType+2);
			GRRLIB_DrawTile(200, 195+(2.6*(mii.glassesVertPos)), img_glasses, 0, .1+(mii.glassesSize*0.1), .1+(mii.glassesSize*0.1), 0xFFFFFFFF, mii.glassesType-1);
		}
	}
	GRRLIB_CompoEnd(200, 160, this->texture);		// finish the composition and store it as whatever texture was passed to GRRLIB_Build Mii
	this->needs_updating = false;
}

void MiiImage::render()
{
	GRRLIB_Circle(this->pos_x, this->pos_y, 65, 0xFFFFFFFF, true);
    GRRLIB_Circle(this->pos_x, this->pos_y, 60, 0xFFFFFFFF, true);

	if (this->show_name_card) {
		float top_left_x = this->pos_x - 65;
		float top_left_y = this->pos_y + 65 + 5;
		float width = 65 * 2;
		float height = 40;
		float radius = 10;
		// Full height, less width
		GRRLIB_Rectangle(top_left_x + radius, top_left_y, width - 2 * radius, height, 0xFFFFFFFF, true);
		// Full width, less height
		GRRLIB_Rectangle(top_left_x, top_left_y + radius, width, height - 2 * radius, 0xFFFFFFFF, true);
		GRRLIB_Circle(top_left_x + radius, top_left_y + radius, radius, 0xFFFFFFFF, true);
		GRRLIB_Circle(top_left_x + width - radius, top_left_y + radius, radius, 0xFFFFFFFF, true);
		GRRLIB_Circle(top_left_x + radius, top_left_y + height - radius, radius, 0xFFFFFFFF, true);
		GRRLIB_Circle(top_left_x + width - radius, top_left_y + height - radius, radius, 0xFFFFFFFF, true);

		char name[] = "bilbo";
		int text_start_x = top_left_x - 2 + width / 2 - (sizeof(name) - 1) * 6;
		GRRLIB_PrintfTTF(text_start_x, (int) top_left_y + 8, mii_name_font, name, 20, 0x000000FF);
	}

	float mii_position_x = this->pos_x;
	float mii_position_y = this->pos_y;

	if (this->bobbing_enabled) {
		this->bobbing_param += this->bobbing_param_inc;
		mii_position_x += 5 * cos(this->bobbing_param);
		mii_position_y += 2 * sin(2*this->bobbing_param);
	}

    GRRLIB_DrawImg(mii_position_x, mii_position_y+5, this->texture, 0, 1, 1, 0xFFFFFFFF);
}

MiiImage::MiiImage(Character character, f32 pos_x, f32 pos_y)
{
	this->character = character;
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->eye_override = 0;
	this->mouth_override = 0;
	this->blink_enabled = false;
	this->blink_timer = 0;
	this->blinking = false;
	this->bobbing_enabled = false;
	this->texture = GRRLIB_CreateEmptyTexture(180, 200);
	this->bobbing_param = (rand() % 6280) % 1000;
	this->bobbing_param_inc = 0.05; //((rand() % 30) + 30) / 1000.0f;
	GRRLIB_SetMidHandle(this->texture, true);

	this->needs_updating = true;
}

MiiImage::~MiiImage()
{
	GRRLIB_FreeTexture(this->texture);
}