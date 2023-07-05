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

//meme
int get_act(char* actName) {
    if(strstr(actName,"uninitialized") && strlen(actName) == strlen("uninitialized")) {
        return ACT_UNINITIALIZED;
    }
    else if(strstr(actName,"idle") && strlen(actName) == strlen("idle")) {
        return ACT_IDLE;
    }
    else if(strstr(actName,"start_sleeping") && strlen(actName) == strlen("start_sleeping")) {
        return ACT_START_SLEEPING;
    }
    else if(strstr(actName,"sleeping") && strlen(actName) == strlen("sleeping")) {
        return ACT_SLEEPING;
    }
    else if(strstr(actName,"waking_up") && strlen(actName) == strlen("waking_up")) {
        return ACT_WAKING_UP;
    }
    else if(strstr(actName,"panting") && strlen(actName) == strlen("panting")) {
        return ACT_PANTING;
    }
    else if(strstr(actName,"hold_panting_(unused)") && strlen(actName) == strlen("hold_panting_(unused)")) {
        return ACT_HOLD_PANTING_(UNUSED);
    }
    else if(strstr(actName,"hold_idle") && strlen(actName) == strlen("hold_idle")) {
        return ACT_HOLD_IDLE;
    }
    else if(strstr(actName,"hold_heavy_idle") && strlen(actName) == strlen("hold_heavy_idle")) {
        return ACT_HOLD_HEAVY_IDLE;
    }
    else if(strstr(actName,"standing_against_wall") && strlen(actName) == strlen("standing_against_wall")) {
        return ACT_STANDING_AGAINST_WALL;
    }
    else if(strstr(actName,"coughing") && strlen(actName) == strlen("coughing")) {
        return ACT_COUGHING;
    }
    else if(strstr(actName,"shivering") && strlen(actName) == strlen("shivering")) {
        return ACT_SHIVERING;
    }
    else if(strstr(actName,"in_quicksand") && strlen(actName) == strlen("in_quicksand")) {
        return ACT_IN_QUICKSAND;
    }
    else if(strstr(actName,"unknown_2020e") && strlen(actName) == strlen("unknown_2020e")) {
        return ACT_UNKNOWN_2020E;
    }
    else if(strstr(actName,"crouching") && strlen(actName) == strlen("crouching")) {
        return ACT_CROUCHING;
    }
    else if(strstr(actName,"start_crouching") && strlen(actName) == strlen("start_crouching")) {
        return ACT_START_CROUCHING;
    }
    else if(strstr(actName,"stop_crouching") && strlen(actName) == strlen("stop_crouching")) {
        return ACT_STOP_CROUCHING;
    }
    else if(strstr(actName,"start_crawling") && strlen(actName) == strlen("start_crawling")) {
        return ACT_START_CRAWLING;
    }
    else if(strstr(actName,"stop_crawling") && strlen(actName) == strlen("stop_crawling")) {
        return ACT_STOP_CRAWLING;
    }
    else if(strstr(actName,"slide_kick_slide_stop") && strlen(actName) == strlen("slide_kick_slide_stop")) {
        return ACT_SLIDE_KICK_SLIDE_STOP;
    }
    else if(strstr(actName,"shockwave_bounce") && strlen(actName) == strlen("shockwave_bounce")) {
        return ACT_SHOCKWAVE_BOUNCE;
    }
    else if(strstr(actName,"first_person") && strlen(actName) == strlen("first_person")) {
        return ACT_FIRST_PERSON;
    }
    else if(strstr(actName,"backflip_land_stop") && strlen(actName) == strlen("backflip_land_stop")) {
        return ACT_BACKFLIP_LAND_STOP;
    }
    else if(strstr(actName,"jump_land_stop") && strlen(actName) == strlen("jump_land_stop")) {
        return ACT_JUMP_LAND_STOP;
    }
    else if(strstr(actName,"double_jump_land_stop") && strlen(actName) == strlen("double_jump_land_stop")) {
        return ACT_DOUBLE_JUMP_LAND_STOP;
    }
    else if(strstr(actName,"freefall_land_stop") && strlen(actName) == strlen("freefall_land_stop")) {
        return ACT_FREEFALL_LAND_STOP;
    }
    else if(strstr(actName,"side_flip_land_stop") && strlen(actName) == strlen("side_flip_land_stop")) {
        return ACT_SIDE_FLIP_LAND_STOP;
    }
    else if(strstr(actName,"hold_jump_land_stop") && strlen(actName) == strlen("hold_jump_land_stop")) {
        return ACT_HOLD_JUMP_LAND_STOP;
    }
    else if(strstr(actName,"hold_freefall_land_stop") && strlen(actName) == strlen("hold_freefall_land_stop")) {
        return ACT_HOLD_FREEFALL_LAND_STOP;
    }
    else if(strstr(actName,"air_throw_land") && strlen(actName) == strlen("air_throw_land")) {
        return ACT_AIR_THROW_LAND;
    }
    else if(strstr(actName,"twirl_land") && strlen(actName) == strlen("twirl_land")) {
        return ACT_TWIRL_LAND;
    }
    else if(strstr(actName,"lava_boost_land") && strlen(actName) == strlen("lava_boost_land")) {
        return ACT_LAVA_BOOST_LAND;
    }
    else if(strstr(actName,"triple_jump_land_stop") && strlen(actName) == strlen("triple_jump_land_stop")) {
        return ACT_TRIPLE_JUMP_LAND_STOP;
    }
    else if(strstr(actName,"long_jump_land_stop") && strlen(actName) == strlen("long_jump_land_stop")) {
        return ACT_LONG_JUMP_LAND_STOP;
    }
    else if(strstr(actName,"ground_pound_land") && strlen(actName) == strlen("ground_pound_land")) {
        return ACT_GROUND_POUND_LAND;
    }
    else if(strstr(actName,"braking_stop") && strlen(actName) == strlen("braking_stop")) {
        return ACT_BRAKING_STOP;
    }
    else if(strstr(actName,"butt_slide_stop") && strlen(actName) == strlen("butt_slide_stop")) {
        return ACT_BUTT_SLIDE_STOP;
    }
    else if(strstr(actName,"hold_butt_slide_stop") && strlen(actName) == strlen("hold_butt_slide_stop")) {
        return ACT_HOLD_BUTT_SLIDE_STOP;
    }
    else if(strstr(actName,"walking") && strlen(actName) == strlen("walking")) {
        return ACT_WALKING;
    }
    else if(strstr(actName,"hold_walking") && strlen(actName) == strlen("hold_walking")) {
        return ACT_HOLD_WALKING;
    }
    else if(strstr(actName,"turning_around") && strlen(actName) == strlen("turning_around")) {
        return ACT_TURNING_AROUND;
    }
    else if(strstr(actName,"finish_turning_around") && strlen(actName) == strlen("finish_turning_around")) {
        return ACT_FINISH_TURNING_AROUND;
    }
    else if(strstr(actName,"braking") && strlen(actName) == strlen("braking")) {
        return ACT_BRAKING;
    }
    else if(strstr(actName,"riding_shell_ground") && strlen(actName) == strlen("riding_shell_ground")) {
        return ACT_RIDING_SHELL_GROUND;
    }
    else if(strstr(actName,"hold_heavy_walking") && strlen(actName) == strlen("hold_heavy_walking")) {
        return ACT_HOLD_HEAVY_WALKING;
    }
    else if(strstr(actName,"crawling") && strlen(actName) == strlen("crawling")) {
        return ACT_CRAWLING;
    }
    else if(strstr(actName,"burning_ground") && strlen(actName) == strlen("burning_ground")) {
        return ACT_BURNING_GROUND;
    }
    else if(strstr(actName,"decelerating") && strlen(actName) == strlen("decelerating")) {
        return ACT_DECELERATING;
    }
    else if(strstr(actName,"hold_decelerating") && strlen(actName) == strlen("hold_decelerating")) {
        return ACT_HOLD_DECELERATING;
    }
    else if(strstr(actName,"begin_sliding") && strlen(actName) == strlen("begin_sliding")) {
        return ACT_BEGIN_SLIDING;
    }
    else if(strstr(actName,"hold_begin_sliding") && strlen(actName) == strlen("hold_begin_sliding")) {
        return ACT_HOLD_BEGIN_SLIDING;
    }
    else if(strstr(actName,"butt_slide") && strlen(actName) == strlen("butt_slide")) {
        return ACT_BUTT_SLIDE;
    }
    else if(strstr(actName,"stomach_slide") && strlen(actName) == strlen("stomach_slide")) {
        return ACT_STOMACH_SLIDE;
    }
    else if(strstr(actName,"hold_butt_slide") && strlen(actName) == strlen("hold_butt_slide")) {
        return ACT_HOLD_BUTT_SLIDE;
    }
    else if(strstr(actName,"hold_stomach_slide") && strlen(actName) == strlen("hold_stomach_slide")) {
        return ACT_HOLD_STOMACH_SLIDE;
    }
    else if(strstr(actName,"dive_slide") && strlen(actName) == strlen("dive_slide")) {
        return ACT_DIVE_SLIDE;
    }
    else if(strstr(actName,"move_punching") && strlen(actName) == strlen("move_punching")) {
        return ACT_MOVE_PUNCHING;
    }
    else if(strstr(actName,"crouch_slide") && strlen(actName) == strlen("crouch_slide")) {
        return ACT_CROUCH_SLIDE;
    }
    else if(strstr(actName,"slide_kick_slide") && strlen(actName) == strlen("slide_kick_slide")) {
        return ACT_SLIDE_KICK_SLIDE;
    }
    else if(strstr(actName,"hard_backward_ground_kb") && strlen(actName) == strlen("hard_backward_ground_kb")) {
        return ACT_HARD_BACKWARD_GROUND_KB;
    }
    else if(strstr(actName,"hard_forward_ground_kb") && strlen(actName) == strlen("hard_forward_ground_kb")) {
        return ACT_HARD_FORWARD_GROUND_KB;
    }
    else if(strstr(actName,"backward_ground_kb") && strlen(actName) == strlen("backward_ground_kb")) {
        return ACT_BACKWARD_GROUND_KB;
    }
    else if(strstr(actName,"forward_ground_kb") && strlen(actName) == strlen("forward_ground_kb")) {
        return ACT_FORWARD_GROUND_KB;
    }
    else if(strstr(actName,"soft_backward_ground_kb") && strlen(actName) == strlen("soft_backward_ground_kb")) {
        return ACT_SOFT_BACKWARD_GROUND_KB;
    }
    else if(strstr(actName,"soft_forward_ground_kb") && strlen(actName) == strlen("soft_forward_ground_kb")) {
        return ACT_SOFT_FORWARD_GROUND_KB;
    }
    else if(strstr(actName,"ground_bonk") && strlen(actName) == strlen("ground_bonk")) {
        return ACT_GROUND_BONK;
    }
    else if(strstr(actName,"death_exit_land") && strlen(actName) == strlen("death_exit_land")) {
        return ACT_DEATH_EXIT_LAND;
    }
    else if(strstr(actName,"jump_land") && strlen(actName) == strlen("jump_land")) {
        return ACT_JUMP_LAND;
    }
    else if(strstr(actName,"freefall_land") && strlen(actName) == strlen("freefall_land")) {
        return ACT_FREEFALL_LAND;
    }
    else if(strstr(actName,"double_jump_land") && strlen(actName) == strlen("double_jump_land")) {
        return ACT_DOUBLE_JUMP_LAND;
    }
    else if(strstr(actName,"side_flip_land") && strlen(actName) == strlen("side_flip_land")) {
        return ACT_SIDE_FLIP_LAND;
    }
    else if(strstr(actName,"hold_jump_land") && strlen(actName) == strlen("hold_jump_land")) {
        return ACT_HOLD_JUMP_LAND;
    }
    else if(strstr(actName,"hold_freefall_land") && strlen(actName) == strlen("hold_freefall_land")) {
        return ACT_HOLD_FREEFALL_LAND;
    }
    else if(strstr(actName,"quicksand_jump_land") && strlen(actName) == strlen("quicksand_jump_land")) {
        return ACT_QUICKSAND_JUMP_LAND;
    }
    else if(strstr(actName,"hold_quicksand_jump_land") && strlen(actName) == strlen("hold_quicksand_jump_land")) {
        return ACT_HOLD_QUICKSAND_JUMP_LAND;
    }
    else if(strstr(actName,"triple_jump_land") && strlen(actName) == strlen("triple_jump_land")) {
        return ACT_TRIPLE_JUMP_LAND;
    }
    else if(strstr(actName,"long_jump_land") && strlen(actName) == strlen("long_jump_land")) {
        return ACT_LONG_JUMP_LAND;
    }
    else if(strstr(actName,"backflip_land") && strlen(actName) == strlen("backflip_land")) {
        return ACT_BACKFLIP_LAND;
    }
    else if(strstr(actName,"jump") && strlen(actName) == strlen("jump")) {
        return ACT_JUMP;
    }
    else if(strstr(actName,"double_jump") && strlen(actName) == strlen("double_jump")) {
        return ACT_DOUBLE_JUMP;
    }
    else if(strstr(actName,"triple_jump") && strlen(actName) == strlen("triple_jump")) {
        return ACT_TRIPLE_JUMP;
    }
    else if(strstr(actName,"backflip") && strlen(actName) == strlen("backflip")) {
        return ACT_BACKFLIP;
    }
    else if(strstr(actName,"steep_jump") && strlen(actName) == strlen("steep_jump")) {
        return ACT_STEEP_JUMP;
    }
    else if(strstr(actName,"wall_kick_air") && strlen(actName) == strlen("wall_kick_air")) {
        return ACT_WALL_KICK_AIR;
    }
    else if(strstr(actName,"side_flip") && strlen(actName) == strlen("side_flip")) {
        return ACT_SIDE_FLIP;
    }
    else if(strstr(actName,"long_jump") && strlen(actName) == strlen("long_jump")) {
        return ACT_LONG_JUMP;
    }
    else if(strstr(actName,"water_jump") && strlen(actName) == strlen("water_jump")) {
        return ACT_WATER_JUMP;
    }
    else if(strstr(actName,"dive") && strlen(actName) == strlen("dive")) {
        return ACT_DIVE;
    }
    else if(strstr(actName,"freefall") && strlen(actName) == strlen("freefall")) {
        return ACT_FREEFALL;
    }
    else if(strstr(actName,"top_of_pole_jump") && strlen(actName) == strlen("top_of_pole_jump")) {
        return ACT_TOP_OF_POLE_JUMP;
    }
    else if(strstr(actName,"butt_slide_air") && strlen(actName) == strlen("butt_slide_air")) {
        return ACT_BUTT_SLIDE_AIR;
    }
    else if(strstr(actName,"flying_triple_jump") && strlen(actName) == strlen("flying_triple_jump")) {
        return ACT_FLYING_TRIPLE_JUMP;
    }
    else if(strstr(actName,"shot_from_cannon") && strlen(actName) == strlen("shot_from_cannon")) {
        return ACT_SHOT_FROM_CANNON;
    }
    else if(strstr(actName,"flying") && strlen(actName) == strlen("flying")) {
        return ACT_FLYING;
    }
    else if(strstr(actName,"riding_shell_jump") && strlen(actName) == strlen("riding_shell_jump")) {
        return ACT_RIDING_SHELL_JUMP;
    }
    else if(strstr(actName,"riding_shell_fall") && strlen(actName) == strlen("riding_shell_fall")) {
        return ACT_RIDING_SHELL_FALL;
    }
    else if(strstr(actName,"vertical_wind") && strlen(actName) == strlen("vertical_wind")) {
        return ACT_VERTICAL_WIND;
    }
    else if(strstr(actName,"hold_jump") && strlen(actName) == strlen("hold_jump")) {
        return ACT_HOLD_JUMP;
    }
    else if(strstr(actName,"hold_freefall") && strlen(actName) == strlen("hold_freefall")) {
        return ACT_HOLD_FREEFALL;
    }
    else if(strstr(actName,"hold_butt_slide_air") && strlen(actName) == strlen("hold_butt_slide_air")) {
        return ACT_HOLD_BUTT_SLIDE_AIR;
    }
    else if(strstr(actName,"hold_water_jump") && strlen(actName) == strlen("hold_water_jump")) {
        return ACT_HOLD_WATER_JUMP;
    }
    else if(strstr(actName,"twirling") && strlen(actName) == strlen("twirling")) {
        return ACT_TWIRLING;
    }
    else if(strstr(actName,"forward_rollout") && strlen(actName) == strlen("forward_rollout")) {
        return ACT_FORWARD_ROLLOUT;
    }
    else if(strstr(actName,"air_hit_wall") && strlen(actName) == strlen("air_hit_wall")) {
        return ACT_AIR_HIT_WALL;
    }
    else if(strstr(actName,"riding_hoot") && strlen(actName) == strlen("riding_hoot")) {
        return ACT_RIDING_HOOT;
    }
    else if(strstr(actName,"ground_pound") && strlen(actName) == strlen("ground_pound")) {
        return ACT_GROUND_POUND;
    }
    else if(strstr(actName,"slide_kick") && strlen(actName) == strlen("slide_kick")) {
        return ACT_SLIDE_KICK;
    }
    else if(strstr(actName,"air_throw") && strlen(actName) == strlen("air_throw")) {
        return ACT_AIR_THROW;
    }
    else if(strstr(actName,"jump_kick") && strlen(actName) == strlen("jump_kick")) {
        return ACT_JUMP_KICK;
    }
    else if(strstr(actName,"backward_rollout") && strlen(actName) == strlen("backward_rollout")) {
        return ACT_BACKWARD_ROLLOUT;
    }
    else if(strstr(actName,"crazy_box_bounce") && strlen(actName) == strlen("crazy_box_bounce")) {
        return ACT_CRAZY_BOX_BOUNCE;
    }
    else if(strstr(actName,"special_triple_jump") && strlen(actName) == strlen("special_triple_jump")) {
        return ACT_SPECIAL_TRIPLE_JUMP;
    }
    else if(strstr(actName,"backward_air_kb") && strlen(actName) == strlen("backward_air_kb")) {
        return ACT_BACKWARD_AIR_KB;
    }
    else if(strstr(actName,"forward_air_kb") && strlen(actName) == strlen("forward_air_kb")) {
        return ACT_FORWARD_AIR_KB;
    }
    else if(strstr(actName,"hard_forward_air_kb") && strlen(actName) == strlen("hard_forward_air_kb")) {
        return ACT_HARD_FORWARD_AIR_KB;
    }
    else if(strstr(actName,"hard_backward_air_kb") && strlen(actName) == strlen("hard_backward_air_kb")) {
        return ACT_HARD_BACKWARD_AIR_KB;
    }
    else if(strstr(actName,"burning_jump") && strlen(actName) == strlen("burning_jump")) {
        return ACT_BURNING_JUMP;
    }
    else if(strstr(actName,"burning_fall") && strlen(actName) == strlen("burning_fall")) {
        return ACT_BURNING_FALL;
    }
    else if(strstr(actName,"soft_bonk") && strlen(actName) == strlen("soft_bonk")) {
        return ACT_SOFT_BONK;
    }
    else if(strstr(actName,"lava_boost") && strlen(actName) == strlen("lava_boost")) {
        return ACT_LAVA_BOOST;
    }
    else if(strstr(actName,"getting_blown") && strlen(actName) == strlen("getting_blown")) {
        return ACT_GETTING_BLOWN;
    }
    else if(strstr(actName,"thrown_forward") && strlen(actName) == strlen("thrown_forward")) {
        return ACT_THROWN_FORWARD;
    }
    else if(strstr(actName,"thrown_backward") && strlen(actName) == strlen("thrown_backward")) {
        return ACT_THROWN_BACKWARD;
    }
    else if(strstr(actName,"water_idle") && strlen(actName) == strlen("water_idle")) {
        return ACT_WATER_IDLE;
    }
    else if(strstr(actName,"hold_water_idle") && strlen(actName) == strlen("hold_water_idle")) {
        return ACT_HOLD_WATER_IDLE;
    }
    else if(strstr(actName,"water_action_end") && strlen(actName) == strlen("water_action_end")) {
        return ACT_WATER_ACTION_END;
    }
    else if(strstr(actName,"hold_water_action_end") && strlen(actName) == strlen("hold_water_action_end")) {
        return ACT_HOLD_WATER_ACTION_END;
    }
    else if(strstr(actName,"drowning") && strlen(actName) == strlen("drowning")) {
        return ACT_DROWNING;
    }
    else if(strstr(actName,"backward_water_kb") && strlen(actName) == strlen("backward_water_kb")) {
        return ACT_BACKWARD_WATER_KB;
    }
    else if(strstr(actName,"forward_water_kb") && strlen(actName) == strlen("forward_water_kb")) {
        return ACT_FORWARD_WATER_KB;
    }
    else if(strstr(actName,"water_death") && strlen(actName) == strlen("water_death")) {
        return ACT_WATER_DEATH;
    }
    else if(strstr(actName,"water_shocked") && strlen(actName) == strlen("water_shocked")) {
        return ACT_WATER_SHOCKED;
    }
    else if(strstr(actName,"breaststroke") && strlen(actName) == strlen("breaststroke")) {
        return ACT_BREASTSTROKE;
    }
    else if(strstr(actName,"swimming_end") && strlen(actName) == strlen("swimming_end")) {
        return ACT_SWIMMING_END;
    }
    else if(strstr(actName,"flutter_kick") && strlen(actName) == strlen("flutter_kick")) {
        return ACT_FLUTTER_KICK;
    }
    else if(strstr(actName,"hold_breaststroke") && strlen(actName) == strlen("hold_breaststroke")) {
        return ACT_HOLD_BREASTSTROKE;
    }
    else if(strstr(actName,"hold_swimming_end") && strlen(actName) == strlen("hold_swimming_end")) {
        return ACT_HOLD_SWIMMING_END;
    }
    else if(strstr(actName,"hold_flutter_kick") && strlen(actName) == strlen("hold_flutter_kick")) {
        return ACT_HOLD_FLUTTER_KICK;
    }
    else if(strstr(actName,"water_shell_swimming") && strlen(actName) == strlen("water_shell_swimming")) {
        return ACT_WATER_SHELL_SWIMMING;
    }
    else if(strstr(actName,"water_throw") && strlen(actName) == strlen("water_throw")) {
        return ACT_WATER_THROW;
    }
    else if(strstr(actName,"water_punch") && strlen(actName) == strlen("water_punch")) {
        return ACT_WATER_PUNCH;
    }
    else if(strstr(actName,"water_plunge") && strlen(actName) == strlen("water_plunge")) {
        return ACT_WATER_PLUNGE;
    }
    else if(strstr(actName,"caught_in_whirlpool") && strlen(actName) == strlen("caught_in_whirlpool")) {
        return ACT_CAUGHT_IN_WHIRLPOOL;
    }
    else if(strstr(actName,"metal_water_standing") && strlen(actName) == strlen("metal_water_standing")) {
        return ACT_METAL_WATER_STANDING;
    }
    else if(strstr(actName,"hold_metal_water_standing") && strlen(actName) == strlen("hold_metal_water_standing")) {
        return ACT_HOLD_METAL_WATER_STANDING;
    }
    else if(strstr(actName,"metal_water_walking") && strlen(actName) == strlen("metal_water_walking")) {
        return ACT_METAL_WATER_WALKING;
    }
    else if(strstr(actName,"hold_metal_water_walking") && strlen(actName) == strlen("hold_metal_water_walking")) {
        return ACT_HOLD_METAL_WATER_WALKING;
    }
    else if(strstr(actName,"metal_water_falling") && strlen(actName) == strlen("metal_water_falling")) {
        return ACT_METAL_WATER_FALLING;
    }
    else if(strstr(actName,"hold_metal_water_falling") && strlen(actName) == strlen("hold_metal_water_falling")) {
        return ACT_HOLD_METAL_WATER_FALLING;
    }
    else if(strstr(actName,"metal_water_fall_land") && strlen(actName) == strlen("metal_water_fall_land")) {
        return ACT_METAL_WATER_FALL_LAND;
    }
    else if(strstr(actName,"hold_metal_water_fall_land") && strlen(actName) == strlen("hold_metal_water_fall_land")) {
        return ACT_HOLD_METAL_WATER_FALL_LAND;
    }
    else if(strstr(actName,"metal_water_jump") && strlen(actName) == strlen("metal_water_jump")) {
        return ACT_METAL_WATER_JUMP;
    }
    else if(strstr(actName,"hold_metal_water_jump") && strlen(actName) == strlen("hold_metal_water_jump")) {
        return ACT_HOLD_METAL_WATER_JUMP;
    }
    else if(strstr(actName,"metal_water_jump_land") && strlen(actName) == strlen("metal_water_jump_land")) {
        return ACT_METAL_WATER_JUMP_LAND;
    }
    else if(strstr(actName,"hold_metal_water_jump_land") && strlen(actName) == strlen("hold_metal_water_jump_land")) {
        return ACT_HOLD_METAL_WATER_JUMP_LAND;
    }
    else if(strstr(actName,"disappeared") && strlen(actName) == strlen("disappeared")) {
        return ACT_DISAPPEARED;
    }
    else if(strstr(actName,"intro_cutscene") && strlen(actName) == strlen("intro_cutscene")) {
        return ACT_INTRO_CUTSCENE;
    }
    else if(strstr(actName,"star_dance_exit") && strlen(actName) == strlen("star_dance_exit")) {
        return ACT_STAR_DANCE_EXIT;
    }
    else if(strstr(actName,"star_dance_water") && strlen(actName) == strlen("star_dance_water")) {
        return ACT_STAR_DANCE_WATER;
    }
    else if(strstr(actName,"fall_after_star_grab") && strlen(actName) == strlen("fall_after_star_grab")) {
        return ACT_FALL_AFTER_STAR_GRAB;
    }
    else if(strstr(actName,"reading_automatic_dialog") && strlen(actName) == strlen("reading_automatic_dialog")) {
        return ACT_READING_AUTOMATIC_DIALOG;
    }
    else if(strstr(actName,"reading_npc_dialog") && strlen(actName) == strlen("reading_npc_dialog")) {
        return ACT_READING_NPC_DIALOG;
    }
    else if(strstr(actName,"star_dance_no_exit") && strlen(actName) == strlen("star_dance_no_exit")) {
        return ACT_STAR_DANCE_NO_EXIT;
    }
    else if(strstr(actName,"reading_sign") && strlen(actName) == strlen("reading_sign")) {
        return ACT_READING_SIGN;
    }
    else if(strstr(actName,"grand_star_cutscene") && strlen(actName) == strlen("grand_star_cutscene")) {
        return ACT_GRAND_STAR_CUTSCENE;
    }
    else if(strstr(actName,"waiting_for_dialog") && strlen(actName) == strlen("waiting_for_dialog")) {
        return ACT_WAITING_FOR_DIALOG;
    }
    else if(strstr(actName,"debug_free_move") && strlen(actName) == strlen("debug_free_move")) {
        return ACT_DEBUG_FREE_MOVE;
    }
    else if(strstr(actName,"standing_death") && strlen(actName) == strlen("standing_death")) {
        return ACT_STANDING_DEATH;
    }
    else if(strstr(actName,"quicksand_death") && strlen(actName) == strlen("quicksand_death")) {
        return ACT_QUICKSAND_DEATH;
    }
    else if(strstr(actName,"electrocution") && strlen(actName) == strlen("electrocution")) {
        return ACT_ELECTROCUTION;
    }
    else if(strstr(actName,"suffocation") && strlen(actName) == strlen("suffocation")) {
        return ACT_SUFFOCATION;
    }
    else if(strstr(actName,"death_on_stomach") && strlen(actName) == strlen("death_on_stomach")) {
        return ACT_DEATH_ON_STOMACH;
    }
    else if(strstr(actName,"death_on_back") && strlen(actName) == strlen("death_on_back")) {
        return ACT_DEATH_ON_BACK;
    }
    else if(strstr(actName,"eaten_by_bubba") && strlen(actName) == strlen("eaten_by_bubba")) {
        return ACT_EATEN_BY_BUBBA;
    }
    else if(strstr(actName,"peach_cutscene") && strlen(actName) == strlen("peach_cutscene")) {
        return ACT_PEACH_CUTSCENE;
    }
    else if(strstr(actName,"credits") && strlen(actName) == strlen("credits")) {
        return ACT_CREDITS;
    }
    else if(strstr(actName,"waving") && strlen(actName) == strlen("waving")) {
        return ACT_WAVING;
    }
    else if(strstr(actName,"pulling_door") && strlen(actName) == strlen("pulling_door")) {
        return ACT_PULLING_DOOR;
    }
    else if(strstr(actName,"pushing_door") && strlen(actName) == strlen("pushing_door")) {
        return ACT_PUSHING_DOOR;
    }
    else if(strstr(actName,"warp_door_spawn") && strlen(actName) == strlen("warp_door_spawn")) {
        return ACT_WARP_DOOR_SPAWN;
    }
    else if(strstr(actName,"emerge_from_pipe") && strlen(actName) == strlen("emerge_from_pipe")) {
        return ACT_EMERGE_FROM_PIPE;
    }
    else if(strstr(actName,"spawn_spin_airborne") && strlen(actName) == strlen("spawn_spin_airborne")) {
        return ACT_SPAWN_SPIN_AIRBORNE;
    }
    else if(strstr(actName,"spawn_spin_landing") && strlen(actName) == strlen("spawn_spin_landing")) {
        return ACT_SPAWN_SPIN_LANDING;
    }
    else if(strstr(actName,"exit_airborne") && strlen(actName) == strlen("exit_airborne")) {
        return ACT_EXIT_AIRBORNE;
    }
    else if(strstr(actName,"exit_land_save_dialog") && strlen(actName) == strlen("exit_land_save_dialog")) {
        return ACT_EXIT_LAND_SAVE_DIALOG;
    }
    else if(strstr(actName,"death_exit") && strlen(actName) == strlen("death_exit")) {
        return ACT_DEATH_EXIT;
    }
    else if(strstr(actName,"death_exit_(unused)") && strlen(actName) == strlen("death_exit_(unused)")) {
        return ACT_DEATH_EXIT_(UNUSED);
    }
    else if(strstr(actName,"falling_death_exit") && strlen(actName) == strlen("falling_death_exit")) {
        return ACT_FALLING_DEATH_EXIT;
    }
    else if(strstr(actName,"special_exit_airborne") && strlen(actName) == strlen("special_exit_airborne")) {
        return ACT_SPECIAL_EXIT_AIRBORNE;
    }
    else if(strstr(actName,"special_death_exit") && strlen(actName) == strlen("special_death_exit")) {
        return ACT_SPECIAL_DEATH_EXIT;
    }
    else if(strstr(actName,"falling_exit_airborne") && strlen(actName) == strlen("falling_exit_airborne")) {
        return ACT_FALLING_EXIT_AIRBORNE;
    }
    else if(strstr(actName,"unlocking_key_door") && strlen(actName) == strlen("unlocking_key_door")) {
        return ACT_UNLOCKING_KEY_DOOR;
    }
    else if(strstr(actName,"unlocking_star_door") && strlen(actName) == strlen("unlocking_star_door")) {
        return ACT_UNLOCKING_STAR_DOOR;
    }
    else if(strstr(actName,"entering_star_door") && strlen(actName) == strlen("entering_star_door")) {
        return ACT_ENTERING_STAR_DOOR;
    }
    else if(strstr(actName,"spawn_no_spin_airborne") && strlen(actName) == strlen("spawn_no_spin_airborne")) {
        return ACT_SPAWN_NO_SPIN_AIRBORNE;
    }
    else if(strstr(actName,"spawn_no_spin_landing") && strlen(actName) == strlen("spawn_no_spin_landing")) {
        return ACT_SPAWN_NO_SPIN_LANDING;
    }
    else if(strstr(actName,"bbh_enter_jump") && strlen(actName) == strlen("bbh_enter_jump")) {
        return ACT_BBH_ENTER_JUMP;
    }
    else if(strstr(actName,"bbh_enter_spin") && strlen(actName) == strlen("bbh_enter_spin")) {
        return ACT_BBH_ENTER_SPIN;
    }
    else if(strstr(actName,"teleport_fade_out") && strlen(actName) == strlen("teleport_fade_out")) {
        return ACT_TELEPORT_FADE_OUT;
    }
    else if(strstr(actName,"teleport_fade_in") && strlen(actName) == strlen("teleport_fade_in")) {
        return ACT_TELEPORT_FADE_IN;
    }
    else if(strstr(actName,"shocked") && strlen(actName) == strlen("shocked")) {
        return ACT_SHOCKED;
    }
    else if(strstr(actName,"squished") && strlen(actName) == strlen("squished")) {
        return ACT_SQUISHED;
    }
    else if(strstr(actName,"head_stuck_in_ground") && strlen(actName) == strlen("head_stuck_in_ground")) {
        return ACT_HEAD_STUCK_IN_GROUND;
    }
    else if(strstr(actName,"butt_stuck_in_ground") && strlen(actName) == strlen("butt_stuck_in_ground")) {
        return ACT_BUTT_STUCK_IN_GROUND;
    }
    else if(strstr(actName,"feet_stuck_in_ground") && strlen(actName) == strlen("feet_stuck_in_ground")) {
        return ACT_FEET_STUCK_IN_GROUND;
    }
    else if(strstr(actName,"putting_on_cap") && strlen(actName) == strlen("putting_on_cap")) {
        return ACT_PUTTING_ON_CAP;
    }
    else if(strstr(actName,"holding_pole") && strlen(actName) == strlen("holding_pole")) {
        return ACT_HOLDING_POLE;
    }
    else if(strstr(actName,"grab_pole_slow") && strlen(actName) == strlen("grab_pole_slow")) {
        return ACT_GRAB_POLE_SLOW;
    }
    else if(strstr(actName,"grab_pole_fast") && strlen(actName) == strlen("grab_pole_fast")) {
        return ACT_GRAB_POLE_FAST;
    }
    else if(strstr(actName,"climbing_pole") && strlen(actName) == strlen("climbing_pole")) {
        return ACT_CLIMBING_POLE;
    }
    else if(strstr(actName,"top_of_pole_transition") && strlen(actName) == strlen("top_of_pole_transition")) {
        return ACT_TOP_OF_POLE_TRANSITION;
    }
    else if(strstr(actName,"top_of_pole") && strlen(actName) == strlen("top_of_pole")) {
        return ACT_TOP_OF_POLE;
    }
    else if(strstr(actName,"start_hanging") && strlen(actName) == strlen("start_hanging")) {
        return ACT_START_HANGING;
    }
    else if(strstr(actName,"hanging") && strlen(actName) == strlen("hanging")) {
        return ACT_HANGING;
    }
    else if(strstr(actName,"hang_moving") && strlen(actName) == strlen("hang_moving")) {
        return ACT_HANG_MOVING;
    }
    else if(strstr(actName,"ledge_grab") && strlen(actName) == strlen("ledge_grab")) {
        return ACT_LEDGE_GRAB;
    }
    else if(strstr(actName,"ledge_climb_slow_1") && strlen(actName) == strlen("ledge_climb_slow_1")) {
        return ACT_LEDGE_CLIMB_SLOW_1;
    }
    else if(strstr(actName,"ledge_climb_slow_2") && strlen(actName) == strlen("ledge_climb_slow_2")) {
        return ACT_LEDGE_CLIMB_SLOW_2;
    }
    else if(strstr(actName,"ledge_climb_down") && strlen(actName) == strlen("ledge_climb_down")) {
        return ACT_LEDGE_CLIMB_DOWN;
    }
    else if(strstr(actName,"ledge_climb_fast") && strlen(actName) == strlen("ledge_climb_fast")) {
        return ACT_LEDGE_CLIMB_FAST;
    }
    else if(strstr(actName,"grabbed") && strlen(actName) == strlen("grabbed")) {
        return ACT_GRABBED;
    }
    else if(strstr(actName,"in_cannon") && strlen(actName) == strlen("in_cannon")) {
        return ACT_IN_CANNON;
    }
    else if(strstr(actName,"tornado_twirling") && strlen(actName) == strlen("tornado_twirling")) {
        return ACT_TORNADO_TWIRLING;
    }
    else if(strstr(actName,"punching") && strlen(actName) == strlen("punching")) {
        return ACT_PUNCHING;
    }
    else if(strstr(actName,"picking_up") && strlen(actName) == strlen("picking_up")) {
        return ACT_PICKING_UP;
    }
    else if(strstr(actName,"dive_picking_up") && strlen(actName) == strlen("dive_picking_up")) {
        return ACT_DIVE_PICKING_UP;
    }
    else if(strstr(actName,"stomach_slide_stop") && strlen(actName) == strlen("stomach_slide_stop")) {
        return ACT_STOMACH_SLIDE_STOP;
    }
    else if(strstr(actName,"placing_down") && strlen(actName) == strlen("placing_down")) {
        return ACT_PLACING_DOWN;
    }
    else if(strstr(actName,"throwing") && strlen(actName) == strlen("throwing")) {
        return ACT_THROWING;
    }
    else if(strstr(actName,"heavy_throw") && strlen(actName) == strlen("heavy_throw")) {
        return ACT_HEAVY_THROW;
    }
    else if(strstr(actName,"picking_up_bowser") && strlen(actName) == strlen("picking_up_bowser")) {
        return ACT_PICKING_UP_BOWSER;
    }
    else if(strstr(actName,"holding_bowser") && strlen(actName) == strlen("holding_bowser")) {
        return ACT_HOLDING_BOWSER;
    }
    else if(strstr(actName,"releasing_bowser") && strlen(actName) == strlen("releasing_bowser")) {
        return ACT_RELEASING_BOWSER;
    }
    return ACT_ANY;
}


//meme
void print_act(SO *so) {
    int action_trunc = *so->marioAction & 0x1FF;
    switch (action_trunc) {
    case ACT_UNINITIALIZED:
        printf("uninitialized\n");
        break;
    case ACT_IDLE:
        printf("idle\n");
        break;
    case ACT_START_SLEEPING:
        printf("start sleeping\n");
        break;
    case ACT_SLEEPING:
        printf("sleeping\n");
        break;
    case ACT_WAKING_UP:
        printf("waking up\n");
        break;
    case ACT_PANTING:
        printf("panting\n");
        break;
    case ACT_HOLD_PANTING_(UNUSED):
        printf("hold panting (unused)\n");
        break;
    case ACT_HOLD_IDLE:
        printf("hold idle\n");
        break;
    case ACT_HOLD_HEAVY_IDLE:
        printf("hold heavy idle\n");
        break;
    case ACT_STANDING_AGAINST_WALL:
        printf("standing against wall\n");
        break;
    case ACT_COUGHING:
        printf("coughing\n");
        break;
    case ACT_SHIVERING:
        printf("shivering\n");
        break;
    case ACT_IN_QUICKSAND:
        printf("in quicksand\n");
        break;
    case ACT_UNKNOWN_2020E:
        printf("unknown 2020E\n");
        break;
    case ACT_CROUCHING:
        printf("crouching\n");
        break;
    case ACT_START_CROUCHING:
        printf("start crouching\n");
        break;
    case ACT_STOP_CROUCHING:
        printf("stop crouching\n");
        break;
    case ACT_START_CRAWLING:
        printf("start crawling\n");
        break;
    case ACT_STOP_CRAWLING:
        printf("stop crawling\n");
        break;
    case ACT_SLIDE_KICK_SLIDE_STOP:
        printf("slide kick slide stop\n");
        break;
    case ACT_SHOCKWAVE_BOUNCE:
        printf("shockwave bounce\n");
        break;
    case ACT_FIRST_PERSON:
        printf("first person\n");
        break;
    case ACT_BACKFLIP_LAND_STOP:
        printf("backflip land stop\n");
        break;
    case ACT_JUMP_LAND_STOP:
        printf("jump land stop\n");
        break;
    case ACT_DOUBLE_JUMP_LAND_STOP:
        printf("double jump land stop\n");
        break;
    case ACT_FREEFALL_LAND_STOP:
        printf("freefall land stop\n");
        break;
    case ACT_SIDE_FLIP_LAND_STOP:
        printf("side flip land stop\n");
        break;
    case ACT_HOLD_JUMP_LAND_STOP:
        printf("hold jump land stop\n");
        break;
    case ACT_HOLD_FREEFALL_LAND_STOP:
        printf("hold freefall land stop\n");
        break;
    case ACT_AIR_THROW_LAND:
        printf("air throw land\n");
        break;
    case ACT_TWIRL_LAND:
        printf("twirl land\n");
        break;
    case ACT_LAVA_BOOST_LAND:
        printf("lava boost land\n");
        break;
    case ACT_TRIPLE_JUMP_LAND_STOP:
        printf("triple jump land stop\n");
        break;
    case ACT_LONG_JUMP_LAND_STOP:
        printf("long jump land stop\n");
        break;
    case ACT_GROUND_POUND_LAND:
        printf("ground pound land\n");
        break;
    case ACT_BRAKING_STOP:
        printf("braking stop\n");
        break;
    case ACT_BUTT_SLIDE_STOP:
        printf("butt slide stop\n");
        break;
    case ACT_HOLD_BUTT_SLIDE_STOP:
        printf("hold butt slide stop\n");
        break;
    case ACT_WALKING:
        printf("walking\n");
        break;
    case ACT_HOLD_WALKING:
        printf("hold walking\n");
        break;
    case ACT_TURNING_AROUND:
        printf("turning around\n");
        break;
    case ACT_FINISH_TURNING_AROUND:
        printf("finish turning around\n");
        break;
    case ACT_BRAKING:
        printf("braking\n");
        break;
    case ACT_RIDING_SHELL_GROUND:
        printf("riding shell ground\n");
        break;
    case ACT_HOLD_HEAVY_WALKING:
        printf("hold heavy walking\n");
        break;
    case ACT_CRAWLING:
        printf("crawling\n");
        break;
    case ACT_BURNING_GROUND:
        printf("burning ground\n");
        break;
    case ACT_DECELERATING:
        printf("decelerating\n");
        break;
    case ACT_HOLD_DECELERATING:
        printf("hold decelerating\n");
        break;
    case ACT_BEGIN_SLIDING:
        printf("begin sliding\n");
        break;
    case ACT_HOLD_BEGIN_SLIDING:
        printf("hold begin sliding\n");
        break;
    case ACT_BUTT_SLIDE:
        printf("butt slide\n");
        break;
    case ACT_STOMACH_SLIDE:
        printf("stomach slide\n");
        break;
    case ACT_HOLD_BUTT_SLIDE:
        printf("hold butt slide\n");
        break;
    case ACT_HOLD_STOMACH_SLIDE:
        printf("hold stomach slide\n");
        break;
    case ACT_DIVE_SLIDE:
        printf("dive slide\n");
        break;
    case ACT_MOVE_PUNCHING:
        printf("move punching\n");
        break;
    case ACT_CROUCH_SLIDE:
        printf("crouch slide\n");
        break;
    case ACT_SLIDE_KICK_SLIDE:
        printf("slide kick slide\n");
        break;
    case ACT_HARD_BACKWARD_GROUND_KB:
        printf("hard backward ground kb\n");
        break;
    case ACT_HARD_FORWARD_GROUND_KB:
        printf("hard forward ground kb\n");
        break;
    case ACT_BACKWARD_GROUND_KB:
        printf("backward ground kb\n");
        break;
    case ACT_FORWARD_GROUND_KB:
        printf("forward ground kb\n");
        break;
    case ACT_SOFT_BACKWARD_GROUND_KB:
        printf("soft backward ground kb\n");
        break;
    case ACT_SOFT_FORWARD_GROUND_KB:
        printf("soft forward ground kb\n");
        break;
    case ACT_GROUND_BONK:
        printf("ground bonk\n");
        break;
    case ACT_DEATH_EXIT_LAND:
        printf("death exit land\n");
        break;
    case ACT_JUMP_LAND:
        printf("jump land\n");
        break;
    case ACT_FREEFALL_LAND:
        printf("freefall land\n");
        break;
    case ACT_DOUBLE_JUMP_LAND:
        printf("double jump land\n");
        break;
    case ACT_SIDE_FLIP_LAND:
        printf("side flip land\n");
        break;
    case ACT_HOLD_JUMP_LAND:
        printf("hold jump land\n");
        break;
    case ACT_HOLD_FREEFALL_LAND:
        printf("hold freefall land\n");
        break;
    case ACT_QUICKSAND_JUMP_LAND:
        printf("quicksand jump land\n");
        break;
    case ACT_HOLD_QUICKSAND_JUMP_LAND:
        printf("hold quicksand jump land\n");
        break;
    case ACT_TRIPLE_JUMP_LAND:
        printf("triple jump land\n");
        break;
    case ACT_LONG_JUMP_LAND:
        printf("long jump land\n");
        break;
    case ACT_BACKFLIP_LAND:
        printf("backflip land\n");
        break;
    case ACT_JUMP:
        printf("jump\n");
        break;
    case ACT_DOUBLE_JUMP:
        printf("double jump\n");
        break;
    case ACT_TRIPLE_JUMP:
        printf("triple jump\n");
        break;
    case ACT_BACKFLIP:
        printf("backflip\n");
        break;
    case ACT_STEEP_JUMP:
        printf("steep jump\n");
        break;
    case ACT_WALL_KICK_AIR:
        printf("wall kick air\n");
        break;
    case ACT_SIDE_FLIP:
        printf("side flip\n");
        break;
    case ACT_LONG_JUMP:
        printf("long jump\n");
        break;
    case ACT_WATER_JUMP:
        printf("water jump\n");
        break;
    case ACT_DIVE:
        printf("dive\n");
        break;
    case ACT_FREEFALL:
        printf("freefall\n");
        break;
    case ACT_TOP_OF_POLE_JUMP:
        printf("top of pole jump\n");
        break;
    case ACT_BUTT_SLIDE_AIR:
        printf("butt slide air\n");
        break;
    case ACT_FLYING_TRIPLE_JUMP:
        printf("flying triple jump\n");
        break;
    case ACT_SHOT_FROM_CANNON:
        printf("shot from cannon\n");
        break;
    case ACT_FLYING:
        printf("flying\n");
        break;
    case ACT_RIDING_SHELL_JUMP:
        printf("riding shell jump\n");
        break;
    case ACT_RIDING_SHELL_FALL:
        printf("riding shell fall\n");
        break;
    case ACT_VERTICAL_WIND:
        printf("vertical wind\n");
        break;
    case ACT_HOLD_JUMP:
        printf("hold jump\n");
        break;
    case ACT_HOLD_FREEFALL:
        printf("hold freefall\n");
        break;
    case ACT_HOLD_BUTT_SLIDE_AIR:
        printf("hold butt slide air\n");
        break;
    case ACT_HOLD_WATER_JUMP:
        printf("hold water jump\n");
        break;
    case ACT_TWIRLING:
        printf("twirling\n");
        break;
    case ACT_FORWARD_ROLLOUT:
        printf("forward rollout\n");
        break;
    case ACT_AIR_HIT_WALL:
        printf("air hit wall\n");
        break;
    case ACT_RIDING_HOOT:
        printf("riding hoot\n");
        break;
    case ACT_GROUND_POUND:
        printf("ground pound\n");
        break;
    case ACT_SLIDE_KICK:
        printf("slide kick\n");
        break;
    case ACT_AIR_THROW:
        printf("air throw\n");
        break;
    case ACT_JUMP_KICK:
        printf("jump kick\n");
        break;
    case ACT_BACKWARD_ROLLOUT:
        printf("backward rollout\n");
        break;
    case ACT_CRAZY_BOX_BOUNCE:
        printf("crazy box bounce\n");
        break;
    case ACT_SPECIAL_TRIPLE_JUMP:
        printf("special triple jump\n");
        break;
    case ACT_BACKWARD_AIR_KB:
        printf("backward air kb\n");
        break;
    case ACT_FORWARD_AIR_KB:
        printf("forward air kb\n");
        break;
    case ACT_HARD_FORWARD_AIR_KB:
        printf("hard forward air kb\n");
        break;
    case ACT_HARD_BACKWARD_AIR_KB:
        printf("hard backward air kb\n");
        break;
    case ACT_BURNING_JUMP:
        printf("burning jump\n");
        break;
    case ACT_BURNING_FALL:
        printf("burning fall\n");
        break;
    case ACT_SOFT_BONK:
        printf("soft bonk\n");
        break;
    case ACT_LAVA_BOOST:
        printf("lava boost\n");
        break;
    case ACT_GETTING_BLOWN:
        printf("getting blown\n");
        break;
    case ACT_THROWN_FORWARD:
        printf("thrown forward\n");
        break;
    case ACT_THROWN_BACKWARD:
        printf("thrown backward\n");
        break;
    case ACT_WATER_IDLE:
        printf("water idle\n");
        break;
    case ACT_HOLD_WATER_IDLE:
        printf("hold water idle\n");
        break;
    case ACT_WATER_ACTION_END:
        printf("water action end\n");
        break;
    case ACT_HOLD_WATER_ACTION_END:
        printf("hold water action end\n");
        break;
    case ACT_DROWNING:
        printf("drowning\n");
        break;
    case ACT_BACKWARD_WATER_KB:
        printf("backward water kb\n");
        break;
    case ACT_FORWARD_WATER_KB:
        printf("forward water kb\n");
        break;
    case ACT_WATER_DEATH:
        printf("water death\n");
        break;
    case ACT_WATER_SHOCKED:
        printf("water shocked\n");
        break;
    case ACT_BREASTSTROKE:
        printf("breaststroke\n");
        break;
    case ACT_SWIMMING_END:
        printf("swimming end\n");
        break;
    case ACT_FLUTTER_KICK:
        printf("flutter kick\n");
        break;
    case ACT_HOLD_BREASTSTROKE:
        printf("hold breaststroke\n");
        break;
    case ACT_HOLD_SWIMMING_END:
        printf("hold swimming end\n");
        break;
    case ACT_HOLD_FLUTTER_KICK:
        printf("hold flutter kick\n");
        break;
    case ACT_WATER_SHELL_SWIMMING:
        printf("water shell swimming\n");
        break;
    case ACT_WATER_THROW:
        printf("water throw\n");
        break;
    case ACT_WATER_PUNCH:
        printf("water punch\n");
        break;
    case ACT_WATER_PLUNGE:
        printf("water plunge\n");
        break;
    case ACT_CAUGHT_IN_WHIRLPOOL:
        printf("caught in whirlpool\n");
        break;
    case ACT_METAL_WATER_STANDING:
        printf("metal water standing\n");
        break;
    case ACT_HOLD_METAL_WATER_STANDING:
        printf("hold metal water standing\n");
        break;
    case ACT_METAL_WATER_WALKING:
        printf("metal water walking\n");
        break;
    case ACT_HOLD_METAL_WATER_WALKING:
        printf("hold metal water walking\n");
        break;
    case ACT_METAL_WATER_FALLING:
        printf("metal water falling\n");
        break;
    case ACT_HOLD_METAL_WATER_FALLING:
        printf("hold metal water falling\n");
        break;
    case ACT_METAL_WATER_FALL_LAND:
        printf("metal water fall land\n");
        break;
    case ACT_HOLD_METAL_WATER_FALL_LAND:
        printf("hold metal water fall land\n");
        break;
    case ACT_METAL_WATER_JUMP:
        printf("metal water jump\n");
        break;
    case ACT_HOLD_METAL_WATER_JUMP:
        printf("hold metal water jump\n");
        break;
    case ACT_METAL_WATER_JUMP_LAND:
        printf("metal water jump land\n");
        break;
    case ACT_HOLD_METAL_WATER_JUMP_LAND:
        printf("hold metal water jump land\n");
        break;
    case ACT_DISAPPEARED:
        printf("disappeared\n");
        break;
    case ACT_INTRO_CUTSCENE:
        printf("intro cutscene\n");
        break;
    case ACT_STAR_DANCE_EXIT:
        printf("star dance exit\n");
        break;
    case ACT_STAR_DANCE_WATER:
        printf("star dance water\n");
        break;
    case ACT_FALL_AFTER_STAR_GRAB:
        printf("fall after star grab\n");
        break;
    case ACT_READING_AUTOMATIC_DIALOG:
        printf("reading automatic dialog\n");
        break;
    case ACT_READING_NPC_DIALOG:
        printf("reading npc dialog\n");
        break;
    case ACT_STAR_DANCE_NO_EXIT:
        printf("star dance no exit\n");
        break;
    case ACT_READING_SIGN:
        printf("reading sign\n");
        break;
    case ACT_GRAND_STAR_CUTSCENE:
        printf("grand star cutscene\n");
        break;
    case ACT_WAITING_FOR_DIALOG:
        printf("waiting for dialog\n");
        break;
    case ACT_DEBUG_FREE_MOVE:
        printf("debug free move\n");
        break;
    case ACT_STANDING_DEATH:
        printf("standing death\n");
        break;
    case ACT_QUICKSAND_DEATH:
        printf("quicksand death\n");
        break;
    case ACT_ELECTROCUTION:
        printf("electrocution\n");
        break;
    case ACT_SUFFOCATION:
        printf("suffocation\n");
        break;
    case ACT_DEATH_ON_STOMACH:
        printf("death on stomach\n");
        break;
    case ACT_DEATH_ON_BACK:
        printf("death on back\n");
        break;
    case ACT_EATEN_BY_BUBBA:
        printf("eaten by bubba\n");
        break;
    case ACT_PEACH_CUTSCENE:
        printf("peach cutscene\n");
        break;
    case ACT_CREDITS:
        printf("credits\n");
        break;
    case ACT_WAVING:
        printf("waving\n");
        break;
    case ACT_PULLING_DOOR:
        printf("pulling door\n");
        break;
    case ACT_PUSHING_DOOR:
        printf("pushing door\n");
        break;
    case ACT_WARP_DOOR_SPAWN:
        printf("warp door spawn\n");
        break;
    case ACT_EMERGE_FROM_PIPE:
        printf("emerge from pipe\n");
        break;
    case ACT_SPAWN_SPIN_AIRBORNE:
        printf("spawn spin airborne\n");
        break;
    case ACT_SPAWN_SPIN_LANDING:
        printf("spawn spin landing\n");
        break;
    case ACT_EXIT_AIRBORNE:
        printf("exit airborne\n");
        break;
    case ACT_EXIT_LAND_SAVE_DIALOG:
        printf("exit land save dialog\n");
        break;
    case ACT_DEATH_EXIT:
        printf("death exit\n");
        break;
    case ACT_DEATH_EXIT_(UNUSED):
        printf("death exit (unused)\n");
        break;
    case ACT_FALLING_DEATH_EXIT:
        printf("falling death exit\n");
        break;
    case ACT_SPECIAL_EXIT_AIRBORNE:
        printf("special exit airborne\n");
        break;
    case ACT_SPECIAL_DEATH_EXIT:
        printf("special death exit\n");
        break;
    case ACT_FALLING_EXIT_AIRBORNE:
        printf("falling exit airborne\n");
        break;
    case ACT_UNLOCKING_KEY_DOOR:
        printf("unlocking key door\n");
        break;
    case ACT_UNLOCKING_STAR_DOOR:
        printf("unlocking star door\n");
        break;
    case ACT_ENTERING_STAR_DOOR:
        printf("entering star door\n");
        break;
    case ACT_SPAWN_NO_SPIN_AIRBORNE:
        printf("spawn no spin airborne\n");
        break;
    case ACT_SPAWN_NO_SPIN_LANDING:
        printf("spawn no spin landing\n");
        break;
    case ACT_BBH_ENTER_JUMP:
        printf("bbh enter jump\n");
        break;
    case ACT_BBH_ENTER_SPIN:
        printf("bbh enter spin\n");
        break;
    case ACT_TELEPORT_FADE_OUT:
        printf("teleport fade out\n");
        break;
    case ACT_TELEPORT_FADE_IN:
        printf("teleport fade in\n");
        break;
    case ACT_SHOCKED:
        printf("shocked\n");
        break;
    case ACT_SQUISHED:
        printf("squished\n");
        break;
    case ACT_HEAD_STUCK_IN_GROUND:
        printf("head stuck in ground\n");
        break;
    case ACT_BUTT_STUCK_IN_GROUND:
        printf("butt stuck in ground\n");
        break;
    case ACT_FEET_STUCK_IN_GROUND:
        printf("feet stuck in ground\n");
        break;
    case ACT_PUTTING_ON_CAP:
        printf("putting on cap\n");
        break;
    case ACT_HOLDING_POLE:
        printf("holding pole\n");
        break;
    case ACT_GRAB_POLE_SLOW:
        printf("grab pole slow\n");
        break;
    case ACT_GRAB_POLE_FAST:
        printf("grab pole fast\n");
        break;
    case ACT_CLIMBING_POLE:
        printf("climbing pole\n");
        break;
    case ACT_TOP_OF_POLE_TRANSITION:
        printf("top of pole transition\n");
        break;
    case ACT_TOP_OF_POLE:
        printf("top of pole\n");
        break;
    case ACT_START_HANGING:
        printf("start hanging\n");
        break;
    case ACT_HANGING:
        printf("hanging\n");
        break;
    case ACT_HANG_MOVING:
        printf("hang moving\n");
        break;
    case ACT_LEDGE_GRAB:
        printf("ledge grab\n");
        break;
    case ACT_LEDGE_CLIMB_SLOW_1:
        printf("ledge climb slow 1\n");
        break;
    case ACT_LEDGE_CLIMB_SLOW_2:
        printf("ledge climb slow 2\n");
        break;
    case ACT_LEDGE_CLIMB_DOWN:
        printf("ledge climb down\n");
        break;
    case ACT_LEDGE_CLIMB_FAST:
        printf("ledge climb fast\n");
        break;
    case ACT_GRABBED:
        printf("grabbed\n");
        break;
    case ACT_IN_CANNON:
        printf("in cannon\n");
        break;
    case ACT_TORNADO_TWIRLING:
        printf("tornado twirling\n");
        break;
    case ACT_PUNCHING:
        printf("punching\n");
        break;
    case ACT_PICKING_UP:
        printf("picking up\n");
        break;
    case ACT_DIVE_PICKING_UP:
        printf("dive picking up\n");
        break;
    case ACT_STOMACH_SLIDE_STOP:
        printf("stomach slide stop\n");
        break;
    case ACT_PLACING_DOWN:
        printf("placing down\n");
        break;
    case ACT_THROWING:
        printf("throwing\n");
        break;
    case ACT_HEAVY_THROW:
        printf("heavy throw\n");
        break;
    case ACT_PICKING_UP_BOWSER:
        printf("picking up bowser\n");
        break;
    case ACT_HOLDING_BOWSER:
        printf("holding bowser\n");
        break;
    case ACT_RELEASING_BOWSER:
        printf("releasing bowser\n");
        break;
    }

}

