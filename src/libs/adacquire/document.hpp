/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once
#include "adacquire_global.hpp"
#include <deque>
#include <mutex>
#include <memory>
#include <vector>

namespace acqrscontrols { namespace u5303a { class method; class waveform; class threshold_result; class tdcdoc; } }
namespace adcontrols {
    namespace ControlMethod { class Method; }
    struct seconds_t;
    class MappedImage;
    class MassSpectrum;
    class MassSpectrometer;
    class MSMolTable;
    class TraceAccessor;
    class Trace;
    class SampleRun;
    class threshold_method;
    class TofChromatogramsMethod;
}
namespace adextension { class iController; class iSequenceImpl; }
namespace adfs { class sqlite; }
namespace adacquire { class SampleProcessor; }
namespace boost { namespace uuids { struct uuid; } namespace filesystem { class path; } }

namespace adacquire {

    class ADACQUIRESHARED_EXPORT document {
    public:
        document();
        ~document();
        
        void initialSetup();
        void finalClose();

        bool initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& ) const;
        bool prepareStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const;
        bool closingStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const;

        void setScanLawLookupTable( std::shared_ptr< const adcontrols::MSMolTable > );
        std::shared_ptr< const adcontrols::MSMolTable > scanLawLookupTable() const;
        QString scanLawHistoryFile() const;

    private slots:
        
    private:
        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
        std::shared_ptr< const adcontrols::MSMolTable > molTable_;
        
    signals:

    };

}

