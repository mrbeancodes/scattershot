#ifndef JSON_H
#define JSON_H

#include "scattershot/settings.h"
#include "scattershot/so.h"

#define FLOAT 1
#define STRING 2
#define NUMBER 3
#define AREALIST 4
#define CONSTRLIST 5
#define WRITECOND 6

#define SETAREAQU(areas,i,area) \
    areas[i].qu.min_x = cJSON_GetObjectItem(area, "min_x")->valuedouble;\
    areas[i].qu.max_x = cJSON_GetObjectItem(area, "max_x")->valuedouble;\
    areas[i].qu.min_y = cJSON_GetObjectItem(area, "min_y")->valuedouble; \
    areas[i].qu.max_y = cJSON_GetObjectItem(area, "max_y")->valuedouble; \
    areas[i].qu.min_z = cJSON_GetObjectItem(area, "min_z")->valuedouble; \
    areas[i].qu.max_z = cJSON_GetObjectItem(area, "max_z")->valuedouble; 

#define SETAREALINES(areas, i,j,line) \
    areas[i].lines[j].x1 = cJSON_GetObjectItem(line, "x1")->valuedouble;\
    areas[i].lines[j].y1 = cJSON_GetObjectItem(line, "y1")->valuedouble;\
    areas[i].lines[j].x2 = cJSON_GetObjectItem(line, "x2")->valuedouble;\
    areas[i].lines[j].y2 = cJSON_GetObjectItem(line, "y2")->valuedouble;

#define SETCONSTR(gConstr, i, constr) \
    gConstr[i].min_h_spd = cJSON_GetObjectItem(constr, "min_h_spd")->valuedouble; \
    gConstr[i].max_h_spd = cJSON_GetObjectItem(constr, "max_h_spd")->valuedouble; \
    gConstr[i].min_v_spd = cJSON_GetObjectItem(constr, "min_v_spd")->valuedouble; \
    gConstr[i].max_v_spd = cJSON_GetObjectItem(constr, "max_v_spd")->valuedouble; \
    gConstr[i].state = get_act(cJSON_GetObjectItem(constr, "state")->valuestring) & 0x1FF;


typedef struct  {
    void* varptr;
    int type;
    char name[100]; 
} JSON_ENTRY;

int init_settings(char* json_name);

#endif
