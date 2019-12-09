#pragma once

#include "UI.h"
#include "comm/Comm.h"

class UIComm : UI {
public:
    explicit UIComm(Comm* comm);

public:
    void onDraw() override;

private:
    Comm* comm_;

    int portItemCurrent_ = -1;
};
