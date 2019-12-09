#pragma once

#include "UI.h"
#include "comm/Comm.h"

class UIFFT : UI {
public:
    explicit UIFFT(Comm* comm);

public:
    void onDraw() override;

private:
    Comm* comm_;
};
