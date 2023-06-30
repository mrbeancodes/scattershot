#include "scattershot/so.h"
#include "scattershot/settings.h"
//Experimental: penalize blocks that only lead to breaks
const u8 penalProb = 0;
const u8 minProb = 5;
const u8 breakCountFrame = 20;

//problem specific variables
const char* input_m64 = "input.m64";
const int startFrame = 100930; //135170; //ingame frame -1??
const int startFrameCourse = 100930; //doesnt have to be exact
const int readLength = 101100; //135300; 
int max_frames = 250; //max frames to bruteforce
const int update_max_frames = 1; //boolean if update max_frames to fastes found tas
const float COARSENESS  = 25.0f;

//otehr
const int merge_mod = 500;
const int garbage_mod = 3000;
const int subLoopMax = 100;
const int megarand_mod = 2; //inverse probability for pseudorandom input instead of best_pract

//configuration
const int totThreads = 4;
const int segLength = 20;
//
const int maxSegs = 1024;
const int maxBlocks = 2000000;
const int maxHashes = 20000000; //10 * maxBlocks;
const int maxSharedBlocks = 20000000;
const int maxSharedHashes = 200000000; //10 * maxSharedBlocks;
const int maxSharedSegs = 25000000;
const int maxLocalSegs = 2000000;


//can be found automatically apparently
#define MARIO_CASTLE_GROUND_OBJ_INX 96
#define MARIO_SL_OBJ_INX 19

const int MARIO_OBJ_INX = MARIO_SL_OBJ_INX;
