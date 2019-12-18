#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double real;
    double imag;
} fft_complex;

void   fft_cal_fft(fft_complex* x, uint32_t N);
double fft_cal_amp(fft_complex x, uint32_t N);
double fft_cal_pha(fft_complex x);
double fft_cal_fre(uint32_t Fs, uint32_t N, uint32_t n);

#ifdef __cplusplus
}
#endif
