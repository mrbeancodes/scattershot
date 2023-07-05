#ifndef SETTINGS_H
#define SETTINGS_H

#include "so.h"
#include "util.h"

//Experimental: penalize blocks that only lead to breaks
extern int penalProb;
extern int minProb;
extern int breakCountFrame;

extern int usereftas;
extern float pad;

//problem specific variables
extern char input_m64[128];
extern char dllBaseName[128];
extern int startFrame; //ingame frame -1??
extern int startFrameCourse;
extern int switchtoUPDATE;
extern int readLength; //+a couple frames
extern int max_frames; //max frames to bruteforce
extern int update_max_frames;

//other
extern int merge_mod;
extern int garbage_mod;
extern int subLoopMax;
extern int megarand_mod; //inverse probability for pseudorandom input instead of best_pract

//configuration
extern int totThreads;
extern int segLength;
extern int maxSegs;
extern int maxBlocks;
extern int maxHashes;
extern int maxSharedBlocks;
extern int maxSharedHashes;
extern int maxSharedSegs;
extern int maxLocalSegs;

//statespace
extern float COARSENESS;

extern int gAreaCount;
extern AREA areas[100];
extern CONSTR gWhiteList[20];
extern CONSTR gBlackList[20];
extern int gWhiteListCount;
extern int gBlackListCount;

extern STATECOND writeCond[1];

extern Vec3df reachedPos[500];
extern QUBOID cubePos[500];

//maybe unnecessary
extern int MARIO_OBJ_INX; 


#endif
