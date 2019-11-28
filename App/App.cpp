#include <cmath>

#include "App.h"
#include "imgui.h"
#include "Portable.h"
#include "fft.h"
#include "log/log.h"

App::App() : packetProcessor_(false) {
    smartSerial_.getSerial()->setPort(port_);

    packetProcessor_.setOnPacketHandle([this](const uint8_t* data, size_t size) {
        auto msg = (Message*)data;
        auto info = msg->sampleInfo;
        assert(info.sampleNum <= SAMPLE_NUM_MAX);

        {
            info_ = info;

            LOGD("got message: sampleFs:%d, sampleNum:%d", info_.sampleFs, info_.sampleNum);
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
            LOGD("k=%d, max=%f, %f, %f, %f", k, max, amp_fft_, pha_fft_, fre_fft_);

            scaleMinFFT_ = 0;
            scaleMaxFFT_ = A[0];
        }
    });
    smartSerial_.setOnReadHandle([this](const uint8_t* data, size_t size) {
        packetProcessor_.feed(data, size);
    });
}


void App::onDraw() {
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (ImGui::InputText("Serial Port", port_, IM_ARRAYSIZE(port_))) {
        LOGD("open port");
        smartSerial_.getSerial()->setPort(port_);
    }

    drawCmd();

    drawWave();
}

void App::drawCmd() {
    int sampleFs = info_.sampleFs;
    if (ImGui::SliderInt("Sample Fs", &sampleFs, 0 , 50000)) {
        info_.sampleFs = sampleFs;
        sendCmd(Cmd::Type::SET_SAMPLE_FS, {.sampleFs = info_.sampleFs});
    }

    int sampleNum = info_.sampleNum;
    if (ImGui::InputInt("Sample Number", &sampleNum)) {
        info_.sampleNum = sampleNum;
        if (sampleNum > SAMPLE_NUM_MAX) {
            sampleNum = SAMPLE_NUM_MAX;
        }
        sendCmd(Cmd::Type::SET_SAMPLE_NUM, {.sampleNum = info_.sampleNum});
    }

    int triggerMode = static_cast<int>(info_.triggerMode);
    if (ImGui::SliderInt("Trigger Mode", &triggerMode, 0 , 2)) {
        info_.triggerMode = static_cast<TriggerMode>(triggerMode);
        sendCmd(Cmd::Type::SET_TRIGGER_MODE, {.triggerMode = info_.triggerMode});
    }

    int triggerSlope = static_cast<int>(info_.triggerSlope);
    if (ImGui::SliderInt("Trigger Slope", &triggerSlope, 0 , 1)) {
        info_.triggerSlope = static_cast<TriggerSlope>(triggerSlope);
        sendCmd(Cmd::Type::SET_TRIGGER_SLOPE, {.triggerSlope = info_.triggerSlope});
    }

    int triggerLevel = info_.triggerLevel;
    if (ImGui::SliderInt("Trigger Level", &triggerLevel, 0 , info_.volMaxmV)) {
        info_.triggerLevel = static_cast<TriggerLevel>(triggerLevel);
        sendCmd(Cmd::Type::SET_TRIGGER_LEVEL, {.triggerLevel = info_.triggerLevel});
    }
}

void App::drawWave() {
    ImGui::SliderFloat("xsize", &xsize_, 0, 50000);

    ImGui::Text("Sample Info: sampleFre=%u(%gkHz), sampleNum=%u", info_.sampleFs, info_.sampleFs / 1000.f, info_.sampleNum);

    ImGui::PlotLines("Vol/mV", points_[0], info_.sampleNum, 0, "vol", scaleMinVol_, scaleMaxVol_, ImVec2(xsize_, ysize_));
    char overlay_text[128];
    sprintf(overlay_text, "FFT Basic Signal: fre=%.3f, amp=%.3f, pha=%.3f", fre_fft_, amp_fft_, pha_fft_);
    ImGui::PlotHistogram("FFT", points_[1], info_.sampleNum / 2 + 1, 0, overlay_text, scaleMinFFT_, scaleMaxFFT_, ImVec2(xsize_, ysize_));
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
