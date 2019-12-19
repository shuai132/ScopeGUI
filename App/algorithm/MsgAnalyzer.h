#pragma once

#include <vector>
#include "base/noncopyable.h"
#include "comm/Portable.h"
#include "algorithm/fft.h"

using namespace scope;

class MsgAnalyzer : noncopyable {
public:
    void onMessage(const Message& message);

    void calcFFT();

public:
    // ScopeMCU
    SampleInfo info = {};

    // Wave
    std::vector<float> pointsAmp;
    std::vector<float> pointsFFT;

    float volMin = 0;
    float volMax = 0;
    float fftMin = 0;
    float fftMax = 0;

    float fftCursor = 0;

    // FFT Result
    std::vector<fft_complex> fftResult;
    typeof(SampleInfo::sampleSn) fftNum = 0;
    typeof(fftNum) fftK = 0;
    float fftAmp = 0;
    float fftPha = 0;
    float fftFre = 0;
};
