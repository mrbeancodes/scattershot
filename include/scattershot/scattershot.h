#ifndef SCATTERSHOT_H
#define SCATTERSHOT_H

#include "so.h"
#include <math.h>

void perturbInput(Input *in, uint64_t *seed, int frame,SO* so, int megaRandom);
Vec3d truncFunc(SO* so);
int truncEq(Vec3d a, Vec3d b);
int blockLength(Block a);
float dist3d(float a, float b, float c, float x, float y, float z);
int angDist(unsigned short a, unsigned short b);
int leftLine(float a, float b, float c, float d, float e, float f);
uint64_t hashPos(Vec3d pos);
int findNewHashInx(int *hashTab, int maxHashes, Vec3d pos);
int findBlock(Block *blocks, int *hashTab, int maxHashes, Vec3d pos, int nMin, int nMax);

void merge(int* nBlocks, Block* allBlocks,Block* sharedBlocks, int* allHashTabs,int* sharedHashTab, struct Segment ** allSegs, int* numSegs );
void segment_garbage_collection(struct Segment ** allSegs, int* numSegs, Block* sharedBlocks, int* nBlocks);
int pick_block_index(int* nBlocks, int mainLoop,Block* sharedBlocks, int* sharedHashTab, Vec3d *lightningList, uint64_t* seed,int lightLen);
void set_new_seg(Segment* newSeg, Block* origBlock,Block* newBlock,Block* blocks,int* nBlocks, Segment** allSegs, int* numSegs ,uint64_t origSeed, int tid, int f, int refCount);

#endif
