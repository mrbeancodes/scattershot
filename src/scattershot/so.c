#include "scattershot/so.h"
#include "scattershot/settings.h"

#ifdef _WIN32
#define dlsym(hDLL, name) GetProcAddress(hDLL, name)
#define dlopen(dll, flag) LoadLibrary(dll)
#endif

void init_so(SO* so,char* dllName) {

    so->handle = dlopen(dllName, RTLD_LAZY); //???
    soStruct helper = {.fname= dllName, .base_addr=0};
    dl_iterate_phdr(find_sm64so_base, &helper);
    so->sm64_base = (void*) helper.base_addr; 

    // Functions
    so->sm64_init   = (VOIDFUNC)dlsym(so->handle, "sm64_init");
    so->sm64_update = (VOIDFUNC)dlsym(so->handle, "sm64_update");

    //so->sm64_update_and_render = (RENDERFUNC)dlsym(so->handle, "sm64_update_and_render");

    so->envfx_update_particles = (GFXFUNC)dlsym(so->handle, "envfx_update_particles");
    so->execute_mario_action = (EXECFUNC)dlsym(so->handle, "execute_mario_action");
    so->update_camera = (CAMFUNC) dlsym(so->handle, "update_camera");   
    so->geo_update_animation_frame = (GEOFUNC)dlsym(so->handle, "geo_update_animation_frame");

    // Variables
    so->gControllerPads  = (Input *)dlsym(so->handle, "gControllerPads");
    so->gMarioStates     = dlsym(so->handle, "gMarioStates");
    so->gObjectPool      = dlsym(so->handle, "gObjectPool");
    so->gEnvironmentRegions = dlsym(so->handle, "gEnvironmentRegions");
    so->gCamera          = (Camera **) dlsym(so->handle, "gCamera");
    so->gCameraOld          = dlsym(so->handle, "gCamera");
    so->gLakituState     = dlsym(so->handle, "gLakituState");

    so->gFrameLogLength  = (u32 *) dlsym(so->handle, "gFrameLogLength");
    so->gMarioObject     = (void *) dlsym(so->handle, "gMarioObject");
    //unsigned char *gLastCompletedStarNum = (unsigned char *)dlsym(so->handle, "gLastCompletedStarNum");
    so->gCurrCourseNum = (short *)dlsym(so->handle, "gCurrCourseNum");
    so->gCurrAreaIndex = (short *)dlsym(so->handle, "gCurrAreaIndex");
    so->gControllers = (Controller *)dlsym(so->handle, "gControllers");
    //uint32_t *gTimeStopState = (uint32_t *)dlsym(so->handle, "gTimeStopState");
    //uint8_t *redCnt = (uint8_t *)dlsym(so->handle, "gRedCoinsCollected");
    so->gAreaUpdateCounter = (u16 *)dlsym(so->handle, "gAreaUpdateCounter");
    
    //short *starCnt    = (short *)((char *)gMarioStates + 230);
    so->marioX     = (float *)((char *)so->gMarioStates + 60);
    so->marioY     = (float *)((char *)so->gMarioStates + 64);
    so->marioZ     = (float *)((char *)so->gMarioStates + 68);
    so->marioFloorHeight = (float *)((char *)so->gMarioStates + 0x07C);
    so->marioHSpd  = (float *)((char *)so->gMarioStates + 0x54);
    so->marioYawFacing = (uint16_t *)((char *)so->gMarioStates + 46);
    so->marioYawIntended = (uint16_t *)((char *)so->gMarioStates + 36);
    //short *marioYawVel = (short *)((char *)so->gMarioStates + 52);
    so->marioYVel = (float *)((char *)so->gMarioStates + 76);
    so->marioPitch = (short *)((char *)so->gMarioStates + 44);
    so->marioPitchVel = (short *) ((char*)so->gMarioStates + 50);
    so->camYaw = (uint16_t *)((char *)so->gCamera + 340);


    //so->camMode = (uint8_t *)so->sm64_base + bssStart + 29605;
    so->camMode = (u8*) dlsym(so->handle, "gCamera");

    so->marioAction = (unsigned int *)((char *)so->gMarioStates + 12);
    //unsigned int *prevAction = (unsigned int *)((char *)so->gMarioStates + 0x10);
    //unsigned short *actionTimer = (unsigned short *)((char *)so->gMarioStates + 0x1A);
    so->controlButDown = (unsigned short *)((char *)so->gControllers + 0x10);
    //uint32_t *ringCnt = (uint32_t *)((char *)so->gObjectPool + 18 * 1392 + 508);
    //char *firstCol = (char *)((char *)so->gObjectPool + 30 * 1392 + 412);
    //
    //so->pyraXNorm = (float *)((char *)so->gObjectPool + 84 * 1392 + 324);
    //so->pyraYNorm = (float *)((char *)so->gObjectPool + 84 * 1392 + 328);
    //so->pyraZNorm = (float *)((char *)so->gObjectPool + 84 * 1392 + 332);
    
    so->obj1AnimInfo =  (AnimInfo *)((char *)so->gObjectPool + 121 * 1392 + 0x50); //carpet
    //so->marioAnimInfo = (AnimInfo *)((char *)so->gObjectPool + MARIO_OBJ_INX * 1392 + 0x50);
    //findMarioIndex(so);

    //float *bullyX = (float *)((char *)gObjectPool + 57 * 1392 + 56);
    //float *bullyY = (float *)((char *)gObjectPool + 57 * 1392 + 60);
    //float *bullyZ = (float *)((char *)gObjectPool + 57 * 1392 + 64);
}

void inputToController(Controller *controller, Input* input) {
        float mag, effX = 0, effY = 0;
        int x = input->x, y = input->y;

        if (x <= -8) effX = x + 6;
        if (x >= 8)  effX = x - 6;
        if (y <= -8) effY = y + 6;
        if (y >= 8)  effY = y - 6;

        mag = sqrtf(effX * effX + effY * effY);
        if (mag > 64) {
            effX *= 64 / mag;
            effY *= 64 / mag;
            mag = 64;
        }
        controller->rawStickX = x;
        controller->rawStickY = y;
        controller->stickX    = effX;
        controller->stickY    = effY;
        controller->stickMag  = mag;
        controller->buttonPressed = input->b & (input->b ^ controller->buttonDown);
        controller->buttonDown    = input->b;
}


void reduced_update(SO* so, Input* in){
    so->gControllerPads = in;
    inputToController(so->gControllers, in);
    so->execute_mario_action(NULL);
    *so->gFrameLogLength = 0;
    *so->gAreaUpdateCounter = *so->gAreaUpdateCounter + 1;
    so->marioAnimInfo->animFrame = so->geo_update_animation_frame(so->marioAnimInfo, &so->marioAnimInfo->animFrameAccelAssist);
    so->marioAnimInfo->animTimer = *so->gAreaUpdateCounter;

    //idk
    //so->obj1AnimInfo->animFrame = so->geo_update_animation_frame(so->obj1AnimInfo, &so->obj1AnimInfo->animFrameAccelAssist);
    //so->obj1AnimInfo->animTimer = *so->gAreaUpdateCounter;
    //
    so->update_camera(*so->gCamera); // Optional
}


void findMarioIndex(SO* so) {
    int objsize = 1392;
    bool found = false;
    for(int i = 0;i < 240;i++){
        int objsize = 1392;
        //printf("%lx == %lx\n",(void *)so->gObjectPool + i*objsize , (void*)so->gMarioObject);  
        float* objX = (float*) ((char *)so->gObjectPool + i * objsize + 56);
        float* objY = (float*) ((char *)so->gObjectPool + i * objsize + 60);
        float* objZ = (float*) ((char *)so->gObjectPool + i * objsize + 64);
        //if (*objY < -1700.0f && *objX > 4100.0f) //filter to find the obj
        //printf("x %f y %f z %f index %d",*objX, *objY, *objZ, i);
        if(*objX == *so->marioX && *objY == *so->marioY && *objZ == *so->marioZ && !found){
            so->marioAnimInfo = (AnimInfo *)((char *)so->gObjectPool + i * objsize + 0x50);
            printf(" found mario");
            found = true;
            //return;
        }
        //printf("\n");
    }
    if (!found) {
        printf("no index found\n");
        exit(1);
    }
    printf("end index\n");
}


