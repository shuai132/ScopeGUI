#include "UIVol.h"
#include "commondef.h"
#include "widgets/plot_vol.h"
#include "utils/Utils.h"
#include "log.h"

using namespace ImGui;

UIVol::UIVol(Comm* comm)
        : comm_(comm)  {
}

void UIVol::onDraw() {
    auto& analyzer = comm_->msgAnalyzer;
    auto& info = analyzer.info;
    const auto& pointsAmp = analyzer.pointsAmp;
    const auto& volMin = analyzer.volMin;
    const auto& volMax = analyzer.volMax;

    Text("Sample Info from MCU: Fs=%u(%gkHz), SampleNum=%u", info.sampleFs, info.sampleFs / 1000.f, info.sampleSn);

    // Scale Slider
    {
        SetNextItemWidth(UI_ITEM_WIDTH - 71);
        SliderFloat("##Scale", &UI_WaveWidth, UI_ITEM_WIDTH, UI_WAVE_WIDTH_SCALE_MAX);
        SameLine();
        if (Button("Reset Scale")) UI_WaveWidth = 0;
    }

    // Vol plot
    int triggerLevel = info.triggerLevel;
    if (VSliderInt("##Trigger Level", ImVec2(UI_WAVE_L_SLIDER_WIDTH, UI_WaveHeight), &triggerLevel, info.volMinmV, info.volMaxmV)) {
        info.triggerLevel = triggerLevel;
        comm_->sendCmd(Cmd::Type::SET_TRIGGER_LEVEL, Cmd::Data{.triggerLevel = info.triggerLevel});
    }

    SameLine();

    char text[30]{};
    {
        auto sn = info.sampleSn;
        double ave = 0;
        if (sn != 0){
            FOR(i, sn) {
                ave += pointsAmp[i];
            }
            ave = ave / sn;
        }
        snprintf(text, sizeof(text), "Vol/mV: ave=%dmV", (int)ave);
    }
    PlotVol(""
            , [&](size_t i) {
                return pointsAmp[i];
            }
            , [&](size_t i) {
                static char v[56];
                snprintf(v, sizeof(v), "%zu %gmV", i, pointsAmp[i]);
                return v;
            }
            , [&](size_t i) {
                size_t xCount = info.sampleSn;
                size_t showCountMax = 20;
                if (xCount > showCountMax && i % (xCount / showCountMax)) return (char*)nullptr;

                static char v[56];
                snprintf(v, sizeof(v), "%zu", i);
                return v;
            }
            , info.sampleSn
            , 0
            , text
            , volMin
            , volMax
            , ImVec2(UI_WaveWidth, UI_WaveHeight)
    );
}
