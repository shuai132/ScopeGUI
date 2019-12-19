#include "Comm.h"
#include "App.h"
#include "log.h"

Comm::Comm(AppContent* content) : packetProcessor_(false), appContent_(content) {
    initSerial();
}

void Comm::setPortName(const std::string& name) {
    smartSerial_.setPortName(name);
}

std::string Comm::getPortName() {
    return smartSerial_.getPortName();
}

bool Comm::isOpen() {
    return smartSerial_.isOpen();
}

void Comm::setRecvEnable(bool enable) {
    recvEnable_ = enable;
}

void Comm::sendCmd(Cmd cmd) {
    if (not smartSerial_.isOpen()) {
        LOGD("not open, cmd ignored!");
        return;
    }
    packetProcessor_.packForeach((uint8_t*)&cmd, sizeof(cmd), [this](uint8_t* data, size_t size) {
        smartSerial_.write(data, size);
    });
}

void Comm::sendCmd(Cmd::Type type, Cmd::Data data) {
    Cmd cmd;
    cmd.type = type;
    cmd.data = data;
    sendCmd(cmd);
}

void Comm::initSerial() {
    smartSerial_.setOnOpenHandle([this](bool isOpen) {
        if (isOpen) {
            sendCmd(Cmd::Type::SOFTWARE_TRIGGER);
        } else {
            smartSerial_.setPortName("");
        }
    });

    smartSerial_.setVidPid(PORT_VID, PORT_PID);

    smartSerial_.setOnReadHandle([this](const uint8_t* data, size_t size) {
        if (not recvEnable_) return;
        packetProcessor_.feed(data, size);
    });

    packetProcessor_.setOnPacketHandle([this](const uint8_t* data, size_t size) {
        if (processing) return;
        processing = true;
        auto dataHolder = std::shared_ptr<uint8_t>(new uint8_t[size], [this](const uint8_t* p) {
            delete[] p;
            processing = false;
        });
        memcpy(dataHolder.get(), data, size);
        appContent_->getUIContext().dispatch([this, dataHolder]{
            onMessage(*(Message*)dataHolder.get());
        });
    });
}

void Comm::onMessage(const Message& message) {
    msgAnalyzer.onMessage(message);
}
