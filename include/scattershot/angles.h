#ifndef ANGLES_H
#define ANGLES_H
#include "stdint.h"
#include "util.h"
#include <stdbool.h>

#define max_inx_diff 96
#define max_angle 65536
#define min_(a, b) (a) < (b) ? (a) : (b)
#define max_(a, b) (a) > (b) ? (a) : (b)

void match_yaw(Input* in, int angle, bool reverse);

#endif
