#include "scattershot/so.h"
#include "scattershot/settings.h"
//Experimental: penalize blocks that only lead to breaks
const u8 penalProb = 50;
const u8 minProb = 10;
const u8 breakCountFrame = 4;

//problem specific variables
const char* input_m64 = "input.m64";
const int startFrame = 100930; //ingame frame -1??
const int readLength = 102300; //+5
int max_frames = 16*30; //max frames to bruteforce

//otehr
const int merge_mod = 500;
const int garbage_mod = 3000;
const int subLoopMax = 100;
const int megarand_mod = 2; //inverse probability for pseudorandom input instead of best_pract

//configuration
const int totThreads = 4;
const int segLength = 20;
const int maxSegs = 1024;
const int maxBlocks = 2000000;
const int maxHashes = 20000000; //10 * maxBlocks;
const int maxSharedBlocks = 20000000;
const int maxSharedHashes = 200000000; //10 * maxSharedBlocks;
const int maxSharedSegs = 25000000;
const int maxLocalSegs = 2000000;

#define MARIO_CASTLE_GROUND_OBJ_INX 96
#define MARIO_SL_OBJ_INX 19

const int MARIO_OBJ_INX = MARIO_SL_OBJ_INX;
