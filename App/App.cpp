#include "App.h"
#include "imgui.h"
#include "ui/commondef.h"

using namespace ImGui;

App::App()
    : comm_(std::unique_ptr<Comm>(new Comm(this)))
    , uiComm_(comm_.get())
    , uiCmd_(comm_.get())
    , uiVol_(comm_.get())
    , uiFFT_(comm_.get())
    {
    initGUI();
}

void App::onDraw() {
    uiContext_.poll();
    uiContext_.restart();

    ImGui::Begin("MainWindow", nullptr, windowFlags_);
    ImGui::SetWindowPos({0, 0});
    ImGui::SetWindowSize({MainWindowWidth, MainWindowHeight});
    PushItemWidth(UI_ITEM_WIDTH);

    Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / GetIO().Framerate, GetIO().Framerate);

    uiComm_.onDraw();

    uiCmd_.onDraw();

    Dummy({0, 4});

    uiVol_.onDraw();

    Dummy({0, 1});

    uiFFT_.onDraw();

    PopItemWidth();
    ImGui::End();
}

void App::runOnUiThread(const std::function<void()>& task) {
    uiContext_.dispatch(task);
}

void App::post(const std::function<void()>& task) {
    uiContext_.post(task);
}

void App::initGUI() {
    windowFlags_ |= ImGuiWindowFlags_NoMove;
    windowFlags_ |= ImGuiWindowFlags_NoResize;
    windowFlags_ |= ImGuiWindowFlags_NoCollapse;
    windowFlags_ |= ImGuiWindowFlags_NoTitleBar;
}

const char* App::MainWindowTitle = "ScopeGUI";
