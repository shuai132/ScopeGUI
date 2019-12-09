#pragma once

#include "base/noncopyable.h"

class UI : noncopyable {
    virtual void onDraw() = 0;
};
