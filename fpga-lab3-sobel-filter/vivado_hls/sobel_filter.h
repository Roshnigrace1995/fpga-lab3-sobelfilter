#ifndef SOBEL_FILTER_H
#define SOBEL_FILTER_H

#include <ap_int.h>

#ifndef IMG_ROWS
#define IMG_ROWS 512
#endif
#ifndef IMG_COLS
#define IMG_COLS 512
#endif
#define IMG_SIZE (IMG_ROWS * IMG_COLS)

// 8-bit grayscale pixel
typedef ap_uint<8> pix_t;

// ==== Bit-width optimized types ====
// |sumx|, |sumy| ≤ 1020  → needs signed 11 bits
typedef ap_int<11>  grad_t;
// |sumx|+|sumy| ≤ 2040   → needs unsigned 12 bits
typedef ap_uint<12> mag_t;

void sobel_filter(const pix_t in[IMG_SIZE], pix_t out[IMG_SIZE]);

#endif

