#pragma once

#include "Framework.h"

#include <optional>

// Handle mouse movement
// Handle Wintab events

// This is a singleton class as only one instance of input can be listenned to
// TODO: fully support the mouse without break changes

class Inputs {
  public:
    Inputs(const Inputs &) = delete;
    Inputs(Inputs &&) = delete;
    Inputs &operator=(const Inputs &) = delete;
    Inputs &operator=(Inputs &&) = delete;

    Inputs() = default;
    ~Inputs() = default;

    std::optional<LRESULT> HandleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    struct Packet {
        double orientation = 0.0; // Stylus angle compared to the surface 0 = facing north, 0.25 facing east, etc...
        double rotation = 0.0;    // Rotation of the stylus clockwise 0.0 no rotation 360.0 full rotation
        double pressure = 1.0;    // How much the pen is pressed the value is normalized from 0.0 to 1.0
        double tilt = 0.0;        // How much is the stylus parralle to the surface, 0.0 perpendicular, 1.0 coplanar
        double x = 0.0;           // X position of the stylus in window space
        double y = 0.0;           // Y position of the stylus in window space
    };

    Packet _current_packet, _previous_packet;

  private:
    int _x;
    int _y;
    int _width;
    int _height;

    // Modifier key
    bool _control;
    bool _alt;
    bool _shift;

    // stylus state
    bool _stylus_leave;
    bool _stylus_hover;
    double _wheel_y;
    double _wheel_x;

    // Mouse button
    bool _middle;
    bool _left;
    bool _right;
    bool _xbutton1;
    bool _xbutton2;

  private:
    void GetInputData(LPARAM &lparam);
};