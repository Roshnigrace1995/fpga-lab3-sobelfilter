#include "sobel_filter.h"
#include <ap_int.h>

// synthesis-safe clip
static inline pix_t clip8(grad_t v) {
    if (v < 0)   v = 0;
    if (v > 255) v = 255;
    return (pix_t)v;
}

void sobel_filter(const pix_t in[IMG_SIZE], pix_t out[IMG_SIZE]) {
#pragma HLS INLINE off

    // Sobel kernels
    const grad_t Gx[3][3] = {
        {-1,  0,  1},
        {-2,  0,  2},
        {-1,  0,  1}
    };
    const grad_t Gy[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1}
    };

    // Line buffers and 3x3 window
    pix_t linebuf[3][IMG_COLS];
#pragma HLS ARRAY_PARTITION variable=linebuf complete dim=1

    pix_t window[3][3];
#pragma HLS ARRAY_PARTITION variable=window complete dim=0

    //-------------------------------------------------------------
    // ✅ Initialize buffers to zero (fixes 'X' unknown values)
    //-------------------------------------------------------------
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < IMG_COLS; j++) {
#pragma HLS UNROLL factor=1
            linebuf[i][j] = 0;
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
            window[i][j] = 0;
        }
    }
    //-------------------------------------------------------------

    // ---------- Row loop (512 iters) ----------
#pragma HLS LOOP_TRIPCOUNT min=512 max=512
    for (int r = 0; r < IMG_ROWS; r++) {

        // ---------- Column loop (512 iters) ----------
#pragma HLS LOOP_TRIPCOUNT min=512 max=512
#pragma HLS UNROLL factor=1
        for (int c = 0; c < IMG_COLS; c++) {
#pragma HLS PIPELINE II=1   // maintain initiation interval = 1

            // Update line buffers
            linebuf[0][c] = linebuf[1][c];
            linebuf[1][c] = linebuf[2][c];
            linebuf[2][c] = in[r * IMG_COLS + c];

            // Build 3x3 window (explicit types)
            for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
                for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
                    int rr = r - 2 + i;
                    int cc = c - 1 + j;
                    pix_t v = 0;
                    if (rr >= 0 && rr < IMG_ROWS && cc >= 0 && cc < IMG_COLS) {
                        if (rr >= (r - 2))
                            v = linebuf[2 - (r - rr)][cc];
                        else
                            v = 0;
                    }
                    window[i][j] = v;
                }
            }

            // Convolution
            grad_t sumx = 0, sumy = 0;
            for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
                for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
                    sumx += (grad_t)Gx[i][j] * (grad_t)window[i][j];
                    sumy += (grad_t)Gy[i][j] * (grad_t)window[i][j];
                }
            }

            // |Gx| + |Gy|
            grad_t ax = (sumx < 0) ? (grad_t)(-sumx) : sumx;
            grad_t ay = (sumy < 0) ? (grad_t)(-sumy) : sumy;
            grad_t mag = ax + ay;

            out[r * IMG_COLS + c] = clip8(mag);
        }
    }
}
