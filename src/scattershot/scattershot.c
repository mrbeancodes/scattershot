#include "scattershot/scattershot.h"
#include "scattershot/angles.h"
#include "scattershot/settings.h"

#define COARSENESS 10.0f

//a couple of hints for scattershot to use sometimes
bool best_practice(Input *in, uint64_t* seed,unsigned int actTrunc, int frame, SO* so) {
    //TODO
    //speedkick
    //fast 0 input brakes
    //speedpunch
    //dont do these dumb random jumps and keep holding a
    
    bool ret=0;
    ////probably walking against wall
    if (actTrunc == ACT_WALKING && *so->marioHSpd == 6.0f) {
        in->b = 0;
        in->x = 0;
        in->y = 0;
        return 1;
    }

    ////dive if jump action and 31.5-32 speed
    if (actTrunc == ACT_JUMP && *so->marioHSpd >= 31.5f && *so->marioHSpd <= 32.0f) {
        in->b = 0;
        in->b |= CONT_B;
        return 1;
    }

    ////first frame rollout usually good, TODO when no slippery fast ground
    if (actTrunc == ACT_DIVE_SLIDE) {
        in->b |= CONT_B;
        in->b |= CONT_A;
        return 1;
    }

    //not pressing a or b while in dive action
    if (actTrunc == ACT_DIVE) {
        in->b &= ~CONT_B;
        in->b &= ~CONT_A;
        return 1;
    }

    //holding a while forward rollout for speedkick
    //but sometimes not useful if want to jump 
    if (actTrunc == ACT_FORWARD_ROLLOUT && *so->marioHSpd > 32.0f) {
        int random = xoro_r(seed) % 10;
        if (random < 8) {
            in->b = 0;
            in->b |= CONT_A;
            return 1;
        }
    }

    //TODO speedkicking after landing, needs proper magnitude calculation
    if (actTrunc == ACT_FREEFALL_LAND_STOP && *so->marioHSpd > 32.0f) {
        if(in->b & CONT_A) {
            in->b = 0;
            in->b |= CONT_A;
            in->b |= CONT_B;
            int tarAng = (int)*so->marioYawFacing - (int)*so->camYaw;
            match_yaw(in, tarAng, false);
            if(in->x > 39) in->x = 39;
            if(in->x < -39) in->x = -39;
            if(in->y > 39) in->y = 39;
            if(in->y < -39) in->y = -39;
            return 1;
        }
    }
    //dont kick when holding a in single jump mode ffs
    if (actTrunc == ACT_JUMP && *so->marioYVel > 16.0f) {
        in->b &= ~CONT_B;
        ret = 1;
    }

    //walking is kinda slow
    if (actTrunc == ACT_WALKING) {
        int random = xoro_r(seed) % 100;
        //press z for lj if > 10.64 speed
        if (*so->marioHSpd > 10.64f && *so->marioHSpd < 40.0f) {
            if (random < 30) {
                in->b = 0;
                in->b |= CONT_Z;
            }
        }
        //prime time for jump time
        else if(*so->marioHSpd < 10.0f && *so->marioHSpd > 8.8f) {
            if (random < 50) {
                in->b = 0;
                in->b |= CONT_A;
            }
        }
        ////jump or jumpdive or TODO dive
        //else if (*so->marioHSpd > 40.0f) { //jumpdive with ~47h
        //    if (random < 30) {
        //        in->b = 0;
        //        in->b |= CONT_A;
        //        if(random < 15) {
        //            in->b |= CONT_B;
        //        }
        //    }
        //}
        ret = 1;
    }

    //if (actTrunc == ACT_CROUCH_SLIDE) {
    //    //speedy slidekick
    //    if (*so->marioHSpd > 48.0f) {
    //        in->b = 0;
    //        in->b |= CONT_B;
    //    }
    //    //slidekick only makes sense with little and a lot of speed
    //    else if (*so->marioHSpd < 12.0f && *so->marioHSpd >= 10.0f){ 
    //        bool random = xoro_r(seed) % 2;
    //        //press z for lj if > 10.64 speed
    //        in->b = 0;
    //        if (random) in->b |= CONT_A;
    //        else        in->b |= CONT_A;
    //    }
    //    //speed over 12 but less than 48 -> LJ usually best
    //    else {
    //        in->b = 0;
    //        in->b |= CONT_A;
    //    }
    //    ret = 1;
    //}

    ////triple jump
    //if (actTrunc == ACT_DOUBLE_JUMP_LAND && *so->marioHSpd >= 16.0f) {
    //    if(xoro_r(seed) % 2 == 0) {
    //        in->b = 0;
    //        in->b |= CONT_A;
    //        ret = 1;
    //    }
    //}

    ////kick for height
    if (actTrunc == ACT_DOUBLE_JUMP && *so->marioHSpd < 28.0f && *so->marioYVel < 16.0f && *so->marioYVel > -10.0f ) {
        if(xoro_r(seed) % 4 == 0) {
            in->b = 0;
            in->b |= CONT_B;
            ret = 1;
        }
    }

    ////hold a for height TODO: make only as much as necessary
    if ((actTrunc == ACT_DOUBLE_JUMP || actTrunc == ACT_JUMP) && (in->b & CONT_A)) {
        if(xoro_r(seed) % 6) {
            in->b = 0;
            in->b |= CONT_A;
            ret = 1;
        }
    }

    //??
    //if (actTrunc == ACT_FORWARD_ROLLOUT ) {
    //    in->b = 0;
    //    in->b |= CONT_A;
    //    ret = 1;
    //}
    
    //decide what to do with joystick
    int random = xoro_r(seed) % 100;
    int tarAng = (int)*so->marioYawFacing - (int)*so->camYaw;
    if (random < 92) {
        match_yaw(in, tarAng, false);
        return ret;
    }
    random -= 92;

    if (random < 4) {
        //TODO overflow???
        int16_t angle = (xoro_r(seed) % 2048) - 4096;
        match_yaw(in, tarAng + angle, false);
        return ret;
    }
    random -= 4;

    if (random < 2) {
        match_yaw(in, tarAng, true);
        return ret;
    }
    random -= 2;

    if (random < 2) {
        int16_t angle = (xoro_r(seed) % 4096) - 2048;
        match_yaw(in, tarAng+angle, true);
        return ret;
    }
    return ret;
}


//fifd: Where new inputs to try are actually produced
//I think this is a perturbation of the previous frame's input to be used for the upcoming frame
void perturbInput(Input *in, uint64_t *seed, int frame,SO* so, int megaRandom) {
    //TODO always, best practice, always for this problem, best practice for this problem, weighted random
    //TODO encode length of every state
    
    //COMMENTING THIS out can cause segfault because uninited input i think
    if (frame == 0) {
        //buggy angles???
        in->b = 0;
        int range = 2500;
        int goal_angle = 62600;
        int rand_ang = (((int)(xoro_r(seed) % 2*range) - range) + goal_angle) % (2*DEG180);
        int tarAng = rand_ang - (int)*so->camYaw;
        match_yaw(in, tarAng, false);
        return;
    }
    
    unsigned int actTrunc = *so->marioAction & 0x1FF;
    //if (must_user_def(in, seed, actTrunc, frame, so)) return;
	
    //must
	if((in->b & CONT_DDOWN) != 0){ //on first frame of pause buffer
		in->x = in->y = in->b = 0;
		in->b |= CONT_DLEFT;  //mark that we are on second frame
		return;
	}
	if((in->b & CONT_DLEFT) != 0){ //on second frame of pause buffer
		in->x = in->y = in->b = 0;
		in->b |= CONT_DUP;  //mark that we are on the third frame
		in->b |= CONT_START;  //unpause
		return;
	}
	if((in->b & CONT_DUP) != 0){  //on third frame of pause buffer
		in->x = in->y = in->b = 0; //wait for unpause to happen
		return;
	}

	//fifd: CONT_A and similar are bitmasks to identify buttons.
	//so doA is set to whether A is pressed in "in"
    //c is specifically c^
    int doA = (in->b & CONT_A) != 0;
    int doB = (in->b & CONT_B) != 0;
    int doZ = (in->b & CONT_Z) != 0;
    int doC = (in->b & CONT_CUP) != 0;
    int doS = 0;


	//fifd: Inverses of probabilities with which we toggle button statuses
	//or, in the case of jFact, joystick inputs
    int jFact = 15;
    int aFact = 4;
    int bFact = 10;
    int zFact = 20;
    if (megaRandom) { //TODO megaRandom bool
        //jFact = 2;
        jFact = 10000;
        if(best_practice(in, seed,actTrunc,frame, so)) 
            return; //if button was already decided
        //aFact = 3;
        //bFact = 3;
        //zFact = 3;
    }

    //put into best_practice
	//if(actTrunc == ACT_TURNAROUND_1 || actTrunc == ACT_TURNAROUND_2 || actTrunc == ACT_BRAKE){jFact *= 5;}
	
    if (frame == 0 || xoro_r(seed) % jFact == 0) {
        if (xoro_r(seed) % 30 == 0) {
            in->x = 0;
            in->y = 0;
        } else {
            in->x = (xoro_r(seed) % 256) - 128;
            in->y = (xoro_r(seed) % 256) - 128;

            // match yaw  idk which is better
            //int tarAng = (int)*so->marioYawFacing - (int)*so->camYaw;
            //float tarRad = tarAng * M_PI / 32768.0;
            //float tarX = 100 * sin(tarRad);
            //float tarY = -100 * cos(tarRad);
            //if (tarX > 0) {tarX += 6;} else {tarX -= 6;};
            //if (tarY > 0) {tarY += 6;} else {tarY -= 6;};
            //in->x = round(tarX);
            //in->y = round(tarY);
        }
        
    }
	
	
    if (xoro_r(seed) % aFact == 0)          doA = !doA;
    if (doB || xoro_r(seed) % bFact == 0)   doB = !doB; //always press b for 1f
    if (doZ || xoro_r(seed) % zFact == 0)   doZ = !doZ; //always press z for 1f
    if (doC) {
        doC = 0;
    } 

    if (actTrunc == ACT_PUNCHING) {
        doB = 0;
    }

    in->b = 0;
    if (doA) in->b |= CONT_A;
    if (doB) in->b |= CONT_B;
    if (doZ) in->b |= CONT_Z;
    if (doC) in->b |= CONT_CUP;
    if (doS) {
        in->b |= CONT_DDOWN;
        in->b |= CONT_START;
    }

}

//fifd: This function maps game states to a "truncated" version -
//that is, identifies the part of the state space partition this game state belongs to.
//output has 3 spatial coordinates (which cube in space Mario is in) and a variable called
//s, which contains information about the action, button presses, camera mode,
//hspd, and yaw
Vec3d truncFunc(SO* so) {
    int s = 0;
    unsigned int actTrunc = *so->marioAction & 0x1FF;
    if (actTrunc == ACT_IDLE) s = 1;
    if (actTrunc >= 0x020 && actTrunc <= 0x022) s = 2; // Crouches
    if ((actTrunc >= 0x02F && actTrunc <= 0x035) || actTrunc == 0x03A || actTrunc == 0x03B) s = 3; // Land stops;
    if (actTrunc == ACT_WALKING) s = 4; 
    if (actTrunc == ACT_TURNING_AROUND) s = 5; 
    if (actTrunc == ACT_BRAKING) s = 6;
    if (actTrunc == ACT_DECELERATING) s = 7; 
    if (actTrunc == ACT_BEGIN_SLIDING) s = 8;
    if (actTrunc == ACT_BUTT_SLIDE) s = 9; 
    if (actTrunc == ACT_STOMACH_SLIDE) s = 10; 
    if (actTrunc == ACT_DIVE_SLIDE) s = 11; 
    if (actTrunc == ACT_MOVE_PUNCHING) s = 12; 
    if (actTrunc == ACT_CROUCH_SLIDE) s = 13;
    if (actTrunc == ACT_SLIDE_KICK_SLIDE) s = 14; 
    if (actTrunc == ACT_JUMP_LAND || actTrunc == ACT_FREEFALL_LAND || actTrunc == ACT_SIDE_FLIP_LAND) s = 15; // Regular lands
    if (actTrunc == ACT_DOUBLE_JUMP_LAND) s = 16;
    if (actTrunc == ACT_QUICKSAND_JUMP_LAND) s = 17;
    if (actTrunc >= ACT_TRIPLE_JUMP_LAND) s = 18; 
    if (actTrunc >= ACT_LONG_JUMP_LAND) s = 19;
    if (actTrunc >= ACT_BACKFLIP_LAND) s = 20;
    if (actTrunc >= ACT_JUMP && actTrunc <= ACT_SPECIAL_TRIPLE_JUMP) s = 21 + (actTrunc - ACT_JUMP);

    float x = *so->marioX;
    float y = *so->marioY;
    float z = *so->marioZ;

    s *= 3;

    if (actTrunc < ACT_JUMP || actTrunc == ACT_AIR_HIT_WALL) {
        if (*so->controlButDown == 0)     s+= 1; // Pure nothing
        if (*so->controlButDown == 32768) s+= 2; // Pure A 
    }

    s *= 2;

    if (actTrunc == ACT_WALKING || actTrunc == ACT_BRAKING) {
        if (*so->camMode == 32) s+= 1; // Close cam
    }

    s *= 2;

    //air kick candidate
    if (*so->marioHSpd < 28 && (actTrunc == ACT_JUMP || actTrunc == ACT_DOUBLE_JUMP) && *so->marioYVel < 16.0f) {
        s += 1;
    }

    //s += 100000 * (int)floor((float)*so->marioYawFacing / 1024.0);

    return (Vec3d){floor(x / COARSENESS), floor(y / COARSENESS), floor(z / COARSENESS), s};
}

//fifd: Check equality of truncated states
int truncEq(Vec3d a, Vec3d b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.s == b.s);
}

//UPDATED FOR SEGMENT STRUCT
int blockLength(Block a){
	int len = 0;
	Segment *curSeg = a.tailSeg;
	if(a.tailSeg->depth == 0){printf("tailSeg depth is 0!\n");}
	while(curSeg != 0){
		if(curSeg->depth == 0){printf("curSeg depth is 0!\n");}
		len += curSeg->numFrames;
		curSeg = curSeg->parent;
	}
	return len;
}

float dist3d(float a, float b, float c, float x, float y, float z) {
    return sqrt((a - x) * (a - x) + (b - y) * (b - y) + (c - z) * (c - z));
}

int angDist(unsigned short a, unsigned short b) {
    int dist = abs((int)a - (int)b);
    if (dist > 32768) dist = 65536 - dist;
    return dist;
}

int leftLine(float a, float b, float c, float d, float e, float f) {
    return (b - d) * (e - a) + (c - a) * (f - b) > 0 ? 0 : 1;
}

// pos -> hash
uint64_t hashPos(Vec3d pos) {
    uint64_t tmpSeed = 0xCABBA6ECABBA6E;
    tmpSeed += pos.x + 0xCABBA6E;
    xoro_r(&tmpSeed);
    tmpSeed += pos.y + 0xCABBA6E;
    xoro_r(&tmpSeed);
    tmpSeed += pos.z + 0xCABBA6E;
    xoro_r(&tmpSeed);
    tmpSeed += pos.s + 0xCABBA6E;
    xoro_r(&tmpSeed);
    return tmpSeed;
}

int findNewHashInx(int *hashTab, int maxHashes, Vec3d pos) {
    uint64_t tmpSeed = hashPos(pos);
    for (int i = 0; i < 100; i++) {
        int inx = tmpSeed % maxHashes;
        if (hashTab[inx] == -1) return inx;
        xoro_r(&tmpSeed);
    }
    printf("Failed to find new hash index after 100 tries!\n");
    return -1;
}

//fifd: Given the data identifying the current block (pos), identify
//the index of that block in the hash table.
//Have not read this in detail or figured out the other arguments,
//but probably not important to understand
//me: return the index of the block with the value pos
//hashtab holds index of block
int findBlock(Block *blocks, int *hashTab, int maxHashes, Vec3d pos, int nMin, int nMax) {
    uint64_t tmpSeed = hashPos(pos);
    for (int i = 0; i < 100; i++) { //100 just like in findNewHashInx
        int inx = tmpSeed % maxHashes;
        int blockInx = hashTab[inx];
        if (blockInx == -1) return nMax;
        if (blockInx >= nMin && blockInx < nMax) { //TODO is this check necessary?
            if (truncEq(blocks[blockInx].pos, pos)) {
                return blockInx;
            }
        }
        xoro_r(&tmpSeed);
    }
    printf("Failed to find block from hash after 100 tries!\n");
    return -1; // TODO: Should be nMax?
}


//add

void merge(int* nBlocks, Block* allBlocks,Block* sharedBlocks, int* allHashTabs,int* sharedHashTab, struct Segment ** allSegs, int* numSegs ) {
    printfQ("Gonna merge\n");
    
    // Merge all blocks from all threads and redistribute info.
    // take non shared blocks from each thread and put them in the sharedblocks 
    // section if the block has the same pos but better value than the currently present block
    int otid, n, m; //m is an index of a block
    for (otid = 0; otid < totThreads; otid++) {
        for (n = 0; n < nBlocks[otid]; n++) {
            Block tmpBlock = allBlocks[otid * maxBlocks + n]; //non shared blocks
            m = findBlock(sharedBlocks, sharedHashTab, maxSharedHashes, tmpBlock.pos, 0, nBlocks[totThreads]);
            if (m < nBlocks[totThreads]) {
                if (tmpBlock.value > sharedBlocks[m].value) { // changed to >
                    sharedBlocks[m] = tmpBlock;
                }
            } else {
                sharedHashTab[findNewHashInx(sharedHashTab, maxSharedHashes, tmpBlock.pos)] = nBlocks[totThreads];
                sharedBlocks[nBlocks[totThreads]++] = tmpBlock;
            }
        }
    }
    printf("clearing local hashtables\n");
    memset(allHashTabs, 0xFF, maxHashes * totThreads * sizeof(int)); // Clear all local hash tables.
    for (otid = 0; otid < totThreads; otid++) {
        nBlocks[otid] = 0; // Clear all local blocks.
    }
    
    // Handle segments
    // Get reference counts for each segment. Tried to track this but ran into
    // multi-threading issues, so might as well recompute here.
    // copy all segs to the end
    for(int threadNum = 0; threadNum < totThreads; threadNum++){
    	for(int segInd = threadNum*maxLocalSegs; segInd < threadNum*maxLocalSegs+numSegs[threadNum]; segInd++){
    		//printf("%d %d\n", segInd, numSegs[totThreads]);
    		allSegs[totThreads*maxLocalSegs + numSegs[totThreads]] = allSegs[segInd];
    		numSegs[totThreads]++;
    		allSegs[segInd] = 0;
    	}
    	numSegs[threadNum] = 0;
    }
    
}


void segment_garbage_collection(struct Segment ** allSegs, int* numSegs, Block* sharedBlocks, int* nBlocks) {
    printf("Segment garbage collection. Start with %d segments\n", numSegs[totThreads]);
    for(int segInd = totThreads*maxLocalSegs; segInd < totThreads*maxLocalSegs+numSegs[totThreads]; segInd++){
    	allSegs[segInd]->refCount = 0;
    }
    for(int segInd = totThreads*maxLocalSegs; segInd < totThreads*maxLocalSegs+numSegs[totThreads]; segInd++){
    	if(allSegs[segInd]->parent != 0){allSegs[segInd]->parent->refCount++;}
    }
    for(int blockInd = 0; blockInd < nBlocks[totThreads]; blockInd++){
    	sharedBlocks[blockInd].tailSeg->refCount++;
    }
    //delete all last segements?? (all unreferenced segs)
    //TODO can segemnts just become unreferenced? tails are always unreferenced
    for(int segInd = totThreads*maxLocalSegs; segInd < totThreads*maxLocalSegs+numSegs[totThreads]; segInd++){
    	Segment* curSeg = allSegs[segInd];//shared segs
    	if(curSeg->refCount == 0){
    		//printf("removing a seg\n");
    		if(curSeg->parent != 0){curSeg->parent->refCount -= 1;}
    		//printf("moving %d %d\n", segInd, totThreads*maxLocalSegs+numSegs[totThreads]);
    		allSegs[segInd] = allSegs[totThreads*maxLocalSegs+numSegs[totThreads] - 1];
    		numSegs[totThreads]--;
    		segInd--;
    		free(curSeg);
    	}
    }
}

int pick_block_index(int* nBlocks, int mainLoop,Block* sharedBlocks, int* sharedHashTab, Vec3d *lightningList, uint64_t* seed,int lightLen) { 
    int origInx = nBlocks[totThreads] ;
    if (mainLoop % 15 == 0) {
        origInx = 0; //first block ever
    } 
    else if (mainLoop % 7 == 1 && lightLen > 0) {//pick a lightning block
        for (int try_ = 0; try_ < 1000; try_++) {
            int randomLightInx = xoro_r(seed) % lightLen;
            origInx = findBlock(sharedBlocks, sharedHashTab, maxSharedHashes, lightningList[randomLightInx], 0, nBlocks[totThreads]);
            if (origInx != nBlocks[totThreads]) break;
        }
        if (origInx == nBlocks[totThreads]) {
            printf("Could not find lightning block, using root!\n");
            origInx = 0;
        }
    } 
    else {
        bool weighted = (xoro_r(seed) % 2) && (lightLen > 0);
        //xoro_r(seed); // this updates rng
        for (int try_ = 0; try_ < 100000; try_++) {
            origInx = xoro_r(seed) % nBlocks[totThreads];
    		if(sharedBlocks[origInx].tailSeg == 0){printf("Chosen block tailseg null!\n"); continue;}
    		if(sharedBlocks[origInx].tailSeg->depth == 0){printf("Chosen block tailseg depth 0!\n"); continue;}
            //UNUSED
            //uint64_t s = sharedBlocks[origInx].pos.s;
            //test feature
            if (weighted) {
                if (sharedBlocks[origInx].tailSeg->depth < maxSegs && xoro_r(seed) % (blockLength(sharedBlocks[origInx]) / 1 + 1) == 0 
                        && (xoro_r(seed) % 100 <= sharedBlocks[origInx].prob)) break;
            } else {
                if (sharedBlocks[origInx].tailSeg->depth < maxSegs && (xoro_r(seed) % 100 <= sharedBlocks[origInx].prob)) break;
            }
    		//if(((float)(xoro_r(seed) % 50)/100 < approxXZSum*approxXZSum) & (sharedBlocks[origInx].tailSeg->depth < maxSegs))break;
        }
        if (origInx == nBlocks[totThreads]) {
            printf("Could not find block!\n");
            return -1;
        }
    }
    return origInx;
}

void set_new_seg(Segment* newSeg, Block* origBlock,Block* newBlock,Block* blocks,int* nBlocks, Segment** allSegs, int* numSegs ,uint64_t origSeed, int tid, int f, int refCount) {
    newSeg->parent = origBlock->tailSeg;
    newSeg->refCount = refCount;
    newSeg->numFrames = f + 1;
    newSeg->seed = origSeed;
    newSeg->depth = origBlock->tailSeg->depth + 1;
    if(newSeg->depth == 0){printf("newSeg depth is 0!\n");}
    if(origBlock->tailSeg->depth == 0){printf("origBlock tailSeg depth is 0!\n");} //impossible, right?
    newBlock->tailSeg = newSeg;
    allSegs[tid*maxLocalSegs + numSegs[tid]] = newSeg;
    numSegs[tid] += 1;
    blocks[nBlocks[tid]++] = *newBlock;
    //update load array stuff
    //find differences
}

