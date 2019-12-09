#pragma once

#include "UI.h"
#include "comm/Comm.h"

class UICmd : UI {
public:
    explicit UICmd(Comm* comm);

public:
    void onDraw() override;

private:
    Comm* comm_;

    bool isHold_ = false;
};
