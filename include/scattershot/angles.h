#ifndef ANGLES_H
#define ANGLES_H
#include "stdint.h"
#include "util.h"
#include <stdbool.h>

#define max_inx_diff 96
#define max_angle 65536

void match_yaw(Input* in, int angle, bool reverse);

#endif
