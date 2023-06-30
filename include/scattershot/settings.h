#ifndef SETTINGS_H
#define SETTINGS_H

#include "so.h"
#include "util.h"
//Experimental: penalize blocks that only lead to breaks
extern const u8 penalProb;
extern const u8 minProb;
extern const u8 breakCountFrame;

//problem specific variables
extern const char* input_m64;
extern const int startFrame; //ingame frame -1??
extern const int startFrameCourse;
extern const int readLength; //+a couple frames
extern int max_frames; //max frames to bruteforce
extern const int update_max_frames;

//otehr
extern const int merge_mod;
extern const int garbage_mod;
extern const int subLoopMax;
extern const int megarand_mod; //inverse probability for pseudorandom input instead of best_pract

//configuration
extern const int    totThreads;
extern const int segLength;
extern const int maxSegs;
extern const int maxBlocks;
extern const int maxHashes;
extern const int maxSharedBlocks;
extern const int maxSharedHashes;
extern const int maxSharedSegs;
extern const int maxLocalSegs;

extern const int MARIO_OBJ_INX; 

extern const float COARSENESS;

#endif
