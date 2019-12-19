#pragma once

#include "comm/Comm.h"
#include "ui/UIComm.h"
#include "ui/UICmd.h"
#include "ui/UIVol.h"
#include "ui/UIFFT.h"
#include "AppContent.h"

class App : UI, AppContent {
public:
    App();

public:
    void onDraw() override;

private:
    void initGUI();

public:
    static const int MainWindowWidth  = 1280;
    static const int MainWindowHeight = 720;
    static const char* MainWindowTitle;

private:
    std::unique_ptr<Comm> comm_;

    uint32_t windowFlags_ = 0;

    UIComm uiComm_;
    UICmd uiCmd_;
    UIVol uiVol_;
    UIFFT uiFFT_;
};
