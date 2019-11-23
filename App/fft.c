#include "fft.h"
#include <math.h>

#define pi  M_PI

static fft_complex cexp(fft_complex a)
{
    fft_complex z;
    z.real = (float)(exp(a.real) * cos(a.imag));
    z.imag = (float)(exp(a.real) * sin(a.imag));
    return z;
}

/**
 * 对s做N点FFT，结果仍保存在s中
 */
void fft_cal_fft(fft_complex* x, int n)
{
    fft_complex t, z, ce;
    float pisign;
    int mr, m, l, j, i, nn;
    int isign = -1;
    for (i = 1; i <= 16; i++)                       /*n must be power of 2  */
    {
        nn = (int)pow(2, i);
        if (n == nn)
            break;
    }
    if (i > 16)
    {
        //printf(" N is not a power of 2 ! \n");
        return;
    }
    z.real = 0.0;
    pisign = (float)(4 * isign * atan(1.0));        /*pisign的值为+180度或-180度*/

    mr = 0;
    for (m = 1; m < n; m++)
    {
        l = n;
        while (mr + l >= n)
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
    while (1)
    {
        if (l >= n)
        {
            if (isign == -1)                        /*isign=-1 For Forward Transform*/
                return;
            for (j = 0; j < n; j++)                 /* Inverse Transform */
            {
                x[j].real = x[j].real / n;
                x[j].imag = x[j].imag / n;
            }
            return;
        }
        for (m = 0; m < l; m++)                     /*完成当前级所有蝶形运算 */
        {
            for (i = m; i < n; i = i + 2 * l)       /*完成当前级相同W因子的所有蝶形运算*/
            {
                z.imag = m * pisign / l;
                ce = cexp(z);
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
float fft_cal_amp(fft_complex x, int n)
{
    return ((float)sqrt(x.real * x.real + x.imag * x.imag) * 2 / n);
}

/**
 * 由FFT结果计算相位
 */
float fft_cal_pha(fft_complex x)
{
    float pha = (double)atan(x.imag / x.real) * 180 / pi + 90;
    if(x.real < 0) {
        pha += 180;
    }
    return pha;
}

/**
 * 计算FFT结果中对应点的频率
 */
float fft_cal_fre(int fs, int n, int k)
{
    float fre = ((float)fs / n) * k;
    return fre;
}
