#pragma once

#include "comm/Comm.h"
#include "ui/UIComm.h"
#include "ui/UICmd.h"
#include "ui/UIVol.h"
#include "ui/UIFFT.h"
#include "AppContext.h"
#include "asio.hpp"

class App : UI, AppContext {
public:
    App();

public:
    void onDraw() override;

    void runOnUiThread(const std::function<void()>& task) override;

    void post(const std::function<void()>& task) override;

    void *nativeHandle() override;

private:
    void initGUI();

public:
    static const int MainWindowWidth  = 1280;
    static const int MainWindowHeight = 720;
    static const char* MainWindowTitle;

private:
    asio::io_context uiContext_;
    std::unique_ptr<Comm> comm_;

    int windowFlags_ = 0;

    UIComm uiComm_;
    UICmd uiCmd_;
    UIVol uiVol_;
    UIFFT uiFFT_;
};
