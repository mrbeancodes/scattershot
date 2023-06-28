#ifndef SO_H
#define SO_H

#include "util.h"
#include "math.h"

typedef struct  {
    void* handle;
    void* sm64_base;
    
    VOIDFUNC sm64_init;
    VOIDFUNC sm64_update;

    //RENDERFUNC sm64_update_and_render;
    
    GFXFUNC  envfx_update_particles;
    EXECFUNC execute_mario_action;
    CAMFUNC update_camera; 
    GEOFUNC geo_update_animation_frame;

    // Variables
    Input *gControllerPads;
    void  *gMarioStates;
    void  *gObjectPool;
    void  *gEnvironmentRegions;
    Camera **gCamera;
    void  *gCameraOld;
    void  *gLakituState;
    //unsigned char *gLastCompletedStarNum = (unsigned char *)dlsym(hDLL, "gLastCompletedStarNum");
    short *gCurrCourseNum;
    short *gCurrAreaIndex;
    Controller *gControllers;
    //uint32_t *gTimeStopState = (uint32_t *)dlsym(hDLL, "gTimeStopState");
    //uint8_t *redCnt = (uint8_t *)dlsym(hDLL, "gRedCoinsCollected");
    u16 *gAreaUpdateCounter;
    
    //short *starCnt    = (short *)((char *)gMarioStates + 230);
    float *marioX;
    float *marioY;
    float *marioZ;
    float *marioFloorHeight;
    float *marioHSpd;
    uint16_t *marioYawFacing;
    //short *marioYawVel = (short *)((char *)gMarioStates + 52);
    float *marioYVel;
    short *marioPitch;
    short *marioPitchVel;
    uint16_t *camYaw;
    uint8_t *camMode;
    unsigned int *marioAction;
    //unsigned int *prevAction;
    //unsigned short *actionTimer;
    unsigned short *controlButDown;
    //uint32_t *ringCnt;
    //char *firstCol;
    u32* gFrameLogLength;

    //
    AnimInfo *marioAnimInfo;

    float *pyraXNorm;
    float *pyraYNorm;
    float *pyraZNorm;
    
    //float *bullyX;
    //float *bullyY;
    //float *bullyZ;
} SO;

void init_so(SO* so,char* dllName);
void print_act(SO *so);
void reduced_update(SO* so, Input* in);

#endif
