#include "sobel_filter.h"
#include <ap_int.h>

static inline pix_t clip8(int v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (pix_t)v;
}

// Helpers for ±1 / ±2 multiply without DSPs
static inline int mul_pos1(int x) { return  x; }
static inline int mul_neg1(int x) { return -x; }
static inline int mul_pos2(int x) { return  x << 1; }
static inline int mul_neg2(int x) { return -(x << 1); }

void sobel_filter(const pix_t in[IMG_SIZE], pix_t out[IMG_SIZE]) {
#pragma HLS INLINE off

    // Sliding 3×3 window in regs
    pix_t win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    // Three line buffers (circular), one full row each
    pix_t linebuf[3][IMG_COLS];
#pragma HLS RESOURCE variable=linebuf core=RAM_T2P_BRAM
#pragma HLS ARRAY_PARTITION variable=linebuf complete dim=1  // partition the 3 rows

Row_Loop:
    for (int r = 0; r < IMG_ROWS; r++) {

        const int write_idx =  r      % 3; // r
        const int prev1_idx = (r + 2) % 3; // r-1
        const int prev2_idx = (r + 1) % 3; // r-2

    Col_Loop:
        for (int c = 0; c < IMG_COLS; c++) {
            // (We’re not forcing PIPELINE here since you’re postponing it.)

            // Shift window left
            win[0][0]=win[0][1]; win[0][1]=win[0][2];
            win[1][0]=win[1][1]; win[1][1]=win[1][2];
            win[2][0]=win[2][1]; win[2][1]=win[2][2];

            const int idx = r * IMG_COLS + c;

            // New right column (with safe casts)
            const pix_t p2 = in[idx];
            const pix_t p1 = (r >= 1) ? linebuf[prev1_idx][c] : (pix_t)0;
            const pix_t p0 = (r >= 2) ? linebuf[prev2_idx][c] : (pix_t)0;

            linebuf[write_idx][c] = p2;

            win[0][2] = p0;
            win[1][2] = p1;
            win[2][2] = p2;

            // Border default
            out[idx] = 0;

            if (r >= 2 && c >= 2) {
                // ---- Bit-width optimized accumulators ----
                grad_t sumx = 0;
                grad_t sumy = 0;

                // Unroll the fixed 3×3
#pragma HLS UNROLL
                for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
                    for (int j = 0; j < 3; j++) {
                        int w = (int)win[i][j];

                        // Gx kernel: [-1 0 1; -2 0 2; -1 0 1]
                        // Gy kernel: [ 1 2 1;  0 0 0; -1 -2 -1]
                        // Implement via adds/shifts to avoid DSPs.
                        int gx_contrib = 0;
                        int gy_contrib = 0;

                        // map (i,j) to coeffs quickly
                        if (i == 0) { // row -1
                            if (j == 0) { gx_contrib += mul_neg1(w); gy_contrib += mul_pos1(w); }
                            if (j == 1) { /* gx+=0 */               gy_contrib += mul_pos2(w); }
                            if (j == 2) { gx_contrib += mul_pos1(w); gy_contrib += mul_pos1(w); }
                        } else if (i == 1) { // row 0
                            if (j == 0) { gx_contrib += mul_neg2(w); /* gy+=0 */ }
                            if (j == 1) { /* gx+=0 */               /* gy+=0 */ }
                            if (j == 2) { gx_contrib += mul_pos2(w); /* gy+=0 */ }
                        } else { // i == 2, row +1
                            if (j == 0) { gx_contrib += mul_neg1(w); gy_contrib += mul_neg1(w); }
                            if (j == 1) { /* gx+=0 */               gy_contrib += mul_neg2(w); }
                            if (j == 2) { gx_contrib += mul_pos1(w); gy_contrib += mul_neg1(w); }
                        }

                        sumx += (grad_t)gx_contrib;
                        sumy += (grad_t)gy_contrib;
                    }
                }

                // |Gx| + |Gy| staying inside 12 bits
                ap_uint<11> ax = (sumx < 0) ? (ap_uint<11>)(-sumx) : (ap_uint<11>)sumx;
                ap_uint<11> ay = (sumy < 0) ? (ap_uint<11>)(-sumy) : (ap_uint<11>)sumy;
                mag_t mag = (mag_t)ax + (mag_t)ay;  // ≤ 2040

                const int out_idx = (r - 1) * IMG_COLS + (c - 1);
                out[out_idx] = clip8((int)mag);
            }
        }
    }
}
