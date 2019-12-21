#include "MsgAnalyzer.h"
#include "utils/Utils.h"
#include "log.h"

void MsgAnalyzer::onMessage(Message* message) {
    info = message->sampleInfo;
    //LOGD("got message: sampleFs:%d, sampleSn:%d", info.sampleFs, info.sampleSn);

    pointsAmp.resize(info.sampleSn);

    volMin = info.volMinmV;
    volMax = info.volMaxmV;

    FOR(i, info.sampleSn) {
        pointsAmp[i] = (float) message->sampleCh1[i];
    }

    calcFFT();
}

void MsgAnalyzer::calcFFT() {
    // FFT算法需要N为2的整次幂
    fftNum = utils::nextPow2(info.sampleSn);
    pointsFFT.resize(fftNum / 2);
    fftResult.resize(fftNum);

    const auto& Fs = info.sampleFs;    // 采样频率
    const auto& Fn = info.sampleSn;    // 采样点数
    const auto& N  = fftNum;           // FFT点数

    // FFT
    auto s = fftResult.data();
    FOR (i, N) {
        s[i].real = i < Fn ? pointsAmp[i] : 0; // 自动补零
        s[i].imag = 0;
    }

    // 对s做N点FFT 结果仍保存在s中
    fft_cal_fft(s, N);

    // 计算幅值并找出除直流分量外最大处:k
    float max = 0;
    auto& A = pointsFFT;
    auto& k = fftK;
    k = 0;
    if (fftCursor == 0) {
        // 初始化为直流分量
        fftCursor = (float)fft_cal_amp(s[0], Fn);;
    }
    FOR (i, N / 2) {
        A[i] = (float)fft_cal_amp(s[i], Fn);
        if (i != 0 && A[i] < fftCursor && A[i] > max) {
            max = A[i];
            k = i;
        }
    }

    // 计算k处幅值、相位和频率
    fftAmp = A[k];
    fftPha = (float)fft_cal_pha(s[k]);
    fftFre = (float)fft_cal_fre(Fs, N, k);
    //LOGD("k=%d, max=%f, %f, %f, %f", k, max, fftAmp, fftPha, fftFre);

    fftMin = 0;
    fftMax = A[0];
}
