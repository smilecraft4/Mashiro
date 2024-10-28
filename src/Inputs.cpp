#include "Inputs.h"
#include "Log.h"
#include <format>

// Use digitizint context
#include "msgpack.h"
#include "wintab.h"

#include "pktdef.h"

// #define MS_STYLUSPACKET 0x0500
#define MS_STYLUSDOWN 0x0501
#define MS_STYLUSUP 0x0502
#define MS_STYLUSENTER 0x0503
#define MS_STYLUSEXIT 0x0504
#define MS_STYLUSHOVER 0x0505
#define MS_STYLUSWHEEL 0x0506
#define MS_STYLUSMOVE 0x0507

#define MS_STYLUSBUTTON 0x0508

void GetMousePos(Inputs::Packet *packet, LPARAM lparam) {
}

std::optional<LRESULT> Inputs::HandleEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
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

        _control = GetKeyState(VK_CONTROL);
        _shift = GetKeyState(VK_SHIFT);
        _alt = GetKeyState(VK_MENU) < 0;
        _current_packet.x = GET_X_LPARAM(lparam);
        _current_packet.y = GET_Y_LPARAM(lparam);
        Log::Info(TEXT("WM_MOUSEMOVE"));

        PostMessage(hwnd, MS_STYLUSMOVE, 0, 0);
        return 0;
    }
    case WM_MOUSEHWHEEL: {
        GetInputData(lparam);
        _wheel_x = (double)GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;
        Log::Info(std::format(TEXT("WM_MOUSEHWHEEL: {:.2f}"), _wheel_x));

        PostMessage(hwnd, MS_STYLUSWHEEL, _wheel_x, _wheel_y);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        GetInputData(lparam);
        _wheel_y = (double)GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;
        Log::Info(std::format(TEXT("WM_MOUSEWHEEL: {:.2f}"), _wheel_y));

        PostMessage(hwnd, MS_STYLUSWHEEL, _wheel_x, _wheel_y);
        return 0;
    }
    case WM_MOUSEHOVER:
        _stylus_hover = true;
        Log::Info(TEXT("WM_MOUSEHOVER"));

        PostMessage(hwnd, MS_STYLUSHOVER, 0, 0);
        break;
    case WM_MOUSELEAVE:
        _stylus_leave = true;
        Log::Info(TEXT("WM_MOUSELEAVE"));

        PostMessage(hwnd, MS_STYLUSEXIT, 0, 0);
        break;
    case WM_MOUSEACTIVATE:
        Log::Info(TEXT("WM_MOUSEACTIVATE"));
        break;

    // handle mouse buttons
    case WM_LBUTTONDOWN: {
        GetInputData(lparam);
        _left = true;
        Log::Info(TEXT("WM_LBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_LBUTTONDBLCLK: {
        GetInputData(lparam);
        _left = true;
        Log::Info(TEXT("WM_LBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_LBUTTONUP: {
        GetInputData(lparam);
        _left = false;
        Log::Info(TEXT("WM_LBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_RBUTTONDOWN: {
        GetInputData(lparam);
        _right = true;
        Log::Info(TEXT("WM_RBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_RBUTTONDBLCLK: {
        GetInputData(lparam);
        _right = true;
        Log::Info(TEXT("WM_RBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_RBUTTONUP: {
        GetInputData(lparam);
        _right = false;
        Log::Info(TEXT("WM_RBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_MBUTTONDOWN: {
        GetInputData(lparam);
        _middle = true;
        Log::Info(TEXT("WM_MBUTTONDOWN"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_MBUTTONDBLCLK: {
        GetInputData(lparam);
        _middle = true;
        Log::Info(TEXT("WM_MBUTTONDBLCLK"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_MBUTTONUP: {
        GetInputData(lparam);
        _middle = false;
        Log::Info(TEXT("WM_MBUTTONUP"));

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_XBUTTONDOWN: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = true;
            Log::Info(TEXT("WM_XBUTTON1DOWN"));

        } else {
            _xbutton2 = true;
            Log::Info(TEXT("WM_XBUTTON2DOWN"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_XBUTTONDBLCLK: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = true;
            Log::Info(TEXT("WM_XBUTTON1DBLCLK"));

        } else {
            _xbutton2 = true;
            Log::Info(TEXT("WM_XBUTTON2DBLCLK"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
    }
    case WM_XBUTTONUP: {
        GetInputData(lparam);
        if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
            _xbutton1 = false;
            Log::Info(TEXT("WM_XBUTTON1UP"));

        } else {
            _xbutton2 = false;
            Log::Info(TEXT("WM_XBUTTON2UP"));
        }

        PostMessage(hwnd, MS_STYLUSBUTTON, 0, 0);
        return 0;
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

    return {};
}

void Inputs::GetInputData(LPARAM &lparam) {

    _control = GetKeyState(VK_CONTROL);
    _shift = GetKeyState(VK_SHIFT);
    _alt = GetKeyState(VK_MENU) < 0;
    _current_packet.x = GET_X_LPARAM(lparam);
    _current_packet.y = GET_Y_LPARAM(lparam);
}
