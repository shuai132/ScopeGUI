#pragma once

#include "SmartSerial.h"
#include "PacketProcessor.h"
#include "Portable.h"

using namespace scope;

class App {
public:
    App();

public:
    void onDraw();

private:
    void initGUI();

    void drawSerial();
    void drawCmd();
    void drawWave();

private:
    void sendCmd(Cmd cmd);
    void sendCmd(Cmd::Type type, Cmd::Data data = {});

private:
    static uint32_t nextPow2(uint32_t v);

public:
    static const int MainWindowWidth  = 1280;
    static const int MainWindowHeight = 720;
    static const char* MainWindowTitle;

private:
    // ScopeMCU
    PacketProcessor packetProcessor_;
    SampleInfo info_ = {};

    // Serial Port
    SmartSerial smartSerial_;
    char port_[128] = "/dev/tty.usbmodem8D8842A649551";
    bool isOpen_ = false;

    // Wave
    float points_[2][SAMPLE_NUM_MAX]{};

    // GUI
    uint32_t windowFlags_ = 0;
    bool isHold_ = false;

    const float itemWidth_ = 1200;
    const float itemWidthScaleMax_ = itemWidth_ * 10;
    float waveWidth_ = 0;
    float waveHeight_ = 255;

    float scaleMinVol_ = 0;
    float scaleMaxVol_ = 0;
    float scaleMinFFT_ = 0;
    float scaleMaxFFT_ = 0;

    // FFT Result
    int fft_num_ = 0;
    float amp_fft_ = 0;
    float pha_fft_ = 0;
    float fre_fft_ = 0;
};
