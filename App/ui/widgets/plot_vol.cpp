#include "plot_vol.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

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
        , bool showTip)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    if (frame_size.x == 0.0f)
        frame_size.x = CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0, &frame_bb))
        return;
    const bool hovered = ItemHoverable(frame_bb, id);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = values_getter(i);
            if (v != v) // Ignore NaN values
                continue;
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = 2;
    if (values_count >= values_count_min)
    {
        int res_w = ImMin((int)frame_size.x, values_count) - 1;
        int item_count = values_count - 1;

        // Tooltip on hover
        int v_hovered = -1;
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            if (showTip) {
                SetNextWindowBgAlpha(0.1);
                SetTooltip("%s", get_value_label(v_idx));
            }
            v_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        float v0 = values_getter((0 + values_offset) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle

        const ImU32 col_base = GetColorU32(ImGuiCol_PlotLines);
        const ImU32 col_hovered = GetColorU32(ImVec4(0.90f, 0.70f, 0.00f, 1.00f));

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = values_getter((v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, tp1);

            float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands
            ImVec2 posX = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(tp0.x, histogram_zero_line_t));

            window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);

            {   // 选中点高亮
                if (v_hovered == v1_idx) {
                    auto labelText = get_value_label(n);
                    // label标记在更合适的位置
                    auto textPos = pos0;
                    auto fontHeight = window->DrawList->_Data->FontSize;
                    auto labelWidth = fontHeight / 2 * strlen(labelText);

                    // 正常情况 label在点的右下方
                    // 往右靠一点 往下移动一点
                    float padding = 5;
                    textPos.x = textPos.x + padding;
                    textPos.y = textPos.y + fontHeight + padding;

                    textPos += ImVec2{fontHeight / 2, - fontHeight};

                    // label超出底部时 往上移动
                    if (textPos.y + fontHeight > (frame_bb.Max - style.FramePadding).y) {
                        textPos.y = textPos.y - fontHeight - padding * 2;
                    }

                    // label超出右侧时 往左移动
                    if (textPos.x + labelWidth > (frame_bb.Max - style.FramePadding).x) {
                        textPos.x = textPos.x - labelWidth - fontHeight - padding * 2;
                    }

                    window->DrawList->AddText(textPos, col_hovered, labelText);
                    window->DrawList->AddCircleFilled(pos0, 3, col_hovered);
                }
            }
            {   // 横坐标
                auto xAxisText = get_x_axis_text(n);
                if (xAxisText) {
                    window->DrawList->AddText(posX + ImVec2{1, 0}, col_base, xAxisText);
                    window->DrawList->AddRectFilled(ImVec2{posX.x - 1, posX.y}, ImVec2{posX.x + 1, posX.y + 4}, col_hovered);
                }
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, nullptr, nullptr, ImVec2(0.5f,0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
}

}
