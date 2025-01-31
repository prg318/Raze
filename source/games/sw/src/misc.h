#pragma once

#include "game.h"

// This consolidates the contents of several smaller includes
BEGIN_SW_NS

enum
{
	CACHE_SOUND_PRECACHE = 0,
	CACHE_SOUND_PLAY =1
};

void DoTheCache(void);

void InitCheats();

void MapColors(short num,COLOR_MAP cm,short create);
void InitPalette(void); 
int32_t CONFIG_ReadSetup(void);

bool WarpPlaneSectorInfo(short sectnum, SPRITEp* sp_ceiling, SPRITEp* sp_floor);
SPRITEp WarpPlane(int32_t* x, int32_t* y, int32_t* z, int16_t* sectnum);
SPRITEp WarpToArea(SPRITEp sp_from, int32_t* x, int32_t* y, int32_t* z, int16_t* sectnum);
bool WarpSectorInfo(short sectnum, SPRITEp* sp_warp);
SPRITEp Warp(int32_t* x, int32_t* y, int32_t* z, int16_t* sectnum);

void ProcessVisOn(void);
void VisViewChange(PLAYERp pp, int* vis);
int SpawnVis(short Parent, short sectnum, int x, int y, int z, int amt);

enum TriggerType { TRIGGER_TYPE_REMOTE_SO };

int ActorFollowTrack(short SpriteNum, short locktics);
void ActorLeaveTrack(short SpriteNum);
void RefreshPoints(SECTOR_OBJECTp sop, int nx, int ny, bool dynamic);
void TrackSetup(void);
void PlaceSectorObject(SECTOR_OBJECTp sop, int newx, int newy);
void PlaceSectorObjectsOnTracks(void);
void PlaceActorsOnTracks(void);
void SetupSectorObject(short sectnum, short tag);
void PostSetupSectorObject(void);
void VehicleSetSmoke(SECTOR_OBJECTp sop, ANIMATORp animator);
void CollapseSectorObject(SECTOR_OBJECTp sop, int nx, int ny);
void KillSectorObjectSprites(SECTOR_OBJECTp sop);
void MoveSectorObjects(SECTOR_OBJECTp sop, short locktics);

#define TEXT_INFO_TIME (3)
#define TEXT_INFO_Y (40)
#define TEXT_INFO_YOFF (10)
inline constexpr int TEXT_INFO_LINE(int line) { return (TEXT_INFO_Y + ((line)*TEXT_INFO_YOFF)); }

void PutStringInfo(PLAYERp pp, const char* string);


short DoSlidorMatch(PLAYERp pp, short match, bool);
bool TestSlidorMatchActive(short match);
void InterpSectorSprites(short sectnum, bool state);

using INTERP_FUNC = void(*)(int, int);

void SetSlidorActive(short SpriteNum);
void DoSlidorInterp(short, INTERP_FUNC);

int DoBeginJump(short SpriteNum);
int DoJump(short SpriteNum);
int DoBeginFall(short SpriteNum);
int DoFall(short SpriteNum);
void KeepActorOnFloor(short SpriteNum);
int DoActorSlide(short SpriteNum);
int DoActorSectorDamage(short SpriteNum);
int DoScaleSprite(short SpriteNum);

void InitPlayerSprite(PLAYERp pp);
void InitAllPlayerSprites(void);
void PlayerPanelSetup(void);
void PlayerDeathReset(PLAYERp pp);
void SpawnPlayerUnderSprite(PLAYERp pp);

void DoQuakeMatch(short match);
void ProcessQuakeOn(void);
void ProcessQuakeSpot(void);
void QuakeViewChange(PLAYERp pp, int* z_diff, int* x_diff, int* y_diff, short* ang_diff);
void DoQuake(PLAYERp pp);
bool SetQuake(PLAYERp pp, short tics, short amt);
int SetExpQuake(int16_t Weapon);
int SetGunQuake(int16_t SpriteNum);
int SetPlayerQuake(PLAYERp mpp);
int SetNuclearQuake(int16_t Weapon);
int SetSumoQuake(int16_t SpriteNum);
int SetSumoFartQuake(int16_t SpriteNum);

END_SW_NS
