#include "App.h"
#include "imgui.h"
#include "Portable.h"
#include "Utils.h"
#include "log.h"

App::App() : packetProcessor_(false) {
    initSerial();

    initGUI();
}

void App::onDraw() {
    using namespace ImGui;

    ImGui::Begin("MainWindow", nullptr, windowFlags_);
    ImGui::SetWindowPos({0, 0});
    ImGui::SetWindowSize({MainWindowWidth, MainWindowHeight});
    PushItemWidth(itemWidth_);

    Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / GetIO().Framerate, GetIO().Framerate);

    drawSerial();

    drawCmd();

    drawWave();

    PopItemWidth();
    ImGui::End();
}

void App::initSerial() {
    smartSerial_.setOnOpenHandle([this](bool isOpen) {
        if (isOpen) {
            sendCmd(Cmd::Type::SOFTWARE_TRIGGER);
        } else {
            smartSerial_.setPortName("");
        }
    });

    smartSerial_.setVidPid(PORT_VID, PORT_PID);

    smartSerial_.setOnReadHandle([this](const uint8_t* data, size_t size) {
        packetProcessor_.feed(data, size);
    });

    packetProcessor_.setOnPacketHandle([this](const uint8_t* data, size_t size) {
        onMessage(*(Message*)data);
    });
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

    static std::vector<serial::PortInfo> ports;
    ports = serial::list_ports();
    const int MAX_NAME_LEN = 128;
    char items[ports.size()][MAX_NAME_LEN];

    auto portNameCurrent = smartSerial_.getPortName();
    if (!portNameCurrent.empty()) {
        portItemCurrent_ = 0;
        for (const auto& info : ports) {
            if (info.port == portNameCurrent) break;
            portItemCurrent_++;
        }
    }

    if (ImGui::Combo("Serial Port", &portItemCurrent_, [](void* data, int idx, const char** out_str) {
        *out_str = ports[idx].port.c_str();
        return true;
    }, items, IM_ARRAYSIZE(items))) {
        smartSerial_.setPortName(ports[portItemCurrent_].port);
    }

    SameLine();
    Text("%s", smartSerial_.isOpen() ? "(Opened!)" : "(Closed!)");
}

void App::drawCmd() {
    using namespace ImGui;

    {
        float widthSampleNum = 125;

        /** Sample Number **/
        SetNextItemWidth(widthSampleNum);
        int sampleNum = info_.sampleSn;
        if (InputInt("Sample Number", &sampleNum)) {
            utils::setLimit<int>(sampleNum, 0, info_.sampleNumMax);
            info_.sampleSn = sampleNum;
            sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleSn});
        }
        SameLine();
        SetNextItemWidth(500);
        if (SliderInt("##Sample Num Slider", &sampleNum, 0, info_.sampleNumMax, "Fn = %d")) {
            info_.sampleSn = sampleNum;
            sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleSn});
        }
        SameLine();
        Text("= %gk", (float)sampleNum / 1000);

        /** Sample Fs **/
        int sampleFs = info_.sampleFs;
        SetNextItemWidth(widthSampleNum);
        if (InputInt("Sample Fs    ", &sampleFs)) {
            utils::setLimit<int>(sampleFs, info_.fsMinSps, info_.fsMaxSps);
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
    Text("Sample Info from MCU: Fs=%u(%gkHz), SampleNum=%u", info_.sampleFs, info_.sampleFs / 1000.f, info_.sampleSn);

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

        char text[30]{}; {
            auto sn = info_.sampleSn;
            if (sn != 0){
                double ave = 0;
                FOR(i, sn) {
                    ave += pointsAmp_[i];
                }
                snprintf(text, sizeof(text), "Vol/mV: ave=%dmV", (int)ave / sn);
            }
        }
        PlotLines("AMP", pointsAmp_.data(), info_.sampleSn, 0, text, volMin_, volMax_, ImVec2(waveWidth_, waveHeight_));
    }

    // FFT plot
    {
        if (VSliderFloat("##FFT Amp Level", ImVec2(vSliderWidth, waveHeight_), &fftCursor_, 0, fftMax_)) {
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
    if (not smartSerial_.isOpen()) {
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

void App::calFFT() {
    // FFT算法需要N为2的整次幂
    fftNum_ = utils::nextPow2(info_.sampleSn);
    pointsFFT_.reserve(fftNum_ / 2);
    fftResult_.reserve(fftNum_);

    const auto& Fs = info_.sampleFs;    // 采样频率
    const auto& Fn = info_.sampleSn;    // 采样点数
    const auto& N  = fftNum_;           // FFT点数

    // FFT
    auto s = fftResult_.data();
    FOR (i, N) {
        s[i].real = i < Fn ? pointsAmp_[i] : 0; // 自动补零
        s[i].imag = 0;
    }

    // 对s做N点FFT 结果仍保存在s中
    fft_cal_fft(s, N);

    // 计算幅值并找出除直流分量外最大处:k
    float max = 0;
    auto& A = pointsFFT_;
    std::remove_const<typeof(N)>::type k = 0;
    if (fftCursor_ == 0) {
        // 初始化为直流分量
        fftCursor_ = (float)fft_cal_amp(s[0], Fn);;
    }
    FOR (i, N / 2) {
        A[i] = (float)fft_cal_amp(s[i], Fn);
        if (i != 0 && A[i] < fftCursor_ && A[i] > max) {
            max = A[i];
            k = i;
        }
    }

    // 计算k处幅值、相位和频率
    fftAmp_ = A[k];
    fftPha_ = (float)fft_cal_pha(s[k]);
    fftFre_ = (float)fft_cal_fre(Fs, N, k);
    //LOGD("k=%d, max=%f, %f, %f, %f", k, max, fftAmp_, fftPha_, fftFre_);

    fftMin_ = 0;
    fftMax_ = A[0];
}

void App::onMessage(const Message& message) {
    if (isHold_) return;

    info_ = message.sampleInfo;
    //LOGD("got message: sampleFs:%d, sampleSn:%d", info_.sampleFs, info_.sampleSn);

    pointsAmp_.reserve(info_.sampleSn);

    volMin_ = info_.volMinmV;
    volMax_ = info_.volMaxmV;

    FOR (i, info_.sampleSn) {
        pointsAmp_[i] = (float)message.sampleCh1[i];
    }

    calFFT();
}

const char* App::MainWindowTitle = "ScopeGUI";
