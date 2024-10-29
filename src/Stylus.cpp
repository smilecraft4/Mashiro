#include "Stylus.h"
#include "Log.h"
#include <format>

// Use digitizint context
#include "msgpack.h"
#include "wintab.h"

#include "pktdef.h"

bool Stylus::HandleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *result) {
    _wheel_x = 0.0;
    _wheel_y = 0.0;

    switch (msg) {
    // handle mouse movement
    case WM_MOUSEMOVE: {
        if (_stylus_leave) {
            _stylus_leave = false;
            PostMessage(hwnd, MS_STYLUSENTER, 0, 0);
        }

        _stylus_hover = false;

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = hwnd;
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.dwHoverTime = HOVER_DEFAULT;
        TrackMouseEvent(&tme);

        GetInputData(lparam);
        // LOG_INFO(TEXT("WM_MOUSEMOVE"));

        PostMessage(hwnd, MS_STYLUSMOVE, 0, 0);
        result = 0;
        return true;
    }
    case WM_MOUSEHWHEEL: {
        GetInputData(lparam);
        _wheel_x = (double)GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;
        // LOG_INFO(std::format(TEXT("WM_MOUSEHWHEEL: {:.2f}"), _wheel_x));

        PostMessage(hwnd, MS_STYLUSWHEEL, _wheel_x, _wheel_y);
        result = 0;
        return true;
    }
    case WM_MOUSEWHEEL: {
        GetInputData(lparam);
        _wheel_y = (double)GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;
        // LOG_INFO(std::format(TEXT("WM_MOUSEWHEEL: {:.2f}"), _wheel_y));

        PostMessage(hwnd, MS_STYLUSWHEEL, _wheel_x, _wheel_y);
        result = 0;
        return true;
    }
    case WM_MOUSEHOVER: {
        _stylus_hover = true;
        LOG_INFO(TEXT("WM_MOUSEHOVER"));

        PostMessage(hwnd, MS_STYLUSHOVER, 0, 0);
        result = 0;
        return true;
    }
    case WM_MOUSELEAVE: {
        _stylus_leave = true;
        // LOG_INFO(TEXT("WM_MOUSELEAVE"));

        PostMessage(hwnd, MS_STYLUSEXIT, 0, 0);
        result = 0;
        return true;
    }
    case WM_MOUSEACTIVATE:
        // LOG_INFO(TEXT("WM_MOUSEACTIVATE"));
        break;

    // handle mouse buttons
    case WM_LBUTTONDOWN: {
        GetInputData(lparam);
        _left = true;
        // LOG_INFO(TEXT("WM_LBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_LBUTTONDBLCLK: {
        GetInputData(lparam);
        _left = true;
        // LOG_INFO(TEXT("WM_LBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_LBUTTONUP: {
        GetInputData(lparam);
        _left = false;
        // LOG_INFO(TEXT("WM_LBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_RBUTTONDOWN: {
        GetInputData(lparam);
        _right = true;
        // LOG_INFO(TEXT("WM_RBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_RBUTTONDBLCLK: {
        GetInputData(lparam);
        _right = true;
        // LOG_INFO(TEXT("WM_RBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_RBUTTONUP: {
        GetInputData(lparam);
        _right = false;
        // LOG_INFO(TEXT("WM_RBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_MBUTTONDOWN: {
        GetInputData(lparam);
        _middle = true;
        // LOG_INFO(TEXT("WM_MBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_MBUTTONDBLCLK: {
        GetInputData(lparam);
        _middle = true;
        // LOG_INFO(TEXT("WM_MBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_MBUTTONUP: {
        GetInputData(lparam);
        _middle = false;
        // LOG_INFO(TEXT("WM_MBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_XBUTTONDOWN: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = true;
            // LOG_INFO(TEXT("WM_XBUTTON1DOWN"));

        } else {
            _xbutton2 = true;
            // LOG_INFO(TEXT("WM_XBUTTON2DOWN"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_XBUTTONDBLCLK: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = true;
            // LOG_INFO(TEXT("WM_XBUTTON1DBLCLK"));

        } else {
            _xbutton2 = true;
            // LOG_INFO(TEXT("WM_XBUTTON2DBLCLK"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }
    case WM_XBUTTONUP: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = false;
            // LOG_INFO(TEXT("WM_XBUTTON1UP"));

        } else {
            _xbutton2 = false;
            // LOG_INFO(TEXT("WM_XBUTTON2UP"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        result = 0;
        return true;
    }

    // Handle wintab event if supported
    case WT_PACKET:
    case WT_CTXOPEN:
    case WT_CTXCLOSE:
    case WT_CTXUPDATE:
    case WT_CTXOVERLAP:
    case WT_PROXIMITY:
    case WT_INFOCHANGE:
    case WT_CSRCHANGE:
    case WT_PACKETEXT:
    default:
        break;
    }

    // PostMessage(hwnd, MS_STYLUSPACKET, 0, 0);

    return false;
}

void Stylus::GetInputData(LPARAM &lparam) {
    _previous_packet = _current_packet;

    _control = GetKeyState(VK_CONTROL);
    _shift = GetKeyState(VK_SHIFT);
    _alt = GetKeyState(VK_MENU) < 0;
    _current_packet.x = GET_X_LPARAM(lparam);
    _current_packet.y = GET_Y_LPARAM(lparam);
}
