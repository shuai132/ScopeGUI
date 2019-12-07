#pragma once

#include <functional>
#include "imgui.h"

namespace ImGui {

void PlotFFT(const char* label, const std::function<float(float* data, size_t i)>& values_getter, float* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 frame_size);

}
