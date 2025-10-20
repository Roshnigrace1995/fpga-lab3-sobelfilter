// tb_sobel.cpp — Testbench for Sobel HLS top (fixed 512x512 image)
// Expects PGM (P5 binary grayscale) input, writes PGM (P5) output.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>

#include "sobel_filter.h"   // defines pix_t, grad_t, IMG_ROWS/COLS, sobel_filter()

// ---------- P5 PGM I/O -------------------------------------------------------
static bool load_p5(const std::string &path, std::vector<pix_t> &img,
                    int &rows, int &cols) {
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) { std::cerr << "Cannot open " << path << "\n"; return false; }

    char magic[3] = {0};
    if (fscanf(f, "%2s", magic) != 1 || std::strcmp(magic, "P5") != 0) {
        std::cerr << "Not a P5 PGM: " << path << "\n";
        fclose(f); return false;
    }

    // Skip comment lines
    int c = fgetc(f);
    while (c == '#') { while (c != '\n' && c != EOF) c = fgetc(f); c = fgetc(f); }
    ungetc(c, f);

    int w=0, h=0, maxv=0;
    if (fscanf(f, "%d %d", &w, &h) != 2) { fclose(f); return false; }
    if (fscanf(f, "%d", &maxv) != 1 || maxv <= 0 || maxv > 255) { fclose(f); return false; }
    fgetc(f); // consume one whitespace after header

    rows = h; cols = w;
    img.resize((size_t)rows * cols);
    size_t n = fread(img.data(), 1, (size_t)rows * cols, f);
    fclose(f);
    return n == (size_t)rows * cols;
}

static bool save_p5(const std::string &path, const std::vector<pix_t> &img,
                    int rows, int cols) {
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) { std::cerr << "Cannot write " << path << "\n"; return false; }
    std::fprintf(f, "P5\n%d %d\n255\n", cols, rows);
    fwrite(img.data(), 1, (size_t)rows * cols, f);
    fclose(f);
    return true;
}
// -----------------------------------------------------------------------------

int main() {
    // === Absolute paths (edit if your folder differs) ========================
    const std::string in_path  =
        "C:/Users/rose4/OneDrive/Desktop/fpga-lab3-sobel/images/lena_input.pgm";
    const std::string out_path =
        "C:/Users/rose4/OneDrive/Desktop/fpga-lab3-sobel/images/lena_output.pgm";
    // =========================================================================

    int rows = 0, cols = 0;
    std::vector<pix_t> in_img, out_img;

    // Load input image
    if (!load_p5(in_path, in_img, rows, cols)) {
        std::cerr << "[TB] Failed to load " << in_path << "\n";
        return 1;
    }

    // Ensure expected 512x512 for the fixed-size top
    if (rows != IMG_ROWS || cols != IMG_COLS) {
        std::cerr << "[TB] ERROR: expected " << IMG_ROWS << "x" << IMG_COLS
                  << " but got " << rows << "x" << cols << "\n";
        return 4;
    }

    out_img.resize((size_t)rows * cols);

    // Call HLS top (fixed-size interface)
    sobel_filter(in_img.data(), out_img.data());

    // Save result
    if (!save_p5(out_path, out_img, rows, cols)) {
        std::cerr << "[TB] Failed to save " << out_path << "\n";
        return 2;
    }

    std::cout << "[TB] Sobel completed. rows=" << rows
              << " cols=" << cols << "\n";
    std::cout << "[TB] Wrote: " << out_path << "\n";

    // Simple sanity check
    size_t nonzero = 0;
    for (pix_t v : out_img) if (v != 0) ++nonzero;
    if (nonzero == 0) {
        std::cerr << "[TB] ERROR: Output appears empty.\n";
        return 3;
    }

    std::cout << "[TB] PASS (non-zero edge pixels: " << nonzero << ")\n";
    return 0;
}
