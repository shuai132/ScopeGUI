#include "UIComm.h"
#include "imgui.h"
#include "serial/serial.h"

using namespace ImGui;

UIComm::UIComm(Comm* comm)
    : comm_(comm) {
}

void UIComm::onDraw() {
    SetNextItemWidth(500);

    static std::vector<serial::PortInfo> ports = serial::list_ports();
    {
        // save a bit of cpu
        static const auto minInterval = std::chrono::seconds(1);
        static auto lastTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - lastTime >= minInterval) {
            lastTime = now;
            ports = serial::list_ports();
        }
    }

    const int MAX_NAME_LEN = 128;
    char items[ports.size()][MAX_NAME_LEN];

    auto portNameCurrent = comm_->getPortName();
    if (!portNameCurrent.empty()) {
        portItemCurrent_ = 0;
        for (const auto& info : ports) {
            if (info.port == portNameCurrent) break;
            portItemCurrent_++;
        }
    }

    if (ImGui::Combo("Serial Port", &portItemCurrent_, [](void* data, int idx, const char** out_str) {
        *out_str = ports[idx].port.c_str();
        return true;
    }, items, IM_ARRAYSIZE(items))) {
        comm_->setPortName(ports[portItemCurrent_].port);
    }

    SameLine();
    Text("%s", comm_->isOpen() ? "(Opened!)" : "(Closed!)");
}
