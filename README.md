# FPGA Lab 3 – Sobel Filter (Vivado HLS)

This project implements the Sobel edge detection algorithm using Vivado HLS 2019.2.
The workflow begins in MATLAB for input image conversion and verification, followed by C/C++ implementation and optimization in Vivado HLS.

## Folder Structure
- **matlab/** – MATLAB script for converting the grayscale image to `.ppm`
- **vivado_hls/** – C/C++ source files for Sobel filter, including:
  - `sobel_filter.cpp` – baseline implementation
  - `sobel_filter_databitwidth.cpp` – data bit-width optimization
  - `sobel_filter_loopopt.cpp` – loop pipelining optimization
  - `tb_sobel.cpp` – testbench for simulation

## Optimizations
- **Baseline**: sequential Sobel edge detection
- **Data bit-width**: reduced variable precision to save resources
- **Loop pipelining**: improved throughput with II = 1

## Tools Used
- MATLAB R2023b
- Vivado HLS 2019.2 (Target: Kintex-7 xc7k70t-fbv676-1)

---

**Author:** Roshni Grace Vijayan  
Lehigh University – Department of Electrical & Computer Engineering
