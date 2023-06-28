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

    //TODO is this correct?
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
    
    so->marioAnimInfo = (AnimInfo *)((char *)so->gObjectPool + MARIO_OBJ_INX * 1392 + 0x50);

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
    //so->update_camera(*so->gCamera); // Optional
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

