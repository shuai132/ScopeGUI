#pragma once

#include "base/noncopyable.h"
#include "SmartSerial.h"
#include "PacketProcessor.h"
#include "algorithm/MsgAnalyzer.h"

using namespace scope;

class Comm : noncopyable {
public:
    Comm();

public:
    void setPortName(const std::string& name);

    std::string getPortName();

    bool isOpen();

    void setRecvEnable(bool enable);

    void sendCmd(Cmd cmd);

    void sendCmd(Cmd::Type type, Cmd::Data data = {});

private:
    void initSerial();

    void onMessage(const Message& message);

public:
    MsgAnalyzer msgAnalyzer;

private:
    PacketProcessor packetProcessor_;

    // Serial Port
    SmartSerial smartSerial_;
    const char* PORT_VID = "1234";
    const char* PORT_PID = "5740";

    bool recvEnable_ = true;
};
