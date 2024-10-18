#include "Stylus.h"
#include "App.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <spdlog/spdlog.h>

IRealTimeStylus *CreateStylus(HWND hwnd) {
    HRESULT result;
    IRealTimeStylus *stylus;
    result = CoCreateInstance(CLSID_RealTimeStylus, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&stylus));
    result = stylus->put_HWND((HANDLE_PTR)hwnd);
    result = stylus->put_Enabled(TRUE);

    return stylus;
}

Stylus::Stylus(App *app) : _app(app) {
    const auto hwnd = glfwGetWin32Window(_app->_window);
    _stylus = CreateStylus(hwnd);

    // https://learn.microsoft.com/en-us/windows/win32/tablet/packetpropertyguids-constants
    GUID guidDesiredPacketDescription[] = {GUID_PACKETPROPERTY_GUID_X,
                                           GUID_PACKETPROPERTY_GUID_Y,
                                           GUID_PACKETPROPERTY_GUID_PACKET_STATUS,
                                           GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE,
                                           GUID_PACKETPROPERTY_GUID_TANGENT_PRESSURE,
                                           GUID_PACKETPROPERTY_GUID_X_TILT_ORIENTATION,
                                           GUID_PACKETPROPERTY_GUID_Y_TILT_ORIENTATION,
                                           GUID_PACKETPROPERTY_GUID_TIMER_TICK};

    ULONG ulProperties = sizeof(guidDesiredPacketDescription) / sizeof(GUID);
    _stylus->SetDesiredPacketDescription(ulProperties, guidDesiredPacketDescription);

    _stylus_sync = std::make_unique<StylusSync>(_app, _stylus);
}

Stylus::~Stylus() {
    _stylus->Release();
}

StylusSync::StylusSync(App *app, IRealTimeStylus *stylus)
    : _app(app), _stylus(stylus), m_cRefCount(1), m_punkFTMarshaller(nullptr), m_nContacts(0) {
    HRESULT result;
    result = CoCreateFreeThreadedMarshaler(this, &m_punkFTMarshaller);
    result = stylus->AddStylusSyncPlugin(0, this);
}

StylusSync::~StylusSync() {
    if (m_punkFTMarshaller) {
        m_punkFTMarshaller->Release();
    }
}

STDMETHODIMP_(HRESULT __stdcall)
StylusSync::StylusDown(IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPropCountPerPkt, LONG *pPacket,
                       LONG **ppInOutPkt) {
    spdlog::info("Stylus down");
    ++m_nContacts;

    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
StylusSync::StylusUp(IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPropCountPerPkt, LONG *pPacket,
                     LONG **ppInOutPkt) {
    spdlog::info("Stylus up");
    --m_nContacts;

    return S_OK;
}
// Pen-move notification.
// In this case, does nothing, but likely to be used in a more complex application.
// RTS framework does stroke collection and rendering for us.
// in:
//      piRtsRtp            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPktCount           number of packets
//      cPktBuffLength      pPacket buffer size, in elements, equal to number of packets times number of properties per
//      packet pPackets            packet data (layout depends on packet description set)
// in/out:
//      pcInOutPkts         modified number of packets
//      ppInOutPkts         modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
STDMETHODIMP_(HRESULT __stdcall)
StylusSync::Packets(IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPktCount, ULONG cPktBuffLength,
                    LONG *pPackets, ULONG *pcInOutPkts, LONG **ppInOutPkts) {
    spdlog::info("Received Packed: count {}", cPktCount);

    FLOAT pfInkToDeviceScaleX{};
    FLOAT pfInkToDeviceScaleY{};
    ULONG cPacketProperties{};
    PACKET_PROPERTY *pPacketProperties;


    piSrcRtp->GetPacketDescriptionData(pStylusInfo->tcid, &pfInkToDeviceScaleX, &pfInkToDeviceScaleY,
                                       &cPacketProperties, &pPacketProperties);
    StylusData data{};
    for (size_t i = 0; i < cPacketProperties; i++) {
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_X) {
            spdlog::warn("{}, GUID_PACKETPROPERTY_GUID_X");
            data.pos_x = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_Y) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_Y");
            data.pos_y = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_PACKET_STATUS) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_PACKET_STATUS");
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE");
            data.pressure_normal = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_TANGENT_PRESSURE) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_TANGENT_PRESSURE");
            data.pressure_tangent = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_X_TILT_ORIENTATION) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_X_TILT_ORIENTATION");
            data.tilt_x = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
        if (pPacketProperties[i].guid == GUID_PACKETPROPERTY_GUID_Y_TILT_ORIENTATION) {
            spdlog::warn("GUID_PACKETPROPERTY_GUID_Y_TILT_ORIENTATION");
            data.tilt_y = pPackets[i] / (float)pPacketProperties[i].PropertyMetrics.nLogicalMax;
        }
    }

    CoTaskMemFree(pPacketProperties);
    
    spdlog::warn("Stylus data:\n\tposition: ({};{})\n\tpressure: {}", data.pos_x, data.pos_y, data.pressure_normal);
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) StylusSync::DataInterest(RealTimeStylusDataInterest *pEventInterest) {
    *pEventInterest = (RealTimeStylusDataInterest)(RTSDI_StylusDown | RTSDI_Packets | RTSDI_StylusUp |
                                                   RTSDI_StylusInRange | RTSDI_StylusOutOfRange);

    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
StylusSync::StylusInRange(IRealTimeStylus *stylus, TABLET_CONTEXT_ID tablet_id, STYLUS_ID stylus_id) {
    spdlog::info("StylusInRange TABLET_CONTEXT_ID: {}, STYLUS_ID: {}", tablet_id, stylus_id);
    return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall)
StylusSync::StylusOutOfRange(IRealTimeStylus *stylus, TABLET_CONTEXT_ID tablet_id, STYLUS_ID stylus_id) {
    spdlog::info("StylusOutOfRange TABLET_CONTEXT_ID: {}, STYLUS_ID: {}", tablet_id, stylus_id);
    return E_NOTIMPL;
}

STDMETHODIMP_(ULONG __stdcall) StylusSync::AddRef() {
    return InterlockedIncrement(&m_cRefCount);
}

STDMETHODIMP_(ULONG __stdcall) StylusSync::Release() {
    ULONG cNewRefCount = InterlockedDecrement(&m_cRefCount);
    if (cNewRefCount == 0) {
        delete this;
    }
    return cNewRefCount;
}

STDMETHODIMP_(HRESULT __stdcall) StylusSync::QueryInterface(REFIID riid, LPVOID *ppvObj) {
    {
        if ((riid == IID_IStylusSyncPlugin) || (riid == IID_IUnknown)) {
            *ppvObj = this;
            AddRef();
            return S_OK;
        } else if ((riid == IID_IMarshal) && (m_punkFTMarshaller != NULL)) {
            return m_punkFTMarshaller->QueryInterface(riid, ppvObj);
        }

        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}
