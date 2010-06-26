#ifndef PEAKS_H
#define PEAKS_H

namespace SAGRAPHICSLib {
struct ISADPPeaks;
}

namespace adil {
  namespace ui {

    class Peaks  {
    public:
		~Peaks();
		Peaks( SAGRAPHICSLib::ISADPPeaks * pi = 0 );
		Peaks( const Peaks& );
    private:
        SAGRAPHICSLib::ISADPPeaks * pi_;
    };

  }
}

#endif // PEAKS_H
