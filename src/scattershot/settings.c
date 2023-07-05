#include "scattershot/so.h"
#include "scattershot/settings.h"
#include "scattershot/util.h"

//Experimental: penalize blocks that only lead to breaks
int penalProb = 0;
int minProb = 5;
int breakCountFrame = 20;

int usereftas = 0; //bool if use reference tas
float pad = 25.0f; //how far away max from reference tas

//problem specific variables
char input_m64[128];
char dllBaseName[128];
int startFrame = 100930; //22630; 
int startFrameCourse = 100930; //22440; 
int switchtoUPDATE = 100930; //22473; //135218; //when to use the other update function, needed when you interact with object before start
int readLength = 101100; //22800; //135255; //135300; 
int max_frames = 250; //max frames to bruteforce
int update_max_frames = 1; //boolean if update max_frames to fastes found tas
float COARSENESS  = 25.0f;

//otehr
int merge_mod = 500;
int garbage_mod = 3000;
int subLoopMax = 100;
int megarand_mod = 2; //inverse probability for pseudorandom input instead of best_pract

//configuration
int totThreads = 4;
int segLength = 20;
//
int maxSegs = 1024;
int maxBlocks = 2000000;
int maxHashes = 20000000; //10 * maxBlocks;
int maxSharedBlocks = 20000000;
int maxSharedHashes = 200000000; //10 * maxSharedBlocks;
int maxSharedSegs = 25000000;
int maxLocalSegs = 2000000;

int gAreaCount = 100;
AREA areas[100];

int gWhiteListCount = 0;
CONSTR gWhiteList[20];

int gBlackListCount = 0;
CONSTR gBlackList[20];

STATECOND writeCond[1];

//tunnel mode stuff test
Vec3df reachedPos[500];
QUBOID cubePos[500] = {0};

//can be found automatically apparently
#define MARIO_CASTLE_GROUND_OBJ_INX 96
#define MARIO_SL_OBJ_INX 19

int MARIO_OBJ_INX = MARIO_SL_OBJ_INX;
