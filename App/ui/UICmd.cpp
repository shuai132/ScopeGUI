#include "UICmd.h"
#include "imgui.h"
#include "utils/Utils.h"

using namespace ImGui;

UICmd::UICmd(Comm* comm)
        : comm_(comm) {
}

void UICmd::onDraw() {
    auto& info = comm_->msgAnalyzer.info;

    {
        float widthSampleNum = 125;

        /** Sample Number **/
        SetNextItemWidth(widthSampleNum);
        int sampleNum = info.sampleSn;
        if (InputInt("Sample Number", &sampleNum)) {
            utils::setLimit<int>(sampleNum, 0, info.sampleNumMax);
            info.sampleSn = sampleNum;
            comm_->sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info.sampleSn});
        }
        SameLine();
        SetNextItemWidth(500);
        if (SliderInt("##Sample Num Slider", &sampleNum, 0, info.sampleNumMax, "Fn = %d")) {
            info.sampleSn = sampleNum;
            comm_->sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info.sampleSn});
        }
        SameLine();
        Text("= %gk", (float)sampleNum / 1000);

        /** Sample Fs **/
        int sampleFs = info.sampleFs;
        SetNextItemWidth(widthSampleNum);
        if (InputInt("Sample Fs    ", &sampleFs)) {
            utils::setLimit<int>(sampleFs, info.fsMinSps, info.fsMaxSps);
            info.sampleFs = sampleFs;
            comm_->sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info.sampleFs});
        }
        SameLine();
        SetNextItemWidth(500);
        if (SliderInt("##Sample Fs Slider", &sampleFs, info.fsMinSps, info.fsMaxSps, "Fs = %d")) {
            info.sampleFs = sampleFs;
            comm_->sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info.sampleFs});
        }
        SameLine();
        Text("= %gk", (float)sampleFs / 1000);
    }

    // trigger mode
    {
        Text("Trigger Mode:");
        SameLine();
        int triggerMode = static_cast<int>(info.triggerMode);
        if (RadioButton("ALWAYS", &triggerMode, static_cast<int>(TriggerMode::ALWAYS))) {
            info.triggerMode = static_cast<TriggerMode>(triggerMode);
            comm_->sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info.triggerMode});
        }
        SameLine();
        if (RadioButton("NORMAL", &triggerMode, static_cast<int>(TriggerMode::NORMAL))) {
            info.triggerMode = static_cast<TriggerMode>(triggerMode);
            comm_->sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info.triggerMode});
        }
        SameLine();
        if (RadioButton("SOFTWARE", &triggerMode, static_cast<int>(TriggerMode::SOFTWARE))) {
            info.triggerMode = static_cast<TriggerMode>(triggerMode);
            comm_->sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info.triggerMode});
        }
    }

    // trigger slope
    {
        Text("Trigger Slope:");
        SameLine();
        int triggerSlope = static_cast<int>(info.triggerSlope);
        if (RadioButton("Up", &triggerSlope, static_cast<int>(TriggerSlope::UP))) {
            info.triggerSlope = static_cast<TriggerSlope>(triggerSlope);
            comm_->sendCmd(Cmd::Type::SET_TRIGGER_SLOPE, Cmd::Data{.triggerSlope = info.triggerSlope});
        }
        SameLine();
        if (RadioButton("Down", &triggerSlope, static_cast<int>(TriggerSlope::DOWN))) {
            info.triggerSlope = static_cast<TriggerSlope>(triggerSlope);
            comm_->sendCmd(Cmd::Type::SET_TRIGGER_SLOPE, Cmd::Data{.triggerSlope = info.triggerSlope});
        }

        {
            SameLine();
            if (Button("Force Trigger")) {
                comm_->sendCmd(Cmd::Type::SOFTWARE_TRIGGER);
            }

            SameLine();
            SetNextItemWidth(1000);
            if (Checkbox("Hold Wave", &isHold_)) {
                comm_->setRecvEnable(!isHold_);
            }
        }
    }
}
