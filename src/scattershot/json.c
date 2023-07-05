#include "scattershot/json.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>

int init_settings(char* json_name) {
    JSON_ENTRY json_entries[] = {
        {&penalProb,            NUMBER,     "penalProb"},
        {&minProb,              NUMBER,     "minProb"},
        {&breakCountFrame,      NUMBER,     "breakCountFrame"},
        {&input_m64,            STRING,     "input_m64"},
        {&dllBaseName,          STRING,     "dllBaseName"},
        {&startFrame,           NUMBER,     "startFrame"},
        {&startFrameCourse,     NUMBER,     "startFrameCourse"},
        {&switchtoUPDATE,       NUMBER,     "switchtoUPDATE"},
        {&readLength,           NUMBER,     "readLength"},
        {&max_frames,           NUMBER,     "max_frames"},    
        {&update_max_frames,    NUMBER,     "update_max_frames"},
        {&merge_mod,            NUMBER,     "merge_mod"},
        {&garbage_mod,          NUMBER,     "garbage_mod"},
        {&subLoopMax,           NUMBER,     "subLoopMax"},
        {&megarand_mod,         NUMBER,     "megarand_mod"},
        {&totThreads,           NUMBER,     "totThreads"},
        {&segLength,            NUMBER,     "segLength"},    
        {&maxSegs,              NUMBER,     "maxSegs"},
        {&maxBlocks,            NUMBER,     "maxBlocks"},
        {&maxHashes,            NUMBER,     "maxHashes"},
        {&maxSharedBlocks,      NUMBER,     "maxSharedBlocks"},
        {&maxSharedHashes,      NUMBER,     "maxSharedHashes"},
        {&maxSharedSegs,        NUMBER,     "maxSharedSegs"},
        {&maxLocalSegs,         NUMBER,     "maxLocalSegs"},
        {&COARSENESS,           FLOAT,      "COARSENESS"},
        {&usereftas,            NUMBER,     "usereftas"},
        {&areas,                AREALIST,   "areas"},
        {&gWhiteList,           CONSTRLIST, "gWhiteList"},
        {&gBlackList,           CONSTRLIST, "gBlackList"},
        {&pad,                  FLOAT,      "pad"},
        {&writeCond,            WRITECOND,  "write_cond"},
    };


    // Read the JSON file
    FILE *file = fopen(json_name, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *jsonData = (char *)malloc(fileSize + 1);
    fread(jsonData, 1, fileSize, file);
    fclose(file);

    // Parse the JSON data
    cJSON *json = cJSON_Parse(jsonData);
    if (json == NULL) {
        printf("Failed to parse JSON data.\n");
        free(jsonData);
        return 1;
    }

    cJSON *elem ;
    for(int i = 0; i < sizeof(json_entries)/sizeof(json_entries[0]); i++ ) {
        JSON_ENTRY json_entry = json_entries[i];
        elem = cJSON_GetObjectItem(json, json_entry.name);
        //printf("json_entry name %s type %d\n",json_entry.name, elem->type);
        if (elem != NULL && elem->type == cJSON_Number) {
            if(json_entry.type == FLOAT) {
                *(float *) json_entry.varptr = elem->valuedouble;
            }
            else {
                *(int *) json_entry.varptr = elem->valueint;
            }
        }
        else if (elem != NULL && elem->type == cJSON_String) {
            strcpy((char*) json_entry.varptr ,elem->valuestring);
        }

        else if (elem != NULL && elem->type == cJSON_Object) {
            if (json_entry.type == WRITECOND) {
                writeCond[0].min_h_spd = cJSON_GetObjectItem(elem, "min_h_spd")->valuedouble; 
                writeCond[0].max_h_spd = cJSON_GetObjectItem(elem, "max_h_spd")->valuedouble;
                writeCond[0].min_v_spd = cJSON_GetObjectItem(elem, "min_v_spd")->valuedouble;
                writeCond[0].max_v_spd = cJSON_GetObjectItem(elem, "max_v_spd")->valuedouble; 
                writeCond[0].min_x = cJSON_GetObjectItem(elem, "min_x")->valuedouble; 
                writeCond[0].max_x = cJSON_GetObjectItem(elem, "max_x")->valuedouble;
                writeCond[0].min_y = cJSON_GetObjectItem(elem, "min_y")->valuedouble;
                writeCond[0].max_y = cJSON_GetObjectItem(elem, "max_y")->valuedouble; 
                writeCond[0].min_z = cJSON_GetObjectItem(elem, "min_z")->valuedouble;
                writeCond[0].max_z = cJSON_GetObjectItem(elem, "max_z")->valuedouble; 
                writeCond[0].state = get_act(cJSON_GetObjectItem(elem, "state")->valuestring) & 0x1FF;
    

            }

        }
        //areas and constraints
        else if(elem != NULL && elem->type == cJSON_Array) {
            printf("name of list %s\n", json_entry.name);
            if(json_entry.type == AREALIST) {
                cJSON *area = NULL;
                int i = 0;
                cJSON_ArrayForEach(area, elem) {
                    if (area != NULL && area->type == cJSON_Object) {
                        SETAREAQU(areas,i,area);

                        printf("area=%d %f %f %f %f %f %f\n",i, areas[i].qu.min_x,
                                    areas[i].qu.max_x,
                                    areas[i].qu.min_y,
                                    areas[i].qu.max_y,
                                    areas[i].qu.min_z,
                                    areas[i].qu.max_z);

                        cJSON* lines = cJSON_GetObjectItem(area, "lines");
                        if (lines != NULL && lines->type == cJSON_Array) {
                            cJSON *line = NULL;
                            int j = 0;
                            cJSON_ArrayForEach(line, lines) {
                                SETAREALINES(areas, i,j,line);

                                printf("line=%d %f %f %f %f\n",j,  areas[i].lines[j].x1,
                                                                areas[i].lines[j].y1,
                                                                areas[i].lines[j].x2,
                                                                areas[i].lines[j].y2);
                        
                                j++;
                            }
                            areas[i].lcount = j;
                        }
                        i++;
                    }
                }
                gAreaCount = i;
            }
            else if(json_entry.type == CONSTRLIST) {
                printf("hello\n");
                cJSON *constr = NULL;
                int i = 0;
                cJSON_ArrayForEach(constr, elem) {
                    if(constr != NULL && constr->type == cJSON_Object) {
                        CONSTR* gConstr = (CONSTR*) json_entry.varptr;
                        SETCONSTR(gConstr,i, constr);
                        printf("i=%d state=%x %f %f %f %f\n",i ,gConstr[i].state, gConstr[i].min_h_spd,
                                                    gConstr[i].max_h_spd,
                                                    gConstr[i].min_v_spd,
                                                    gConstr[i].max_v_spd);

                        i++;
                    }
                }
                if(strstr(json_entry.name, "gWhiteList")) {
                    gWhiteListCount = i;
                }
                else if (strstr(json_entry.name, "gBlackList")){
                    gBlackListCount = i;
                }
            }
            //Lists
        }
        //
    }

    for(int i = 0; i < sizeof(json_entries)/sizeof(json_entries[0]); i++ ) {
        JSON_ENTRY json_entry = json_entries[i];
        if( json_entry.type == NUMBER ) {
            printf("%s: %d\n", json_entry.name, *(int*)json_entry.varptr);
        }
        else if( json_entry.type == STRING ) {
            printf("%s: %s\n", json_entry.name, (char*)json_entry.varptr);
        }
        else if( json_entry.type == FLOAT ) {
            printf("%s: %f\n", json_entry.name, *(float*)json_entry.varptr);
        }
    }

    cJSON_Delete(json);
    free(jsonData);

    return 0;
}
