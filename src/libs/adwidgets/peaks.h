#ifndef PEAKS_H
#define PEAKS_H

struct ISADPPeaks;

namespace adil {
  namespace ui {

    class Peaks  {
    public:
		~Peaks();
		Peaks( ISADPPeaks * pi = 0 );
		Peaks( const Peaks& );
    private:
		ISADPPeaks * pi_;
    };

  }
}

#endif // PEAKS_H
