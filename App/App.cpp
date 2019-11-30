#include <cmath>

#include "App.h"
#include "imgui.h"
#include "Portable.h"
#include "log/log.h"

template <typename T>
static void setLimit(T& v, T min, T max) {
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }
}

App::App() : packetProcessor_(false) {
    smartSerial_.setVidPid(PORT_VID, PORT_PID);

    packetProcessor_.setOnPacketHandle([this](const uint8_t* data, size_t size) {
        onMessage(*(Message*)data);
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
    const auto& portName = smartSerial_.getSerial()->getPort();
    memcpy(port_, portName.c_str(), portName.size());
    if (InputText("Serial Port", port_, IM_ARRAYSIZE(port_))) {
        if (smartSerial_.getSerial()->isOpen()) {
            smartSerial_.getSerial()->close();
        }
        smartSerial_.setPortName(port_);
    }

    SameLine();
    Text("%s", isOpen_ ? "(Opened!)" : "(Closed!)");
}

void App::drawCmd() {
    using namespace ImGui;

    {
        float widthSampleNum = 125;

        // Sample Number
        SetNextItemWidth(widthSampleNum);
        int sampleNum = info_.sampleNum;
        if (InputInt("Sample Number", &sampleNum)) {
            setLimit<int>(sampleNum, 0, info_.sampleNumMax);
            info_.sampleNum = sampleNum;
            sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleNum});
        }
        SameLine();
        SetNextItemWidth(500);
        if (SliderInt("##Sample Num Slider", &sampleNum, 0, info_.sampleNumMax, "Fn = %d")) {
            info_.sampleNum = sampleNum;
            sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleNum});
        }
        SameLine();
        Text("= %gk", (float)sampleNum / 1000);

        /** Sample Fs **/
        int sampleFs = info_.sampleFs;
        SetNextItemWidth(widthSampleNum);
        if (InputInt("Sample Fs    ", &sampleFs)) {
            setLimit<int>(sampleFs, info_.fsMinSps, info_.fsMaxSps);
            info_.sampleFs = sampleFs;
            sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info_.sampleFs});
        }
        SameLine();
        SetNextItemWidth(500);
        if (SliderInt("##Sample Fs Slider", &sampleFs, info_.fsMinSps, info_.fsMaxSps, "Fs = %d")) {
            info_.sampleFs = sampleFs;
            sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info_.sampleFs});
        }
        SameLine();
        Text("= %gk", (float)sampleFs / 1000);
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
        if (VSliderInt("##Trigger Level", ImVec2(vSliderWidth, waveHeight_), &triggerLevel, info_.volMinmV, info_.volMaxmV)) {
            info_.triggerLevel = triggerLevel;
            sendCmd(Cmd::Type::SET_TRIGGER_LEVEL, Cmd::Data{.triggerLevel = info_.triggerLevel});
        }

        SameLine();

        PlotLines("AMP", pointsAmp_.data(), info_.sampleNum, 0, "Vol/mV", volMin_, volMax_, ImVec2(waveWidth_, waveHeight_));
    }

    // FFT plot
    {
        if (VSliderFloat("##FFT Amp Level", ImVec2(vSliderWidth, waveHeight_), &fftAmpMax_, 0, fftMax_)) {
            calFFT();
        }

        SameLine();

        const auto& fftNumber = fftNum_;
        char overlay_text[128];
        sprintf(overlay_text, "FFT Analysis(N=%d): fre=%.3fHz, amp=%.3fmV, pha=%.3f°", fftNumber, fftFre_, fftAmp_, fftPha_);
        PlotHistogram("FFT", pointsFFT_.data(), fftNumber / 2, 0, overlay_text, fftMin_, fftMax_, ImVec2(waveWidth_, waveHeight_));
    }
}

void App::sendCmd(Cmd cmd) {
    if (not isOpen_) {
        LOGD("not open, cmd ignored!");
        return;
    }
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

uint32_t App::nextPow2(uint32_t v) {
    v--;
    v |= v >> 1u;
    v |= v >> 2u;
    v |= v >> 4u;
    v |= v >> 8u;
    v |= v >> 16u;
    v++;
    return v;
}

void App::calFFT() {
    // FFT算法需要N为2的整次幂
    fftNum_ = nextPow2(info_.sampleNum);
    pointsFFT_.reserve(fftNum_ / 2);
    fftResult_.reserve(fftNum_);

    const uint16_t N = fftNum_;

    // FFT
    auto* s = fftResult_.data();
    for (int i = 0; i < N; i++) {
        s[i].real = i < info_.sampleNum ? pointsAmp_[i] : 0;
        s[i].imag = 0;
    }

    // 对s做N点FFT 结果仍保存在s中
    fft_cal_fft(s, N);

    // 找幅值最大处:k
    float max = 0;
    auto& A = pointsFFT_;
    int k = 0;
    if (fftAmpMax_ == 0) fftAmpMax_ = fftMax_;
    for (int i = 0; i < N / 2; i++) {
        A[i] = fft_cal_amp(s[i], N);
        if (i != 0 && A[i] < fftAmpMax_ && A[i] > max) {
            max = A[i];
            k = i;
        }
    }

    // 计算k处幅值、相位和频率
    fftAmp_ = A[k];
    fftPha_ = fft_cal_pha(s[k]);
    fftFre_ = fft_cal_fre(info_.sampleFs, N, k);
    //LOGD("k=%d, max=%f, %f, %f, %f", k, max, amp_fft_, pha_fft_, fre_fft_);

    fftMin_ = 0;
    fftMax_ = A[0];
}

void App::onMessage(const Message& message) {
    if (isHold_) return;

    info_ = message.sampleInfo;
    //LOGD("got messa11ge: sampleFs:%d, sampleNum:%d", info_.sampleFs, info_.sampleNum);

    pointsAmp_.reserve(info_.sampleNum);

    volMin_ = info_.volMinmV;
    volMax_ = info_.volMaxmV;

    for (uint32_t i = 0; i < info_.sampleNum; i++) {
        pointsAmp_[i] = (float)message.sampleCh1[i];
    }

    calFFT();
}

const char* App::MainWindowTitle = "ScopeGUI";
