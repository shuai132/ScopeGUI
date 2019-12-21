#pragma once

#include "base/noncopyable.h"
#include "SmartSerial.h"
#include "algorithm/MsgAnalyzer.h"
#include "AppContent.h"
#include "ScopeGUI.h"

using scope::Cmd;
using scope::ScopeGUI;

class Comm : noncopyable, private ScopeGUI::Comm {
public:
    explicit Comm(AppContent* content);

private:
    void sendToMcu(const uint8_t* data, size_t size) override;

    void onMessage(Message* message, size_t size) override;

public:
    void setPortName(const std::string& name);

    std::string getPortName();

    bool isOpen();

    void setRecvEnable(bool enable);

    void sendCmd(Cmd::Type type, Cmd::Data data = {});

private:
    void initSerial();

public:
    MsgAnalyzer msgAnalyzer;

private:
    ScopeGUI scopeGui_;

    // Serial Port
    SmartSerial smartSerial_;
    const char* PORT_VID = "1234";
    const char* PORT_PID = "5740";

    std::atomic_bool recvEnabled_ {true};

    AppContent* appContent_;
    std::atomic_bool processing_ {false};
};
