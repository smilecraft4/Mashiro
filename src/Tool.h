#pragma once

class Tool {
  public:
    virtual void Use() = 0;
};


class PanTool : Tool {
  public:
    PanTool();

    void Use() override;
};

class ZoomTool : Tool {
  public:
    ZoomTool();

    void Use() override;
};

class RotateTool : Tool {
  public:
    RotateTool();

    void Use() override;
};

class BrushTool : Tool {
  public:
    BrushTool();

    void Use() override;
};