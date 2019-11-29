#include <cmath>

#include "App.h"
#include "imgui.h"
#include "Portable.h"
#include "fft.h"
#include "log/log.h"

App::App() : packetProcessor_(false) {
    smartSerial_.getSerial()->setPort(port_);

    packetProcessor_.setOnPacketHandle([this](const uint8_t* data, size_t size) {
        if (isHold_) return;

        auto msg = (Message*)data;
        auto info = msg->sampleInfo;

        if (info.sampleNum > SAMPLE_NUM_MAX) {
            LOGE();
        }

        info_ = info;

        {
            //LOGD("got message: sampleFs:%d, sampleNum:%d", info_.sampleFs, info_.sampleNum);
            scaleMinVol_ = 0;
            scaleMaxVol_ = info.volMaxmV;

            for (uint32_t i = 0; i < info_.sampleNum; i++) {
                points_[0][i] = (float)msg->sampleCh1[i];
            }
        }

        {
            const uint16_t N = info_.sampleNum;
            fft_complex s[N];
            auto& signal = points_[0];
            for (int i = 0; i < N; i++) {
                s[i].real = signal[i];
                s[i].imag = 0;
            }

            // 对s做N点FFT，结果仍保存在s中
            fft_cal_fft(s, N);

            // 找幅值最大处:k
            float max = 0;
            auto& A = points_[1];
            int k = 0;
            for (int i = 0; i < N / 2; i++) {
                A[i] = fft_cal_amp(s[i], N);
                if (i != 0 && A[i] > max) {
                    max = A[i];
                    k = i;
                }
            }

            // 计算k处幅值、相位和频率
            amp_fft_ = A[k];
            pha_fft_ = fft_cal_pha(s[k]);
            fre_fft_ = fft_cal_fre(info.sampleFs, N, k);
            //LOGD("k=%d, max=%f, %f, %f, %f", k, max, amp_fft_, pha_fft_, fre_fft_);

            scaleMinFFT_ = 0;
            scaleMaxFFT_ = A[0];
        }
    });

    smartSerial_.setOnReadHandle([this](const uint8_t* data, size_t size) {
        packetProcessor_.feed(data, size);
    });

    smartSerial_.setOnOpenHandle([this](bool isOpen) {
        isOpen_ = isOpen;
        if (isOpen_) {
            sendCmd(Cmd::Type::SOFTWARE_TRIGGER);
        }
    });

    initGUI();
}

void App::onDraw() {
    using namespace ImGui;

    ImGui::Begin("MainWindow", nullptr, windowFlags_);
    PushItemWidth(itemWidth_);

    Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / GetIO().Framerate, GetIO().Framerate);

    drawSerial();

    drawCmd();

    drawWave();

    PopItemWidth();
    ImGui::End();
}

void App::initGUI() {
    windowFlags_ |= ImGuiWindowFlags_NoMove;
    windowFlags_ |= ImGuiWindowFlags_NoResize;
    windowFlags_ |= ImGuiWindowFlags_NoCollapse;
    windowFlags_ |= ImGuiWindowFlags_NoTitleBar;
}

void App::drawSerial() {
    using namespace ImGui;

    SetNextItemWidth(500);
    if (InputText("Serial Port", port_, IM_ARRAYSIZE(port_))) {
        if (smartSerial_.getSerial()->isOpen()) {
            smartSerial_.getSerial()->close();
        }
        smartSerial_.getSerial()->setPort(port_);
    }

    SameLine();
    Text("%s", isOpen_ ? "(Opened!)" : "(Closed!)");
}

void App::drawCmd() {
    using namespace ImGui;

    {
        float widthSampleNum = 125;

        SetNextItemWidth(widthSampleNum);
        int sampleNum = info_.sampleNum;
        if (InputInt("Sample Number", &sampleNum)) {
            if (sampleNum > SAMPLE_NUM_MAX) {
                sampleNum = SAMPLE_NUM_MAX;
            }
            else if (sampleNum < 2) {
                sampleNum = 2;
            }
            info_.sampleNum = sampleNum;
            sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleNum});
        }

        SameLine();
        int sampleFs = info_.sampleFs;

        SetNextItemWidth(widthSampleNum);
        if (InputInt("Sample Fs", &sampleFs)) {
            info_.sampleFs = sampleFs;
            sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info_.sampleFs});
        }

        if (sampleFs >= 1000) {
            SameLine();
            Text("(%gk)", (float)sampleFs / 1000);
        }

        SetNextItemWidth(500);
        if (SliderInt("##Sample Fs Slider", &sampleFs, 0 , 50000)) {
            info_.sampleFs = sampleFs;
            sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info_.sampleFs});
        }

    }

    // trigger mode
    {
        Text("Trigger Mode:");
        SameLine();
        int triggerMode = static_cast<int>(info_.triggerMode);
        if (RadioButton("ALWAYS", &triggerMode, static_cast<int>(TriggerMode::ALWAYS))) {
            info_.triggerMode = static_cast<TriggerMode>(triggerMode);
            sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info_.triggerMode});
        }
        SameLine();
        if (RadioButton("NORMAL", &triggerMode, static_cast<int>(TriggerMode::NORMAL))) {
            info_.triggerMode = static_cast<TriggerMode>(triggerMode);
            sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info_.triggerMode});
        }
        SameLine();
        if (RadioButton("SOFTWARE", &triggerMode, static_cast<int>(TriggerMode::SOFTWARE))) {
            info_.triggerMode = static_cast<TriggerMode>(triggerMode);
            sendCmd(Cmd::Type::SET_TRIGGER_MODE, Cmd::Data{.triggerMode = info_.triggerMode});
        }
    }

    // trigger slope
    {
        Text("Trigger Slope:");
        SameLine();
        int triggerSlope = static_cast<int>(info_.triggerSlope);
        if (RadioButton("Up", &triggerSlope, static_cast<int>(TriggerSlope::UP))) {
            info_.triggerSlope = static_cast<TriggerSlope>(triggerSlope);
            sendCmd(Cmd::Type::SET_TRIGGER_SLOPE, Cmd::Data{.triggerSlope = info_.triggerSlope});
        }
        SameLine();
        if (RadioButton("Down", &triggerSlope, static_cast<int>(TriggerSlope::DOWN))) {
            info_.triggerSlope = static_cast<TriggerSlope>(triggerSlope);
            sendCmd(Cmd::Type::SET_TRIGGER_SLOPE, Cmd::Data{.triggerSlope = info_.triggerSlope});
        }

        {
            SameLine();
            if (Button("Force Trigger")) {
                sendCmd(Cmd::Type::SOFTWARE_TRIGGER);
            }

            SameLine();
            SetNextItemWidth(1000);
            Checkbox("Hold Wave", &isHold_);
        }
    }
}

void App::drawWave() {
    using namespace ImGui;
    const float vSliderWidth = 14;

    NewLine();
    Text("Sample Info from MCU: Fs=%u(%gkHz), SampleNum=%u", info_.sampleFs, info_.sampleFs / 1000.f, info_.sampleNum);

    // Scale Slider
    {
        SetNextItemWidth(itemWidth_ - 71);
        SliderFloat("##Scale", &waveWidth_, itemWidth_, itemWidthScaleMax_);
        SameLine();
        if (Button("Reset Scale")) waveWidth_ = 0;
    }

    // Vol plot
    {
        int triggerLevel = info_.triggerLevel;
        if (VSliderInt("##Trigger Level", ImVec2(vSliderWidth, waveHeight_), &triggerLevel, 0, info_.volMaxmV)) {
            info_.triggerLevel = triggerLevel;
            sendCmd(Cmd::Type::SET_TRIGGER_LEVEL, Cmd::Data{.triggerLevel = info_.triggerLevel});
        }

        SameLine();

        PlotLines("AMP", points_[0], info_.sampleNum, 0, "Vol/mV", scaleMinVol_, scaleMaxVol_, ImVec2(waveWidth_, waveHeight_));
    }

    // FFT plot
    {
        static int fftNumber = info_.sampleNum;
        if (VSliderInt("##FFT Number", ImVec2(vSliderWidth, waveHeight_), &fftNumber, info_.sampleNum, SAMPLE_NUM_MAX * 10)) {
            // todo
        }

        SameLine();

        char overlay_text[128];
        sprintf(overlay_text, "FFT Analysis: fre=%.3fHz, amp=%.3fmV, pha=%.3f°", fre_fft_, amp_fft_, pha_fft_);
        PlotHistogram("FFT", points_[1], info_.sampleNum / 2 + 1, 0, overlay_text, scaleMinFFT_, scaleMaxFFT_, ImVec2(waveWidth_, waveHeight_));
    }
}

void App::sendCmd(Cmd cmd) {
    packetProcessor_.packForeach((uint8_t*)&cmd, sizeof(cmd), [this](uint8_t* data, size_t size) {
        smartSerial_.write(data, size);
    });
}

void App::sendCmd(Cmd::Type type, Cmd::Data data) {
    Cmd cmd;
    cmd.type = type;
    cmd.data = data;
    sendCmd(cmd);
}

const char* App::MainWindowTitle = "ScopeGUI";
