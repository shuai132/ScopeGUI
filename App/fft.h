#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double real;
    double imag;
} fft_complex;

void  fft_cal_fft(fft_complex* x, int n);
float fft_cal_amp(fft_complex x, int n);
float fft_cal_pha(fft_complex x);
float fft_cal_fre(int fs, int n, int k);

#ifdef __cplusplus
}
#endif
