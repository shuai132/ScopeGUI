#pragma once

#include <functional>
#include "imgui.h"

namespace ImGui {

void PlotVol(const char* label
        , const std::function<float(size_t i)>& values_getter
        , const std::function<const char*(size_t i)>& get_value_label
        , const std::function<const char*(size_t i)>& get_x_axis_text
        , int values_count
        , int values_offset
        , const char* overlay_text
        , float scale_min
        , float scale_max
        , ImVec2 frame_size
        , bool showTip = false);

}
