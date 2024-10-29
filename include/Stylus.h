#pragma once

#include "Framework.h"

#include <optional>

// Handle mouse movement
// Handle Wintab events

// This is a singleton class as only one instance of input can be listenned to
// TODO: fully support the mouse without break changes

// TODO: directly handling events may be more easir but if the runtime is not fast enough there will be visible lag so
// maybe instead using a deferred method like with events is better
#define MS_STYLUSMOVE 0x0500
#define MS_STYLUSBUTTON 0x0501
#define MS_STYLUSDOWN 0x0502
#define MS_STYLUSUP 0x0503
#define MS_STYLUSENTER 0x0504
#define MS_STYLUSEXIT 0x0505
#define MS_STYLUSHOVER 0x0506 // FIXME: This event is not firing & I don't know why :c
#define MS_STYLUSWHEEL 0x0507

class Stylus {
  public:
    Stylus(const Stylus &) = delete;
    Stylus(Stylus &&) = delete;
    Stylus &operator=(const Stylus &) = delete;
    Stylus &operator=(Stylus &&) = delete;

    Stylus() = default;
    ~Stylus() = default;

    bool HandleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *result);

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