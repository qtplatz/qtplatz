#include "analyzerdevicedata.h"

using namespace tofcontroller;

bool
AnalyzerDeviceData::copy( TOFInstrument::AnalyzerDeviceData& d, const TOFInstrument::AnalyzerDeviceData& s )
{
    TAO_OutputCDR out;
    out << s;
    TAO_InputCDR in( out );
    in >> d;
    return true;
}
