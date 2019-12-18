#include "fft.h"
#include <math.h>
#include <assert.h>

#define pi  M_PI

static fft_complex cp_cexp(fft_complex a)
{
    fft_complex z;
    z.real = (float)(exp(a.real) * cos(a.imag));
    z.imag = (float)(exp(a.real) * sin(a.imag));
    return z;
}

/**
 * 对s做N点FFT，结果仍保存在s中
 */
void fft_cal_fft(fft_complex* x, uint32_t N)
{
    fft_complex t, z, ce;
    double pisign;
    uint32_t mr, m, l, i;
    int isign = -1;

    assert(!(N & (N - 1)));                         // n must be power of 2

    z.real = 0.0;
    pisign = 4 * isign * atan(1.0);                 // pisign的值为+180度或-180度

    mr = 0;
    for (m = 1; m < N; m++)
    {
        l = N;
        while (mr + l >= N)
            l = l / 2;
        mr = mr % l + l;
        if (mr <= m)
            continue;
        t.real = x[m].real;
        t.imag = x[m].imag;
        x[m].real = x[mr].real;
        x[m].imag = x[mr].imag;
        x[mr].real = t.real;
        x[mr].imag = t.imag;
    }

    l = 1;
    while (l < N)
    {
        for (m = 0; m < l; m++)                     /*完成当前级所有蝶形运算 */
        {
            for (i = m; i < N; i = i + 2 * l)       /*完成当前级相同W因子的所有蝶形运算*/
            {
                z.imag = m * pisign / l;
                ce = cp_cexp(z);
                t.real = x[i + l].real * ce.real - x[i + l].imag * ce.imag;
                t.imag = x[i + l].real * ce.imag + x[i + l].imag * ce.real;
                x[i + l].real = x[i].real - t.real; /*原位运算*/
                x[i + l].imag = x[i].imag - t.imag;
                x[i].real = x[i].real + t.real;
                x[i].imag = x[i].imag + t.imag;
            }
        }
        l = 2 * l;                                  /*确定下一级蝶形运算中W因子个数*/
    }
}

/**
 * 由FFT结果计算幅值
 */
double fft_cal_amp(fft_complex x, uint32_t N)
{
    return (sqrt(x.real * x.real + x.imag * x.imag) * 2 / N);
}

/**
 * 由FFT结果计算相位
 */
double fft_cal_pha(fft_complex x)
{
    double pha = atan(x.imag / x.real) * 180 / pi + 90;
    if(x.real < 0) {
        pha += 180;
    }
    return pha;
}

/**
 * 计算FFT结果中对应点的频率
 */
double fft_cal_fre(uint32_t Fs, uint32_t N, uint32_t n)
{
    double fre = ((double)Fs / N) * n;
    return fre;
}
