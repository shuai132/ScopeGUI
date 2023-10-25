#include <memory>

#include "Comm.h"
#include "log.h"

Comm::Comm(AppContext* context)
    : scopeGui_(this), appContext_(context) {}

void Comm::init() {
    serialPort_ = std::make_unique<asio_net::serial_port>(*(asio::io_context*)appContext_->nativeHandle());
    initSerial();
}

void Comm::setPortName(const std::string& name) {
    serialPort_->close();
    serialPort_->open(name);
}

std::string Comm::getPortName() {
    return serialPort_->config().device;
}

bool Comm::isOpen() {
    return serialPort_->is_open();
}

void Comm::setRecvEnable(bool enable) {
    recvEnabled_ = enable;
}

void Comm::sendCmd(Cmd::Type type, Cmd::Data data) {
    auto now = std::chrono::steady_clock::now();

    // limit send cmd speed and ensure messages are received as quickly as possible
    bool canSend = cmdRespond_ || (now - cmdLastTime_ > std::chrono::milliseconds(100));
    if (not canSend) return;
    cmdLastTime_ = now;
    cmdRespond_ = false;

    if (not serialPort_->is_open()) {
        LOGD("not open, cmd ignored!");
        return;
    }
    scopeGui_.sendCmd(type, data);
}

void Comm::initSerial() {
    serialPort_->set_reconnect(1000);
    serialPort_->on_open = ([this]() {
      sendCmd(Cmd::SOFTWARE_TRIGGER);
    });
    serialPort_->open();
//    serialPort_->setVidPid(PORT_VID, PORT_PID);

    serialPort_->on_data = [this](const std::string& data) {
        if (not recvEnabled_) return;
        scopeGui_.onMcuData((uint8_t*)data.data(), data.size());
    };
}

void Comm::sendToMcu(const uint8_t* data, size_t size) {
    serialPort_->send(std::string((char*)data, size));
}

void Comm::onMessage(Message* message, size_t size) {
    if (processing_) return;
    processing_ = true;
    auto dataHolder = std::shared_ptr<uint8_t>(new uint8_t[size], std::default_delete<uint8_t[]>());
    memcpy(dataHolder.get(), message, size);
    appContext_->runOnUiThread([this, dataHolder]{
        msgAnalyzer.onMessage((Message*)dataHolder.get());
        processing_ = false;
        cmdRespond_ = true;
    });
}
