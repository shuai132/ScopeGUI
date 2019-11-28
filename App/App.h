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
    void drawCmd();
    void drawWave();

private:
    void sendCmd(Cmd cmd);
    void sendCmd(Cmd::Type type, Cmd::Data data);

private:
    SmartSerial smartSerial_;
    char port_[128] = "/dev/tty.usbmodem8D8842A649551";

    float points_[2][SAMPLE_NUM_MAX]{};

    float xsize_ = 1000;
    float ysize_ = 240;

    PacketProcessor packetProcessor_;

    SampleInfo info_ = {};

    float scaleMinVol_ = 0;
    float scaleMaxVol_ = 0;
    float scaleMinFFT_ = 0;
    float scaleMaxFFT_ = 0;

    float amp_fft_ = 0;
    float pha_fft_ = 0;
    float fre_fft_ = 0;
};
