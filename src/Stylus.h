#pragma once

#include <memory>

#define NOMINMAX
#include <Windows.h>
#include <ole2.h>
#include <RTSCOM.h>
#include <RTSCOM_i.c>

#include <glm/vec2.hpp>

class App;
class StylusSync;

struct StylusData {
    float pos_x;
    float pos_y;
    float pressure_normal;
    float pressure_tangent;
    float tilt_x;
    float tilt_y;
};

class Stylus {
  public:
    Stylus(const Stylus &) = delete;
    Stylus(Stylus &&) = delete;
    Stylus &operator=(const Stylus &) = delete;
    Stylus &operator=(Stylus &&) = delete;

    Stylus(App *app);
    ~Stylus();

    std::unique_ptr<StylusSync> _stylus_sync;
    IRealTimeStylus *_stylus;
    App *_app;

  private:

    // glm::vec2 _position;
    // glm::vec2 _velocity;
    // float _pressure;
    // float _tilt;
};


class StylusSync : public IStylusSyncPlugin {
  public:
    StylusSync(App *app, IRealTimeStylus *stylus);
    virtual ~StylusSync();

    // IStylusSyncPlugin methods

    // Handled IStylusSyncPlugin methods, they require nontrivial implementation
    STDMETHOD(StylusDown)
    (IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPropCountPerPkt, LONG *pPacket,
     LONG **ppInOutPkt);
    STDMETHOD(StylusUp)
    (IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPropCountPerPkt, LONG *pPacket,
     LONG **ppInOutPkt);
    STDMETHOD(Packets)
    (IRealTimeStylus *piSrcRtp, const StylusInfo *pStylusInfo, ULONG cPktCount, ULONG cPktBuffLength, LONG *pPackets,
     ULONG *pcInOutPkts, LONG **ppInOutPkts);
    STDMETHOD(DataInterest)(RealTimeStylusDataInterest *pEventInterest);
    STDMETHOD(StylusInRange)(IRealTimeStylus * stylus, TABLET_CONTEXT_ID tablet_id, STYLUS_ID stylus_id);
    STDMETHOD(StylusOutOfRange)(IRealTimeStylus *stylus, TABLET_CONTEXT_ID tablet_id, STYLUS_ID stylus_id);

    // IStylusSyncPlugin methods with trivial inline implementation, they all return S_OK
    STDMETHOD(RealTimeStylusEnabled)(IRealTimeStylus *, ULONG, const TABLET_CONTEXT_ID *) {
        return S_OK;
    }
    STDMETHOD(RealTimeStylusDisabled)(IRealTimeStylus *, ULONG, const TABLET_CONTEXT_ID *) {
        return S_OK;
    }
    STDMETHOD(InAirPackets)(IRealTimeStylus *, const StylusInfo *, ULONG, ULONG, LONG *, ULONG *, LONG **) {
        return S_OK;
    }
    STDMETHOD(StylusButtonUp)(IRealTimeStylus *, STYLUS_ID, const GUID *, POINT *) {
        return S_OK;
    }
    STDMETHOD(StylusButtonDown)(IRealTimeStylus *, STYLUS_ID, const GUID *, POINT *) {
        return S_OK;
    }
    STDMETHOD(SystemEvent)(IRealTimeStylus *, TABLET_CONTEXT_ID, STYLUS_ID, SYSTEM_EVENT, SYSTEM_EVENT_DATA) {
        return S_OK;
    }
    STDMETHOD(TabletAdded)(IRealTimeStylus *, IInkTablet *) {
        return S_OK;
    }
    STDMETHOD(TabletRemoved)(IRealTimeStylus *, LONG) {
        return S_OK;
    }
    STDMETHOD(CustomStylusDataAdded)(IRealTimeStylus *, const GUID *, ULONG, const BYTE *) {
        return S_OK;
    }
    STDMETHOD(Error)(IRealTimeStylus *, IStylusPlugin *, RealTimeStylusDataInterest, HRESULT, LONG_PTR *) {
        return S_OK;
    }
    STDMETHOD(UpdateMapping)(IRealTimeStylus *) {
        return S_OK;
    }

    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

  private:
    App *_app;
    IRealTimeStylus *_stylus;
    LONG m_cRefCount;             // COM object reference count
    IUnknown *m_punkFTMarshaller; // free-threaded marshaller
    int m_nContacts;              // number of fingers currently in the contact with the touch digitizer
};