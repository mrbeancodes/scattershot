#include <stdbool.h>
#include <time.h>
#include <omp.h>

#ifdef __linux__
#include <dlfcn.h>
#endif

#include "scattershot/scattershot.h"
#include "scattershot/settings.h"
#include "scattershot/json.h"

//checklist of things adjust to the problem
//everything until end settings
////you have to first find a bss and data map that works for your problem by uncommenting #define FINDMAP
////and then paste it in util.c (this can often result in segfaults if not done right)
//settings.c 
//break_condition
//write_condition
//truncFunc sometimes
//perturbInput sometimes
//end checklist
//

//settings
//also see settings.c
//#define omp_get_wtime() 0 //dont time
                    
#define UPDATE(so,in) reduced_update(so, in) 
//#define UPDATE(so,in) full_update(so, in) 

//#define FINDMAP //finding out what parts of the so change and which dont
#ifdef FINDMAP
#define prstate 1
#define LOAD(sobase, state) riskyLoad(sobase, state)
#define SAVE(sobase, state) save(sobase, state)
#else
#define prstate 0
#define LOAD(sobase, state) reducedLoad(sobase, state)
#define SAVE(sobase, state) reducedSave(sobase, state)
#endif

inline float fitness(SO* so,int frame) { //block fitness
    //return -fabs(*so->pyraYNorm);
    //return -1000.0 * (frame + 1.0) - 10* *so->marioHSpd - *so->marioZ; //abs
    //return -1000.0*(frame + 1.0) + *so->marioZ + 5**so->marioHSpd;
    //return (-1000* (float) frame + (*so->marioYawFacing));
    return (-1000* (float) frame + (*so->marioHSpd));
}

inline float goal_fitness(SO* so, int frame) {
    return -1000.0 * (frame + 1.0) + *so->marioHSpd; //abs
    //return -1000.0 * (frame + 1.0) - *so->marioHSpd; //abs
    //return -1000.0*(frame + 1.0) - *so->marioZ + 5* *so->marioHSpd;
    //return -1000.0 * (frame + 1.0) - 10* *so->marioHSpd - *so->marioZ - 50* *so->marioX; //abs
    //return (-(float)frame - *so->marioY);
    //return (-(float)frame + *so->marioX);
    //return (-1000* (float) frame + (*so->marioYawFacing));
}

inline float test_fitness(SO* so, int frame) {
    return (-1000.0*(float)frame + *so->marioZ + *so->marioY);
}
//end settings

//temporary
int found_best = 0;
float found_test[40][10];
int found_test_count = 0;


bool break_condition(SO* so, unsigned int actionTrunc, int frame) {
    if (frame > max_frames) return true;

    for(int i = 0;i < gBlackListCount; i++){
        int state = gBlackList[i].state;
        if (state == actionTrunc &&
                (gBlackList[i].min_h_spd <= *so->marioHSpd &&
                gBlackList[i].max_h_spd >= *so->marioHSpd) &&
                (gBlackList[i].min_v_spd <= *so->marioYVel &&
                gBlackList[i].max_v_spd >= *so->marioYVel)
                ) {
            return true;
        }
    }

    for(int i = 0;i < gWhiteListCount;i++){
        if (!(gWhiteList[i].state == actionTrunc &&
                (gWhiteList[i].min_h_spd <= *so->marioHSpd &&
                gWhiteList[i].max_h_spd >= *so->marioHSpd) &&
                (gWhiteList[i].min_v_spd <= *so->marioYVel &&
                gWhiteList[i].max_v_spd >= *so->marioYVel)
             )) {
            return true;
        }
    }

    //deepfreeze
    //TODO
    //(actionTrunc == ACT_BACKWARD_AIR_KB && *so->marioZ < 4100.0f)

    if(usereftas) {
        //for tunnel mode
        int i = 0; //max_(frame-1,0);
        while(cubePos[i].min_x != 0.0f && cubePos[i].min_y != 0.0f && cubePos[i].min_z != 0.0f
            &&cubePos[i].max_x != 0.0f && cubePos[i].max_y != 0.0f && cubePos[i].max_z != 0.0f
                ) {
            if(in_quboid(*so->marioX, *so->marioY, *so->marioZ, &cubePos[i++])){
                return false;
            }
        }
        return true;
    } 

    bool inArea = false;
    for(int i = 0;i < gAreaCount;i++){
        if(in_quboid(*so->marioX, *so->marioY, *so->marioZ, &areas[i].qu)) {
            for(int j = 0;j < areas[i].lcount;j++){
                if(!leftLine(areas[i].lines[j].x1,areas[i].lines[j].y1,areas[i].lines[j].x2,areas[i].lines[j].y2, *so->marioX, *so->marioZ)){
                    return true;
                }
            }
            inArea = true;
            //check other constraints
        }
    }
    if(inArea) return false;
    return true;

    
}

bool write_condition(SO* so, unsigned int actionTrunc,Input* m64short ,int f_offset, int tid, int* lightLen, int lightLenLocal, Vec3d* lightningList, Vec3d* lightningLocal, float* bestTimes) {
    if(*so->marioX >= writeCond[0].min_x && *so->marioX <= writeCond[0].max_x && *so->marioY >= writeCond[0].min_y
            && *so->marioY <= writeCond[0].max_y && *so->marioZ >= writeCond[0].min_z && *so->marioZ <= writeCond[0].max_z 
            && *so->marioHSpd >= writeCond[0].min_h_spd && *so->marioHSpd <= writeCond[0].max_h_spd && *so->marioYVel >= writeCond[0].min_v_spd && 
            *so->marioYVel <= writeCond[0].max_v_spd && (actionTrunc == writeCond[0].state || writeCond[0].state == ACT_ANY)){

        if (tid >= 0) { //TODO hacky
            float cur_fitness = goal_fitness(so, f_offset);
            if (cur_fitness > bestTimes[tid] || bestTimes[tid] == 0) {
                bestTimes[tid] = cur_fitness;
                *lightLen = lightLenLocal;
                max_frames = f_offset; //dont bruteforcer longer than necessary
                for (int lightInx = 0; lightInx < *lightLen; lightInx++) {
                    lightningList[lightInx] = lightningLocal[lightInx];
                }
                found_best++;
    	        char fileName[128];
                //sprintf(fileName, "m64/carpet/carp_blj_newland_%d_x_%f_z_%f_spd_%f_%d.m64",f_offset,*so->marioX, *so->marioZ, *so->marioHSpd, tid);
                sprintf(fileName, "m64/deepfreeze/deep_f_%d_x_%f_z_%f_spd_%f_%d.m64",f_offset,*so->marioX, *so->marioZ, *so->marioHSpd, tid);
                writeFile(fileName, input_m64, m64short, startFrame, f_offset + 1);
                printf("found tas\n");
                return true;
            }
        }
        return true;
    }
    if (tid >= 0) {
        for (int i = 0; i < 40; i++) {
            for (int j = 0; j < 10; j++) {
                if ( *so->marioZ > (1200.0f + i*100.0f) && *so->marioZ <= (1200.0f + (i+1)*100.0f) 
                        && test_fitness(so, f_offset) > found_test[i][j] && 
                        *so->marioY > (1400.0f + j*50.0f) && *so->marioY <= (1400.0f + (j+1)*50.0f)) {
                    found_test_count++;
                    found_test[i][j] = test_fitness(so, f_offset);
                    if (found_test_count % 100 == 0) {
                        char fileName[128];
                        sprintf(fileName, "m64/deepfreeze/deep_test_%d_%d_%d_%f_%d.m64",i,j,f_offset,*so->marioZ, tid);
                        writeFile(fileName, input_m64, m64short, startFrame, f_offset + 1);
                        printf("found test\n");
                    }
                }
            }
        }
    }

    return false;
}

void full_update(SO* so, Input* in) {
    *so->gControllerPads = *in;
    so->sm64_update();
}

void read_input(Input* fileInputs) {
        Input in;
		FILE *fp = fopen(input_m64, "r");
        if (!fp) {
            printf("failed to open m64!\n");
            exit(1);
        }
        fseek(fp, 0x400, SEEK_SET);
        for (int i = 0; i < readLength; i++) {
            fread(&in, sizeof(Input), 1, fp);
            in.b = (in.b >> 8) | (in.b << 8); // Fuck me endianness
            fileInputs[i] = in;
        }
        fclose(fp);
}

void writeInputState(Input* in,SO* so, FILE* fp) {
    char a =    (in->b & CONT_A)        != 0 ? 'a' : '.';
    char b =    (in->b & CONT_B)        != 0 ? 'b' : '.';
    char z =    (in->b & CONT_Z)        != 0 ? 'z' : '.';
    //C
    char cr =   (in->b & CONT_CRIGHT)   != 0 ? 'r' : '.';
    char cl =   (in->b & CONT_CLEFT)    != 0 ? 'l' : '.';
    char cd =   (in->b & CONT_CDOWN)    != 0 ? 'd' : '.';
    char cu =   (in->b & CONT_CUP)      != 0 ? 'u' : '.';

    char s =    (in->b & CONT_START)    != 0 ? 's' : '.';
    char l =    (in->b & CONT_L)        != 0 ? 'L' : '.';
    char r =    (in->b & CONT_R)        != 0 ? 'R' : '.';

    //D
    char dr =   (in->b & CONT_DRIGHT)   != 0 ? 'r' : '.';
    char dl =   (in->b & CONT_DLEFT)    != 0 ? 'l' : '.';
    char dd =   (in->b & CONT_DDOWN)    != 0 ? 'd' : '.';
    char du =   (in->b & CONT_DUP)      != 0 ? 'u' : '.';
    char actionName[26];
    get_act_str(*so->marioAction & 0x1FF, actionName);

    fprintf(fp, "x: %d\ty: %d\t%c%c%cC(%c%c%c%c)%c%c%cD(%c%c%c%c) mariox: %f marioy: %f marioz: %f hspd: %f vspd: %f yawfacing: %d yawintended: %d action: %s\n" 
            , in->x, in->y, a, b, z ,cr, cl, cd, cu, s, l ,r, dr, dl, dd, du,
            *so->marioX, *so->marioY, *so->marioZ, *so->marioHSpd, *so->marioYVel, *so->marioYawFacing,*so->marioYawIntended, actionName
            );
}

void dumpTasInfo(SO* so, Input* fileInputs, char* fileName) {
    FILE* fp = fopen(fileName, "w");
    if(!fp){
        printf("couldnt open %s\n", fileName);
        exit(0);
    }
    for(int i = 0; i<readLength; i++) {
        Input in = fileInputs[i];
        full_update(so, &in);
        if(i >= startFrame) {
            writeInputState(&in,so,fp);
        }
    }

    fclose(fp); 
}

void run_inputs(SO* so, Input* fileInputs, SaveState* state,int* startCourse,int* startArea) {
    bool foundEnd = false;
    printf("run inputs\n");
    for (int f = 0; f < readLength; f++) {
        int off = f-startFrame;
        //printf("f %d\n",f);
        //print_act(so);
        if (f == startFrameCourse) {
            findMarioIndex(so);
        }
        if (f > startFrameCourse) {
            printf("frame %d\n",f);
            if( f > switchtoUPDATE) {
                UPDATE(so, &fileInputs[f]); 
            }
            else {
                full_update(so, &fileInputs[f]);
            }
            print_act(*so->marioAction & 0x1FF);
            printf("x %f y %f z %f facing %d cam %u\n",*so->marioX,*so->marioY,*so->marioZ, *so->marioYawFacing, (*so->gCamera)->mode);
            
        }
        else {
            full_update(so, &fileInputs[f]);
        }

    	//if(f > startFrame){
        //    printf("at frame %d ",f);
        //    print_act(so);
    	//}
        if (f == startFrame - 1) {
            save(so->sm64_base, state);
            *startCourse = *so->gCurrCourseNum;
            *startArea = *so->gCurrAreaIndex;
            print_act(*so->marioAction & 0x1FF);
            printf("initial x %f y %f z  %f\n",*so->marioX,*so->marioY,*so->marioZ);
            //exit(0);
        }
        unsigned int actionTrunc = *so->marioAction & 0x1FF;
        if (f >= startFrame) {
            reachedPos[off].x = *so->marioX;
            reachedPos[off].y = *so->marioY; 
            reachedPos[off].z = *so->marioZ;
        }
        if (usereftas) {
            if(write_condition(so, actionTrunc, NULL , f,/*tid -1 dont write the tas*/ -1, NULL, 0, NULL, NULL, NULL)) {
                //max_frames = off;
                printf("using reference tas with max_frames %d\n",max_frames);
                foundEnd = true;
                break;
            }
        }
    }
    if (usereftas && !foundEnd) {
        printf("coundnt find end of tas\n");
        exit(1);
    }
    printf("init cubes\n");
    for(int f = 0;f < max_frames-1; f++){
        ///printf("index %d\n", f);
        Vec3df pos1 = reachedPos[f];
        Vec3df pos2 = reachedPos[f+1];
        cubePos[f].min_x = min_(pos1.x, pos2.x)-pad; 
        cubePos[f].max_x = max_(pos1.x, pos2.x)+pad;
        cubePos[f].min_y = min_(pos1.y, pos2.y)-pad; 
        cubePos[f].max_y = max_(pos1.y, pos2.y)+pad;
        cubePos[f].min_z = min_(pos1.z, pos2.z)-pad; 
        cubePos[f].max_z = max_(pos1.z, pos2.z)+pad;
    }
    printf("init cubes done\n");
    //exit(0);
}
    

//TODO parse args
int main(int argc, char *argv[]) {
    bool stop = false;
    char dumpFileName[128] = "";
    for( int i = 1; i < argc; i++) {
        if (argv[i][0] == '-')  {
            if (argv[i][1] == 'i') {
                if (i+1 < argc)  {
                    init_settings(argv[i+1]);
                }
            }
            if (argv[i][1] == 'd') {
                //dump facing angles, intended angles, position, buttonpresses
                if (i+1 < argc)  {
                    strcpy(dumpFileName, argv[i+1]);
                }
            }
            if (argv[i][1] == 's') {
                stop = true;
            }
        }
    }


    initFields();
    printFields();

    strncpy(gProgName, argv[0], 128);
    char dllFullName[128+3] = "";
    printf("basename %s\n", dllBaseName);
    sprintf(dllFullName, "%s.so", dllBaseName);
    printf("m64: %s sll/so (%s)\n", input_m64, dllFullName);

    void *testDLL = dlopen(dllFullName, RTLD_LAZY); //???
    if (!testDLL)
    {
        printf("load fail, try supplying the full path\n");
        return 1;
    }
    // Find the stupid DLL segments for savestates.
    getDllInfo(dllFullName);
    printf("Got DLL segments data %d %d bss %d %d\n", dataStart, dataLength, bssStart, bssLength);
    if(strlen(dumpFileName) > 0) {
        SO so1;
        init_so(&so1 ,dllFullName);
        so1.sm64_init();
        Input *fileInputs1 = (Input *)malloc(sizeof(Input) * readLength);
        read_input(fileInputs1);
        dumpTasInfo(&so1, fileInputs1, dumpFileName);
    }
    if(stop) return 0;


    // Per thread info.
    float    bestTimes[16] = {0};

    int    nBlocks[17] = {0};
    SaveState xorSlaves[16];
    Block *allBlocks = (Block *)calloc(totThreads * maxBlocks + maxSharedBlocks, sizeof(Block));
    int   *allHashTabs = (int *)calloc(totThreads * maxHashes + maxSharedHashes, sizeof(int));
	
	int numSegs[17] = {0};
	struct Segment **allSegs = (struct Segment **) malloc((maxSharedSegs + totThreads*maxLocalSegs)*sizeof(struct Segment*));

    omp_set_num_threads(totThreads);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("hello thread %d\n", tid);

        char dllName[128] = "";
        sprintf(dllName, "%s_%d.so", dllBaseName, tid);
        cp(dllFullName, dllName);

        SO so;
		
        init_so(&so ,dllName);
        so.sm64_init();
		
        int f;
        Input in;
        uint64_t seed = (uint64_t)(tid + 173) * 5786766484692217813;
        //uint64_t seed = (uint64_t)(tid + 173) * time(NULL);

        double timerStart, loadTime = 0, runTime = 0, blockTime = 0, pertubTime = 0, saveTime = 0;

        int startCourse = 0, startArea = 0;

        SaveState state, state2;
        allocState(&state);
        allocState(&state2);
        if(prstate) allocState(xorSlaves + tid);
        
        // Read inputs from file
        Input *fileInputs = (Input *)malloc(sizeof(Input) * readLength);
        read_input(fileInputs);

        // Run the inputs.
        run_inputs(&so, fileInputs, &state, &startCourse, &startArea);

        //declare stuff
        Input *m64short = (Input *)malloc(sizeof(Input) * (segLength * maxSegs + 256)); // Todo: Nasty
        Block *blocks = allBlocks + tid * maxBlocks;
        int   *hashTab = allHashTabs + tid * maxHashes;
        Block *sharedBlocks = allBlocks + totThreads * maxBlocks;
        int   *sharedHashTab = allHashTabs + totThreads * maxHashes;
        //init the shit
        for(int i = 0; i < 40; i++) {
            for(int j = 0; j < 10; j++) {
                found_test[i][j] = -INFINITY;
            }
        }
        for (int hashInx = 0; hashInx < maxHashes; hashInx++) hashTab[hashInx] = -1; // Init local hash table.
        if (tid == 0) {
            for (int hashInx = 0; hashInx < maxSharedHashes; hashInx++) sharedHashTab[hashInx] = -1; // Init shared hash table.
        }

        //fifd in science: not sure but if TAS sets new record, blocks along the way have a 
        //higher chance of being chosen
        int maxLightning = 10000; //TODO what is this
        int lightLen = 0;
        Vec3d *lightningList = (Vec3d *)malloc(sizeof(Vec3d) * maxLightning);
        Vec3d *lightningLocal = (Vec3d *)malloc(sizeof(Vec3d) * maxLightning);

        // Give info to the root block.
        riskyLoad(so.sm64_base, &state);

        blocks[0].pos = truncFunc(&so);
		blocks[0].tailSeg = (Segment *) malloc(sizeof(Segment)); //Instantiate root segment
		blocks[0].tailSeg->numFrames = 0;
		blocks[0].tailSeg->parent = NULL;
		blocks[0].tailSeg->refCount = 0;
		blocks[0].tailSeg->depth = 1;
		allSegs[numSegs[tid] + tid*maxLocalSegs] = blocks[0].tailSeg;
		//printf("%d\n", numSegs[tid] + tid*maxLocalSegs);
		numSegs[tid]++;
        nBlocks[tid]++;
        hashTab[findNewHashInx(hashTab, maxHashes, blocks[0].pos)] = 0;

        double pureStart = omp_get_wtime();

        for (int mainLoop = 0; mainLoop <= 1000000000; mainLoop++) {
            int blInx;
            Block origBlock, newBlock;
            uint64_t tmpSeed;

            // ALWAYS START WITH A MERGE SO THE SHARED BLOCKS ARE OK.
            if (mainLoop % merge_mod == 0) {
                #pragma omp barrier
                printf("tid %d mainLoop %d\n",tid, mainLoop);
                if (tid == 0) {
                    if(prstate) print_xored_states(xorSlaves, tid);
                    merge(nBlocks, allBlocks, sharedBlocks, allHashTabs, sharedHashTab, allSegs, numSegs);
                    //there used to be a bestY loop but it was unused orig comment-> //Synchronize records across threads
                    if (mainLoop % garbage_mod == 0) {
                        segment_garbage_collection(allSegs, numSegs, sharedBlocks, nBlocks);
                    }
                    printf("Segment garbage collection finished. Ended with %d segments\n", numSegs[totThreads]);
                    
                    double totalTime = omp_get_wtime() - pureStart;
                    double otherTime = totalTime - loadTime - runTime - blockTime - pertubTime - saveTime;
                    printfQ("\nThread ALL Loop %d blocks %d\n", mainLoop, nBlocks[totThreads]);
                    printfQ("LOAD %.3f RUN %.3f BLOCK %.3f PERT %.3f SAVE %.3f OTHER %.3f TOTAL %.3f\n", loadTime, runTime, blockTime, pertubTime,saveTime, otherTime, totalTime);
                    printfQ("found best %d\n", found_best);
                    pureStart = omp_get_wtime();
                    loadTime = runTime = blockTime = pertubTime = saveTime = 0;
                    printfQ("\n\n");
                }
                flushLog();
                printf("merge done\npick a block\n");
                #pragma omp barrier
            }
            
            // Pick a block.
            
            int origInx = pick_block_index(nBlocks, mainLoop, sharedBlocks, sharedHashTab, lightningList, &seed, lightLen);
            if (origInx == -1) break; //couldnt find index

            origBlock = sharedBlocks[origInx];
			if(origBlock.tailSeg->depth > maxSegs + 2){printf("origBlock depth above max!\n");}
			if(origBlock.tailSeg->depth == 0){printf("origBlock depth is zero!\n");}

            // Create a state for this block.
			LOAD(so.sm64_base, &state);

            int lightLenLocal = 0;
            Vec3d newPos = truncFunc(&so);
            lightningLocal[lightLenLocal++] = newPos;

			if(origBlock.tailSeg == 0) printf("origBlock has null tailSeg");
			Segment *thisTailSeg = origBlock.tailSeg;
			Segment *curSeg;
			int thisSegDepth = thisTailSeg->depth;

            //run all segments for this block to get the the state of the block
            int trueF = 0;
			for(int i = 1; i <= thisSegDepth; i++){
				curSeg = thisTailSeg;
				while(curSeg->depth != i){  //inefficient but probably doesn't matter
					if(curSeg->parent == 0) printf("Parent is null!");
					if(curSeg->parent->depth + 1 != curSeg->depth){printf("Depths wrong");}
					curSeg = curSeg->parent;
				}
				//Run the inputs
				tmpSeed = curSeg->seed;
				int megaRandom = xoro_r(&tmpSeed) % megarand_mod;
				for (f = 0; f < curSeg->numFrames; f++){
					perturbInput(&in, &tmpSeed, trueF, &so, megaRandom);
                    m64short[trueF++] = in;

                    //full_update(&so, &fileInputs[f]);
                    UPDATE(&so, &in);

                    newPos = truncFunc(&so);
                    if (!truncEq(newPos, lightningLocal[lightLenLocal - 1])) {
                        if (lightLenLocal < maxLightning) {
                            lightningLocal[lightLenLocal++] = newPos;
                        } else {
                            printf("Reached max lightning!\n");
                        }
                    }
				}
			}

            //printf("saving state2\n");
            timerStart = omp_get_wtime();
			SAVE(so.sm64_base, &state2);
            saveTime += omp_get_wtime() - timerStart;
            if(prstate) xorStates(&state, &state2, xorSlaves + tid, tid);

            Input origLastIn = in;
            Vec3d origPos = truncFunc(&so);
            //int   origValue = value;
            int   origLightLenLocal = lightLenLocal;
            if (!truncEq(origBlock.pos, origPos)){
				printf("ORIG %d %d %d %ld AND BLOCK %d %d %d %ld NOT EQUAL\n", 
                origPos.x, origPos.y, origPos.z, origPos.s, origBlock.pos.x, origBlock.pos.y, origBlock.pos.z, origBlock.pos.s);
				Segment *curSegDebug = origBlock.tailSeg;
				while(curSegDebug != 0){  //inefficient but probably doesn't matter
					if(curSegDebug->parent == 0)printf("Parent is null!");
					if(curSegDebug->parent->depth + 1 != curSegDebug->depth){printf("Depths wrong");}
					curSegDebug = curSegDebug->parent;
				}
			}

            // From state run a bunch of times.
            int breaksCount = 0;
            for (int subLoop = 0; subLoop < subLoopMax; subLoop++) {
                Vec3d oldPos;
                uint64_t origSeed = seed;
                timerStart = omp_get_wtime();
                LOAD(so.sm64_base, &state2);
                loadTime += omp_get_wtime() - timerStart;
                oldPos = origPos;
                in     = origLastIn;
                lightLenLocal = origLightLenLocal;

                int megaRandom = xoro_r(&seed) % megarand_mod;

                int maxRun = segLength;
                bool found = false;
                int extension = 0;
                for (f = 0; f < maxRun + extension; f++) {
                    if (found) break; //break if already found something
                                      //
                    timerStart = omp_get_wtime();
                    perturbInput(&in, &seed, trueF + f, &so, megaRandom);
                    pertubTime += omp_get_wtime() - timerStart;

                    m64short[trueF + f] = in;
                    timerStart = omp_get_wtime();

                    UPDATE(&so, &in);

                    runTime += omp_get_wtime() - timerStart;

                    unsigned int actionTrunc = *so.marioAction & 0x1FF;
                    newPos = truncFunc(&so);

                    if (*so.gCurrCourseNum != startCourse || *so.gCurrAreaIndex != startArea) break;

                    //break condition
                    if (break_condition(&so, actionTrunc, trueF+f)) {
                        if(f <= breakCountFrame) breaksCount++;
                        break;
                    }

                    //write condition
                    found = write_condition(&so, actionTrunc, m64short , f+trueF, tid, &lightLen,lightLenLocal, lightningList,  lightningLocal, bestTimes);

                    if (!found) {
                        if(*so.marioX <= 4429.0f && *so.marioX >= 4324.0f && *so.marioZ >= 4309.0f && *so.marioZ <= 4413.0f && *so.marioY <= 1750.0f){
                            if(extension < 15) extension++;
                        }
                    }

                    //light idk
                    if (!truncEq(newPos, lightningLocal[lightLenLocal - 1])) {
                        if (lightLenLocal < maxLightning) {
                            lightningLocal[lightLenLocal++] = newPos;
                        } else {
                            printf("Reached max lightning!\n");
                        }
                    }

					//fifd: Checks to see if we're in a new Block. If so, save off the segment so far.
                    timerStart = omp_get_wtime();
                    if (actionTrunc < 0xC0 &&  //?? what action exactly
                        !truncEq(newPos, oldPos) && !truncEq(newPos, origPos)) {

                        // Create and add block to list.
                        if (nBlocks[tid] != maxBlocks) {
							//UPDATED FOR SEGMENTS STRUCT
                            newBlock = origBlock;
                            newBlock.pos = newPos;
                            newBlock.value = fitness(&so,trueF+f); 
                            newBlock.prob = 100;
                            int blInxLocal = findBlock(blocks, hashTab, maxHashes, newPos, 0, nBlocks[tid]);
                            blInx = findBlock(sharedBlocks, sharedHashTab, maxSharedHashes, newPos, 0, nBlocks[totThreads]);

                            if (blInxLocal < nBlocks[tid]) { // Existing local block.
                                if (newBlock.value >= blocks[blInxLocal].value) { //better val
									Segment* newSeg = (Segment *) malloc(sizeof(Segment));
                                    set_new_seg(newSeg, &origBlock,&newBlock,blocks,nBlocks, allSegs,numSegs , origSeed, tid, f, 0);
                                }
                            } 
                            else if (blInx >= nBlocks[totThreads] || newBlock.value >= sharedBlocks[blInx].value) {// completely new block OR Existing shared block and better
                                hashTab[findNewHashInx(hashTab, maxHashes, newPos)] = nBlocks[tid];
								Segment* newSeg = (Segment *) malloc(sizeof(Segment));
                                set_new_seg(newSeg, &origBlock,&newBlock,blocks,nBlocks, allSegs,numSegs , origSeed, tid, f, 1);
                            }
                        } 
                        else {
                            printf("Max local blocks reached!\n");
                        }
                        oldPos = newPos; // TODO: Why this here?
                    }
                    blockTime += omp_get_wtime() - timerStart;
                }
            }
            //Experimental: if only leads to breaks penalize block
            if (subLoopMax == breaksCount) {
                if (origBlock.prob > minProb+penalProb) {
                    origBlock.prob -= penalProb;
                }
                else {
                    origBlock.prob = minProb;
                }
            }
            else {
                origBlock.prob = 100;
            }
        }
    }
    return 0;
}



//TODO broken and ununsed
int run_all_segs(Segment* curSeg, Segment* thisTailSeg,Input* m64short,SO* so,Vec3d* lightningLocal,int lightLenLocal,int maxLightning, int thisSegDepth) {
    int trueF = 0;
    Input in;
    for(int i = 1; i <= thisSegDepth; i++){
    	curSeg = thisTailSeg;
    	while(curSeg->depth != i){  //inefficient but probably doesn't matter
    		if(curSeg->parent == 0) printf("Parent is null!");
    		if(curSeg->parent->depth + 1 != curSeg->depth){printf("Depths wrong");}
    		curSeg = curSeg->parent;
    	}
    	//Run the inputs
    	uint64_t tmpSeed = curSeg->seed;
    	int megaRandom = xoro_r(&tmpSeed) % megarand_mod;
    	for (int f = 0; f < curSeg->numFrames; f++){
    		perturbInput(&in, &tmpSeed, trueF, so, megaRandom);
            m64short[trueF++] = in;
            *so->gControllerPads = in;
            so->sm64_update();
            //deleted prevXZSum segment
            Vec3d newPos = truncFunc(so);
            if (!truncEq(newPos, lightningLocal[lightLenLocal - 1])) {
                if (lightLenLocal < maxLightning) {
                    lightningLocal[lightLenLocal++] = newPos;
                } else {
                    printf("Reached max lightning!\n");
                }
            }
    	}
    }
    return trueF;
}
