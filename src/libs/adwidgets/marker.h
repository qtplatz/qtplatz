#ifndef MARKER_H
#define MARKER_H

struct ISADPMarker;

namespace adil {
  namespace ui {

    class Marker  {
    public:
      ~Marker();
      Marker( ISADPMarker * pi = 0 );
      Marker( const Marker& );
    private:
      ISADPMarker * pi_;
    };

  }
}

#endif // MARKER_H
