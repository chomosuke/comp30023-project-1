#ifndef HEAD
#define HEAD

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef unsigned ID;
typedef unsigned Time;

typedef int bool;
#define true 1
#define false 0

#define IDLENGTH 16 /* 10 for 2^32, 4 for up to 1024 CPU, 1 for . and 1 for \0 */

typedef int Type;
#define RUNNING 1
#define FINISHED 0

#endif