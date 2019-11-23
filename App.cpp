#include <cmath>

#include "imgui.h"
#include "App.h"

App::App() {

}

void App::update() {
    const int pointNum = 100;
    static float arr[pointNum] = { 0 };
    for (int i=0; i<pointNum; i++) {
        arr[i] = (float)sin(double(i) / pointNum * 2 * M_PI * 2);
    }

    static float ysize = 200;
    ImGui::SliderFloat("ysize", &ysize, 0, 500);

    ImGui::PlotLines("channel1", arr, IM_ARRAYSIZE(arr), 0, "overlay", -1.0f, 1.0f, ImVec2(0, ysize));
    ImGui::PlotLines("channel2", arr, IM_ARRAYSIZE(arr), 0, "overlay", -1.0f, 1.0f, ImVec2(0, ysize));
    ImGui::PlotLines("channel3", arr, IM_ARRAYSIZE(arr), 0, "overlay", -1.0f, 1.0f, ImVec2(0, ysize));
}
