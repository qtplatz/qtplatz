#ifndef MARKER_H
#define MARKER_H

namespace SAGRAPHICSLib {
struct ISADPMarker;
}

namespace adil {
  namespace ui {

    class Marker  {
    public:
      ~Marker();
      Marker( SAGRAPHICSLib::ISADPMarker * pi = 0 );
      Marker( const Marker& );
    private:
        SAGRAPHICSLib::ISADPMarker * pi_;
    };

  }
}

#endif // MARKER_H
