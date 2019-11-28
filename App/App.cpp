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
            sampleFs_ = info.sampleFs;
            sampleNum_ = info.sampleNum;

            LOGD("got message: sampleFs:%d, sampleNum:%d", sampleFs_, sampleNum_);
            scaleMinVol_ = 0;
            scaleMaxVol_ = info.volMaxmV;

            for (uint32_t i = 0; i < sampleNum_; i++) {
                points_[0][i] = (float)msg->sampleCh1[i];
            }
        }

        {
            const uint16_t N = sampleNum_;
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

void App::update() {
    if (ImGui::InputInt("Sample Number", reinterpret_cast<int*>(&sampleNum_))) {
        if (sampleNum_ > SAMPLE_NUM_MAX) {
            sampleNum_ = SAMPLE_NUM_MAX;
        }
        Cmd cmd;
        cmd.type = Cmd::Type::SET_SAMPLE_NUM;
        cmd.data = Cmd::Data{.sampleNum = sampleNum_};
        packetProcessor_.packForeach((uint8_t*)&cmd, sizeof(cmd), [this](uint8_t* data, size_t size) {
            smartSerial_.write(data, size);
        });
    }

    if (ImGui::SliderInt("Sample Fs", reinterpret_cast<int*>(&sampleFs_), 0 , 50000)) {
        Cmd cmd;
        cmd.type = Cmd::Type::SET_SAMPLE_FS;
        cmd.data = Cmd::Data{.sampleFs = sampleFs_};
        packetProcessor_.packForeach((uint8_t*)&cmd, sizeof(cmd), [this](uint8_t* data, size_t size) {
            smartSerial_.write(data, size);
        });
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::GetFrameHeight();
    if (ImGui::InputText("serial port", port_, IM_ARRAYSIZE(port_))) {
        LOGD("open port");
        smartSerial_.getSerial()->setPort(port_);
    }

    ImGui::SliderFloat("xsize", &xsize_, 0, 50000);

    ImGui::Text("Sample Info: sampleFre=%u(%gkHz), sampleNum=%u", sampleFs_, sampleFs_ / 1000.f, sampleNum_);

    ImGui::PlotLines("Vol/mV", points_[0], sampleNum_, 0, "vol", scaleMinVol_, scaleMaxVol_, ImVec2(xsize_, 280));
    char overlay_text[128];
    sprintf(overlay_text, "FFT Basic Signal: fre=%.3f, amp=%.3f, pha=%.3f", fre_fft_, amp_fft_, pha_fft_);
    ImGui::PlotHistogram("FFT", points_[1], sampleNum_ / 2 + 1, 0, overlay_text, scaleMinFFT_, scaleMaxFFT_, ImVec2(xsize_, 280));
}
