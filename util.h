//
// Created by imd on 7/3/20.
//

#ifndef VIDEOCARDS_UTIL_H
#define VIDEOCARDS_UTIL_H

#define TILE_SIZE 32
#define WPT 4 // Might work better if set to 8

#define WIDTH 4
#if WIDTH == 1
typedef cl_float floatX;
#elif WIDTH == 2
typedef cl_float2 floatX;
#elif WIDTH == 4
typedef cl_float4 floatX;
#elif WIDTH == 8
typedef cl_float8 floatX;
#endif

#define PREFIX_SUM_WORK_SIZE 256

#endif //VIDEOCARDS_UTIL_H
