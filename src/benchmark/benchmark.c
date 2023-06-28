#include <stdbool.h>
#include "scattershot/scattershot.h"
#include <omp.h>

char* input_m64 = "deepfreeze.m64";
int startFrame = 1646; //ingame frame -1??
int readLength = 1651; //+5

void read_input(Input* fileInputs) {
        Input in;
		FILE *fp = fopen(input_m64, "r");
        fseek(fp, 0x400, SEEK_SET);
        for (int i = 0; i < readLength; i++) {
            fread(&in, sizeof(Input), 1, fp);
            in.b = (in.b >> 8) | (in.b << 8); // Fuck me endianness
            fileInputs[i] = in;
        }
        fclose(fp);
}

void run_inputs(SO* so, Input* fileInputs, SaveState* state,int* startCourse,int* startArea) {
    for (int f = 0; f < readLength; f++) {
        *so->gControllerPads = fileInputs[f];
        so->sm64_update();
        if (f == startFrame - 1) {
            save(so->sm64_base, state);
            *startCourse = *so->gCurrCourseNum;
            *startArea = *so->gCurrAreaIndex;
            print_act(so);
        }
    }
}

int main(int argc, char** argv)
{
    SO so;
    init_so(&so ,argv[1]);
    so.sm64_init();

    //load m64, play the inputs til start of game
    Input *fileInputs = (Input *)malloc(sizeof(Input) * readLength);
    read_input(fileInputs);

    int startCourse = 0, startArea = 0;
    SaveState state;
    allocState(&state);
    run_inputs(&so, fileInputs, &state, &startCourse, &startArea);
    
    int max = 10000000;
    //take time
    double start = omp_get_wtime();
    Input in;
    in.b = 0;
    in.x = 0;
    in.y = 0;
    for (int i = 0; i < max; i++) {
        //printf("before i %i\n",i);
        reduced_update(&so, &in);
        //printf("after i %i\n",i);
        //so.sm64_update();
    }
    double end = omp_get_wtime();
    printf("fps: %lf\n", (double) max/ (end-start));
    return 0;
}
