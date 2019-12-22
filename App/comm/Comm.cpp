#include "Comm.h"
#include "log.h"

Comm::Comm(AppContent* content)
    : scopeGui_(this), appContent_(content) {
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
    recvEnabled_ = enable;
}

void Comm::sendCmd(Cmd::Type type, Cmd::Data data) {
    if (not smartSerial_.isOpen()) {
        LOGD("not open, cmd ignored!");
        return;
    }
    scopeGui_.sendCmd(type, data);
}

void Comm::initSerial() {
    smartSerial_.setOnOpenHandle([this](bool isOpen) {
        if (isOpen) {
            sendCmd(Cmd::SOFTWARE_TRIGGER);
        } else {
            smartSerial_.setPortName("");
        }
    });

    smartSerial_.setVidPid(PORT_VID, PORT_PID);

    smartSerial_.setOnReadHandle([this](const uint8_t* data, size_t size) {
        if (not recvEnabled_) return;
        scopeGui_.onMcuData(data, size);
    });
}

void Comm::sendToMcu(const uint8_t* data, size_t size) {
    smartSerial_.write(data, size);
}

void Comm::onMessage(Message* message, size_t size) {
    if (processing_) return;
    processing_ = true;
    auto dataHolder = std::shared_ptr<uint8_t>(new uint8_t[size], std::default_delete<uint8_t[]>());
    memcpy(dataHolder.get(), message, size);
    appContent_->getUIContext().dispatch([this, dataHolder]{
        msgAnalyzer.onMessage((Message*)dataHolder.get());
        processing_ = false;
    });
}
