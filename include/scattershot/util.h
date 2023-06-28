#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> //va_start
#include <stdbool.h>
                    
#ifdef __linux__
#include <errno.h>
#include <link.h>
#include <unistd.h>
#else
#include <windows.h>
#include <dbghelp.h>
#include <winbase.h>
#endif

//not sure if 
#include <fcntl.h>

#include "PR/ultratypes.h"

extern char dataMap[8192];
extern char bssMap[8192];

extern int dataStart, dataLength, bssStart, bssLength;
extern int gPrint, gLog;
extern char gProgName[192];
extern FILE *gLogFP;

#define CONT_A 0x8000
#define CONT_B 0x4000
#define CONT_Z 0x2000
#define CONT_CRIGHT 0x0001
#define CONT_CLEFT 0x0002
#define CONT_CDOWN 0x0004
#define CONT_CUP 0x0008
#define CONT_START 0x1000
#define CONT_R 0x0010
#define CONT_L 0x0020
#define CONT_DRIGHT 0x0100
#define CONT_DLEFT 0x0200
#define CONT_DDOWN 0x0400
#define CONT_DUP 0x0800

#define ACT_UNINITIALIZED 0x0
#define ACT_IDLE 0x1
#define ACT_START_SLEEPING 0x2
#define ACT_SLEEPING 0x3
#define ACT_WAKING_UP 0x4
#define ACT_PANTING 0x5
#define ACT_HOLD_PANTING_(UNUSED) 0x6
#define ACT_HOLD_IDLE 0x7
#define ACT_HOLD_HEAVY_IDLE 0x8
#define ACT_STANDING_AGAINST_WALL 0x9
#define ACT_COUGHING 0xa
#define ACT_SHIVERING 0xb
#define ACT_IN_QUICKSAND 0xd
#define ACT_UNKNOWN_2020E 0xe
#define ACT_CROUCHING 0x20
#define ACT_START_CROUCHING 0x21
#define ACT_STOP_CROUCHING 0x22
#define ACT_START_CRAWLING 0x23
#define ACT_STOP_CRAWLING 0x24
#define ACT_SLIDE_KICK_SLIDE_STOP 0x25
#define ACT_SHOCKWAVE_BOUNCE 0x26
#define ACT_FIRST_PERSON 0x27
#define ACT_BACKFLIP_LAND_STOP 0x2f
#define ACT_JUMP_LAND_STOP 0x30
#define ACT_DOUBLE_JUMP_LAND_STOP 0x31
#define ACT_FREEFALL_LAND_STOP 0x32
#define ACT_SIDE_FLIP_LAND_STOP 0x33
#define ACT_HOLD_JUMP_LAND_STOP 0x34
#define ACT_HOLD_FREEFALL_LAND_STOP 0x35
#define ACT_AIR_THROW_LAND 0x36
#define ACT_TWIRL_LAND 0x38
#define ACT_LAVA_BOOST_LAND 0x39
#define ACT_TRIPLE_JUMP_LAND_STOP 0x3a
#define ACT_LONG_JUMP_LAND_STOP 0x3b
#define ACT_GROUND_POUND_LAND 0x3c
#define ACT_BRAKING_STOP 0x3d
#define ACT_BUTT_SLIDE_STOP 0x3e
#define ACT_HOLD_BUTT_SLIDE_STOP 0x3f
#define ACT_WALKING 0x40
#define ACT_HOLD_WALKING 0x42
#define ACT_TURNING_AROUND 0x43
#define ACT_FINISH_TURNING_AROUND 0x44
#define ACT_BRAKING 0x45
#define ACT_RIDING_SHELL_GROUND 0x46
#define ACT_HOLD_HEAVY_WALKING 0x47
#define ACT_CRAWLING 0x48
#define ACT_BURNING_GROUND 0x49
#define ACT_DECELERATING 0x4a
#define ACT_HOLD_DECELERATING 0x4b
#define ACT_BEGIN_SLIDING 0x50
#define ACT_HOLD_BEGIN_SLIDING 0x51
#define ACT_BUTT_SLIDE 0x52
#define ACT_STOMACH_SLIDE 0x53
#define ACT_HOLD_BUTT_SLIDE 0x54
#define ACT_HOLD_STOMACH_SLIDE 0x55
#define ACT_DIVE_SLIDE 0x56
#define ACT_MOVE_PUNCHING 0x57
#define ACT_CROUCH_SLIDE 0x59
#define ACT_SLIDE_KICK_SLIDE 0x5a
#define ACT_HARD_BACKWARD_GROUND_KB 0x60
#define ACT_HARD_FORWARD_GROUND_KB 0x61
#define ACT_BACKWARD_GROUND_KB 0x62
#define ACT_FORWARD_GROUND_KB 0x63
#define ACT_SOFT_BACKWARD_GROUND_KB 0x64
#define ACT_SOFT_FORWARD_GROUND_KB 0x65
#define ACT_GROUND_BONK 0x66
#define ACT_DEATH_EXIT_LAND 0x67
#define ACT_JUMP_LAND 0x70
#define ACT_FREEFALL_LAND 0x71
#define ACT_DOUBLE_JUMP_LAND 0x72
#define ACT_SIDE_FLIP_LAND 0x73
#define ACT_HOLD_JUMP_LAND 0x74
#define ACT_HOLD_FREEFALL_LAND 0x75
#define ACT_QUICKSAND_JUMP_LAND 0x76
#define ACT_HOLD_QUICKSAND_JUMP_LAND 0x77
#define ACT_TRIPLE_JUMP_LAND 0x78
#define ACT_LONG_JUMP_LAND 0x79
#define ACT_BACKFLIP_LAND 0x7a
#define ACT_JUMP 0x80
#define ACT_DOUBLE_JUMP 0x81
#define ACT_TRIPLE_JUMP 0x82
#define ACT_BACKFLIP 0x83
#define ACT_STEEP_JUMP 0x85
#define ACT_WALL_KICK_AIR 0x86
#define ACT_SIDE_FLIP 0x87
#define ACT_LONG_JUMP 0x88
#define ACT_WATER_JUMP 0x89
#define ACT_DIVE 0x8a
#define ACT_FREEFALL 0x8c
#define ACT_TOP_OF_POLE_JUMP 0x8d
#define ACT_BUTT_SLIDE_AIR 0x8e
#define ACT_FLYING_TRIPLE_JUMP 0x94
#define ACT_SHOT_FROM_CANNON 0x98
#define ACT_FLYING 0x99
#define ACT_RIDING_SHELL_JUMP 0x9a
#define ACT_RIDING_SHELL_FALL 0x9b
#define ACT_VERTICAL_WIND 0x9c
#define ACT_HOLD_JUMP 0xa0
#define ACT_HOLD_FREEFALL 0xa1
#define ACT_HOLD_BUTT_SLIDE_AIR 0xa2
#define ACT_HOLD_WATER_JUMP 0xa3
#define ACT_TWIRLING 0xa4
#define ACT_FORWARD_ROLLOUT 0xa6
#define ACT_AIR_HIT_WALL 0xa7
#define ACT_RIDING_HOOT 0xa8
#define ACT_GROUND_POUND 0xa9
#define ACT_SLIDE_KICK 0xaa
#define ACT_AIR_THROW 0xab
#define ACT_JUMP_KICK 0xac
#define ACT_BACKWARD_ROLLOUT 0xad
#define ACT_CRAZY_BOX_BOUNCE 0xae
#define ACT_SPECIAL_TRIPLE_JUMP 0xaf
#define ACT_BACKWARD_AIR_KB 0xb0
#define ACT_FORWARD_AIR_KB 0xb1
#define ACT_HARD_FORWARD_AIR_KB 0xb2
#define ACT_HARD_BACKWARD_AIR_KB 0xb3
#define ACT_BURNING_JUMP 0xb4
#define ACT_BURNING_FALL 0xb5
#define ACT_SOFT_BONK 0xb6
#define ACT_LAVA_BOOST 0xb7
#define ACT_GETTING_BLOWN 0xb8
#define ACT_THROWN_FORWARD 0xbd
#define ACT_THROWN_BACKWARD 0xbe
#define ACT_WATER_IDLE 0xc0
#define ACT_HOLD_WATER_IDLE 0xc1
#define ACT_WATER_ACTION_END 0xc2
#define ACT_HOLD_WATER_ACTION_END 0xc3
#define ACT_DROWNING 0xc4
#define ACT_BACKWARD_WATER_KB 0xc5
#define ACT_FORWARD_WATER_KB 0xc6
#define ACT_WATER_DEATH 0xc7
#define ACT_WATER_SHOCKED 0xc8
#define ACT_BREASTSTROKE 0xd0
#define ACT_SWIMMING_END 0xd1
#define ACT_FLUTTER_KICK 0xd2
#define ACT_HOLD_BREASTSTROKE 0xd3
#define ACT_HOLD_SWIMMING_END 0xd4
#define ACT_HOLD_FLUTTER_KICK 0xd5
#define ACT_WATER_SHELL_SWIMMING 0xd6
#define ACT_WATER_THROW 0xe0
#define ACT_WATER_PUNCH 0xe1
#define ACT_WATER_PLUNGE 0xe2
#define ACT_CAUGHT_IN_WHIRLPOOL 0xe3
#define ACT_METAL_WATER_STANDING 0xf0
#define ACT_HOLD_METAL_WATER_STANDING 0xf1
#define ACT_METAL_WATER_WALKING 0xf2
#define ACT_HOLD_METAL_WATER_WALKING 0xf3
#define ACT_METAL_WATER_FALLING 0xf4
#define ACT_HOLD_METAL_WATER_FALLING 0xf5
#define ACT_METAL_WATER_FALL_LAND 0xf6
#define ACT_HOLD_METAL_WATER_FALL_LAND 0xf7
#define ACT_METAL_WATER_JUMP 0xf8
#define ACT_HOLD_METAL_WATER_JUMP 0xf9
#define ACT_METAL_WATER_JUMP_LAND 0xfa
#define ACT_HOLD_METAL_WATER_JUMP_LAND 0xfb
#define ACT_DISAPPEARED 0x100
#define ACT_INTRO_CUTSCENE 0x101
#define ACT_STAR_DANCE_EXIT 0x102
#define ACT_STAR_DANCE_WATER 0x103
#define ACT_FALL_AFTER_STAR_GRAB 0x104
#define ACT_READING_AUTOMATIC_DIALOG 0x105
#define ACT_READING_NPC_DIALOG 0x106
#define ACT_STAR_DANCE_NO_EXIT 0x107
#define ACT_READING_SIGN 0x108
#define ACT_GRAND_STAR_CUTSCENE 0x109
#define ACT_WAITING_FOR_DIALOG 0x10a
#define ACT_DEBUG_FREE_MOVE 0x10f
#define ACT_STANDING_DEATH 0x111
#define ACT_QUICKSAND_DEATH 0x112
#define ACT_ELECTROCUTION 0x113
#define ACT_SUFFOCATION 0x114
#define ACT_DEATH_ON_STOMACH 0x115
#define ACT_DEATH_ON_BACK 0x116
#define ACT_EATEN_BY_BUBBA 0x117
#define ACT_PEACH_CUTSCENE 0x118
#define ACT_CREDITS 0x119
#define ACT_WAVING 0x11a
#define ACT_PULLING_DOOR 0x120
#define ACT_PUSHING_DOOR 0x121
#define ACT_WARP_DOOR_SPAWN 0x122
#define ACT_EMERGE_FROM_PIPE 0x123
#define ACT_SPAWN_SPIN_AIRBORNE 0x124
#define ACT_SPAWN_SPIN_LANDING 0x125
#define ACT_EXIT_AIRBORNE 0x126
#define ACT_EXIT_LAND_SAVE_DIALOG 0x127
#define ACT_DEATH_EXIT 0x128
#define ACT_DEATH_EXIT_(UNUSED) 0x129
#define ACT_FALLING_DEATH_EXIT 0x12a
#define ACT_SPECIAL_EXIT_AIRBORNE 0x12b
#define ACT_SPECIAL_DEATH_EXIT 0x12c
#define ACT_FALLING_EXIT_AIRBORNE 0x12d
#define ACT_UNLOCKING_KEY_DOOR 0x12e
#define ACT_UNLOCKING_STAR_DOOR 0x12f
#define ACT_ENTERING_STAR_DOOR 0x131
#define ACT_SPAWN_NO_SPIN_AIRBORNE 0x132
#define ACT_SPAWN_NO_SPIN_LANDING 0x133
#define ACT_BBH_ENTER_JUMP 0x134
#define ACT_BBH_ENTER_SPIN 0x135
#define ACT_TELEPORT_FADE_OUT 0x136
#define ACT_TELEPORT_FADE_IN 0x137
#define ACT_SHOCKED 0x138
#define ACT_SQUISHED 0x139
#define ACT_HEAD_STUCK_IN_GROUND 0x13a
#define ACT_BUTT_STUCK_IN_GROUND 0x13b
#define ACT_FEET_STUCK_IN_GROUND 0x13c
#define ACT_PUTTING_ON_CAP 0x13d
#define ACT_HOLDING_POLE 0x140
#define ACT_GRAB_POLE_SLOW 0x141
#define ACT_GRAB_POLE_FAST 0x142
#define ACT_CLIMBING_POLE 0x143
#define ACT_TOP_OF_POLE_TRANSITION 0x144
#define ACT_TOP_OF_POLE 0x145
#define ACT_START_HANGING 0x148
#define ACT_HANGING 0x149
#define ACT_HANG_MOVING 0x14a
#define ACT_LEDGE_GRAB 0x14b
#define ACT_LEDGE_CLIMB_SLOW_1 0x14c
#define ACT_LEDGE_CLIMB_SLOW_2 0x14d
#define ACT_LEDGE_CLIMB_DOWN 0x14e
#define ACT_LEDGE_CLIMB_FAST 0x14f
#define ACT_GRABBED 0x170
#define ACT_IN_CANNON 0x171
#define ACT_TORNADO_TWIRLING 0x172
#define ACT_PUNCHING 0x180
#define ACT_PICKING_UP 0x183
#define ACT_DIVE_PICKING_UP 0x185
#define ACT_STOMACH_SLIDE_STOP 0x186
#define ACT_PLACING_DOWN 0x187
#define ACT_THROWING 0x188
#define ACT_HEAVY_THROW 0x189
#define ACT_PICKING_UP_BOWSER 0x190
#define ACT_HOLDING_BOWSER 0x191
#define ACT_RELEASING_BOWSER 0x192

//old names can be deleted
#define ACT_STANING_AGAINST_WALL 0x009
#define ACT_DR_LAND 0x032
#define ACT_WALK 0x040
#define ACT_TURNAROUND_1 0x043
#define ACT_TURNAROUND_2 0x044
#define ACT_BRAKE 0x045
#define ACT_DIVE_LAND 0x056
#define ACT_DR 0x0A6

//dumb
#define DEG0 0
#define DEG90 16384
#define DEG180 32768
#define DEG270 49152
#define DEG360 65536

#ifdef __linux__
#define CALLBACK //placeholder
#endif

typedef void (CALLBACK* VOIDFUNC)();
//typedef void (CALLBACK* RENDERFUNC)(uint32_t, uint32_t, struct GfxRenderingAPI *);
typedef void* (CALLBACK* GFXFUNC)(int, void *, void *, void *);
typedef s32 (CALLBACK* EXECFUNC)(void *);

typedef struct  {
    /*0x00 0x38*/ s16 animID;
    /*0x02 0x3A*/ s16 animYTrans;
    /*0x04 0x3C*/ struct Animation *curAnim;
    /*0x08 0x40*/ s16 animFrame;
    /*0x0A 0x42*/ u16 animTimer;
    /*0x0C 0x44*/ s32 animFrameAccelAssist;
    /*0x10 0x48*/ s32 animAccel;
}AnimInfo;

typedef s16 (* GEOFUNC)(AnimInfo* , s32 *);

//for defining areas
typedef struct  {
    float x1; 
    float z1; 
    float x2; 
    float z2; 
    float x3; 
    float z3; 
    float min_y; 
    float max_y; 
} TRIANGLE;

typedef struct  {
    float min_x; 
    float max_x; 
    float min_y; 
    float max_y; 
    float min_z; 
    float max_z;
} QUBOID;

typedef struct {
    unsigned short b;
    char x;
    char y;
} Input;

typedef struct {
    void *data;
    void *bss;
} SaveState;

typedef struct {
    f32 x;
	f32 y;
	f32 z;
} Vec3f;


//controller shid
typedef struct {
	u16     type;                   /* Controller Type */
	u8      status;                 /* Controller status */
	u8      errnum;
}OSContStatus;

typedef struct {
	u16     button;
	s8      stick_x;		/* -80 <= stick_x <= 80 */
	s8      stick_y;		/* -80 <= stick_y <= 80 */
	u8      errnum;
} OSContPad;


typedef struct  {
  /*0x00*/ s16 rawStickX;       //
  /*0x02*/ s16 rawStickY;       //
  /*0x04*/ float stickX;        // [-64, 64] positive is right
  /*0x08*/ float stickY;        // [-64, 64] positive is up
  /*0x0C*/ float stickMag;      // distance from center [0, 64]
  /*0x10*/ u16 buttonDown;
  /*0x12*/ u16 buttonPressed;
  /*0x14*/ OSContStatus *statusData;
  /*0x18*/ OSContPad *controllerData;
}Controller ;

typedef struct {
    /*0x00*/ u8 mode; // What type of mode the camera uses (see defines above)
    /*0x01*/ u8 defMode;
    /**
     * Determines what direction Mario moves in when the analog stick is moved.
     *
     * @warning This is NOT the camera's xz-rotation in world space. This is the angle calculated from the
     *          camera's focus TO the camera's position, instead of the other way around like it should
     *          be. It's effectively the opposite of the camera's actual yaw. Use
     *          vec3f_get_dist_and_angle() if you need the camera's yaw.
     */
    /*0x02*/ s16 yaw;
    /*0x04*/ Vec3f focus;
    /*0x10*/ Vec3f pos;
    /*0x1C*/ Vec3f unusedVec1;
    /// The x coordinate of the "center" of the area. The camera will rotate around this point.
    /// For example, this is what makes the camera rotate around the hill in BoB
    /*0x28*/ f32 areaCenX;
    /// The z coordinate of the "center" of the area. The camera will rotate around this point.
    /// For example, this is what makes the camera rotate around the hill in BoB
    /*0x2C*/ f32 areaCenZ;
    /*0x30*/ u8 cutscene;
    /*0x31*/ u8 filler1[8];
    /*0x3A*/ s16 nextYaw;
    /*0x3C*/ u8 filler2[40];
    /*0x64*/ u8 doorStatus;
    /// The y coordinate of the "center" of the area. Unlike areaCenX and areaCenZ, this is only used
    /// when paused. See zoom_out_if_paused_and_outside
    /*0x68*/ f32 areaCenY;
} Camera;


typedef void (* CAMFUNC)(Camera*);

typedef struct Segment Segment;

struct Segment {
	Segment *parent;
	uint64_t seed;
	uint32_t refCount; 
	uint8_t numFrames; 
	uint8_t depth;     //init'ed to 1 and then incremented by 1 per frame
};

//fifd: Vec3d actually has 4 dimensions, where the first 3 are spatial and
//the 4th encodes a lot of information about Mario's state (actions, speed,
//camera) as well as some button information. These vectors specify a part
//of state space.
typedef struct {
    uint8_t x;
	uint8_t y;
	uint8_t z;
	uint64_t s;
} Vec3d;


//fifd: I think this is an element of the partition of state
//space. Will need to understand what each of its fields are
typedef struct {
    Vec3d         pos;    //fifd: an output of truncFunc. Identifies which block this is
    float         value;    //fifd: a fitness of the best TAS that reaches this block; the higher the better.
    u8 prob;
	Segment *tailSeg;
//Time is most important component of value but keeps higher hspeed if time is tied
} Block;

typedef struct {
    float x, y, z;
    int actTrunc;
    unsigned short yawFacing;
	float hspd;
} FinePos;

typedef struct {
    char* fname;
    uint64_t base_addr;
} soStruct;


int cp(const char *from, const char *to);
void writeFile(char *new_, const char *base, Input *inputs, int offset, int length);
void copyDll(char *new_, char *base);
void printfQ(const char* format, ...);
void flushLog();
uint32_t xoro_r(uint64_t *s);
void orStates(SaveState *s1, SaveState *s2);
void xorStates(const SaveState *s1, const SaveState *s2, SaveState *s3, int tid);
void getDllInfo(char *file_name);
void allocState(SaveState *s);
void allocStateSmall(SaveState *s);
void freeState(SaveState *s);
void load(void* hDLL, SaveState *s);
void riskyLoad(void* hDLL, SaveState *s);
void riskyLoadJ(void* hDLL, SaveState *s);
void riskyLoad2(void* hDLL, SaveState *s);
void riskyLoad3(void* hDLL, SaveState *s);
void save(void* hDLL, SaveState *s);
int find_sm64so_base(struct dl_phdr_info *info, size_t size, void *arg);
void print_xored_states(SaveState* xorSlaves, int tid);
bool in_quboid(float x, float y, float z, QUBOID* qu);
bool in_triangle(float x, float y, float z, TRIANGLE* tr);
void printFields();
void initFields();

#endif
