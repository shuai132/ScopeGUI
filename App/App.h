#pragma once

#include "SmartSerial.h"
#include "PacketProcessor.h"
#include "Portable.h"
#include "fft.h"

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
    void onMessage(const Message& message);
    void calFFT();

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
    const char* PORT_VID = "1234";
    const char* PORT_PID = "5740";
    int portItemCurrent_ = 0;
    bool isOpen_ = false;

    // Wave
    std::vector<float> pointsAmp_;
    std::vector<float> pointsFFT_;

    // GUI
    uint32_t windowFlags_ = 0;
    bool isHold_ = false;

    const float itemWidth_ = 1200;
    const float itemWidthScaleMax_ = itemWidth_ * 10;
    float waveWidth_ = 0;
    float waveHeight_ = 255;

    float volMin_ = 0;
    float volMax_ = 0;
    float fftMin_ = 0;
    float fftMax_ = 0;

    float fftAmpMax_ = 0;

    // FFT Result
    std::vector<fft_complex> fftResult_;
    int   fftNum_ = 0;
    float fftAmp_ = 0;
    float fftPha_ = 0;
    float fftFre_ = 0;
};
