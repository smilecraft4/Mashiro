#include "Tool.h"

#include <spdlog/spdlog.h>

PanTool::PanTool() {
}

void PanTool::Use() {
    spdlog::info("Using PanTool");
}

ZoomTool::ZoomTool() {
}

void ZoomTool::Use() {
    spdlog::info("Using ZoomTool");
}

RotateTool::RotateTool() {
}

void RotateTool::Use() {
    spdlog::info("Using RotateTool");
}

BrushTool::BrushTool() {
}

void BrushTool::Use() {
    spdlog::info("Using BrushTool");
}
