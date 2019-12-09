#pragma once

#include "UI.h"
#include "comm/Comm.h"

class UIVol : UI {
public:
    explicit UIVol(Comm* comm);

public:
    void onDraw() override;

private:
    Comm* comm_;
};
