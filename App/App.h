#pragma once

#include "SmartSerial.h"
#include "PacketProcessor.h"
#include "Portable.h"

class App {
public:
    App();

public:
    void update();

private:
    SmartSerial smartSerial_;
    char port_[128] = "/dev/tty.usbmodem8D8842A649551";

    float points_[2][SAMPLE_NUM_MAX]{};

    float xsize_ = 1000;

    PacketProcessor packetProcessor_;

    uint32_t sampleFs_ = 0;
    uint32_t sampleNum_ = 0;

    float scaleMinVol_ = 0;
    float scaleMaxVol_ = 0;
    float scaleMinFFT_ = 0;
    float scaleMaxFFT_ = 0;

    float amp_fft_ = 0;
    float pha_fft_ = 0;
    float fre_fft_ = 0;
};
