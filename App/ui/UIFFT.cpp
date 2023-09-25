#include <algorithm>
#include "UIFFT.h"
#include "commondef.h"
#include "widgets/plot_fft.h"
#include "log.h"

using namespace ImGui;

UIFFT::UIFFT(Comm* comm)
        : comm_(comm)  {
}

void UIFFT::onDraw() {
    auto& analyzer = comm_->msgAnalyzer;
    auto& info = analyzer.info;
    auto& fftCursor = analyzer.fftCursor;

    // Wave
    const auto& pointsFFT = analyzer.pointsFFT;
    const auto& fftMin = analyzer.fftMin;
    const auto& fftMax = analyzer.fftMax;

    // FFT Result
    const auto& fftNum = analyzer.fftNum;
    const auto& fftK = analyzer.fftK;
    const auto& fftAmp = analyzer.fftAmp;
    const auto& fftPha = analyzer.fftPha;
    const auto& fftFre = analyzer.fftFre;

    // FFT plot
    if (VSliderFloat("##FFT Amp Level", ImVec2(UI_WAVE_L_SLIDER_WIDTH, UI_WaveHeight), &fftCursor, 0, fftMax, "")) {
        analyzer.calcFFT();
    }

    SameLine();

    const auto& Fn = fftNum;
    char overlay_text[128];
    snprintf(overlay_text, sizeof(overlay_text), "FFT Analysis(N=%d): fre=%.3fHz, amp=%.3fmV, pha=%.3f°", Fn, fftFre, fftAmp, fftPha);

    std::vector<size_t> shouldShowAxisIdx;
    {
        size_t xCount = fftNum / 2;
        auto findMaxPos = [&](size_t pos) {
            float maxP = pos;
            float maxV = pointsFFT[pos];
            for (size_t i = pos; i < xCount; i++) {
                auto v = pointsFFT[i];
                if (maxV < v) {
                    maxV = v;
                    maxP = i;
                }
            }
            return maxP;
        };
        shouldShowAxisIdx.push_back(0);
        shouldShowAxisIdx.push_back(fftK);
        size_t showCountMax = 20;
        for (size_t i = fftK + 1; i < xCount && shouldShowAxisIdx.size() < showCountMax; i++) {
            i = findMaxPos(i);
            // 跳过过于密集的点
            if (i - shouldShowAxisIdx.back() < xCount / showCountMax) continue;
            shouldShowAxisIdx.push_back(i);
        }
    }

    PlotFFT(""
            , [&](size_t i) {
                return pointsFFT[i];
            }
            , [&](size_t i, bool force) {
                if (!force && i != fftK) return (char*)nullptr;

                static char v[56];
                auto fre = fft_cal_fre(info.sampleFs, fftNum, i);
                if (fre < 1000) {
                    snprintf(v, sizeof(v), "Fre=%gHz, Amp=%gmV, n=%zu", fre, fftAmp, i);
                } else {
                    snprintf(v, sizeof(v), "Fre=%gkHz, Amp=%gmV, n=%zu", fre / 1000, fftAmp, i);
                }
                return v;
            }
            , [&](size_t i) {
                if (i == 0) return (char*)"0";

                if (not std::binary_search(shouldShowAxisIdx.cbegin(), shouldShowAxisIdx.cend(), i)) {
                    return (char*)nullptr;
                }

                static char v[56];
                auto fre = fft_cal_fre(info.sampleFs, fftNum, i);
                if (fre < 1000) {
                    snprintf(v, sizeof(v), "%.1f", fre);
                } else {
                    snprintf(v, sizeof(v), "%.1fk", fre / 1000);
                }
                return v;
            }
            , Fn / 2
            , 0
            , overlay_text
            , fftMin
            , fftMax
            , ImVec2(UI_WaveWidth, UI_WaveHeight)
    );
}
