/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef FSIO_HPP
#define FSIO_HPP

#include <string>
#include <boost/filesystem.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
    class MSCalibrateResult;
	class PeakResult;
	class datafile;
}

namespace adinterface {
    class Method;
}

namespace adfs { class filesystem; class folder; class file; }
namespace portfolio { class Portfolio; }

namespace adutils {
	
	namespace constants {
        const wchar_t * const extension_mscalibration = L".msclb";
        const wchar_t * const folder_mscalibration = L"/MSCalibration";
        const wchar_t * const file_mscalibration_result = L"MSCalibrateResult"; // same as T::dataClass()
        const wchar_t * const file_mscalibration_massspectrum = L"MassSpectrum"; // same as T::dataClass()
    };
    
	class fsio {
    public:
        fsio();

        static bool mount( adfs::filesystem&, const std::wstring& );
        static bool create( adfs::filesystem&, const std::wstring& );

        static bool save( adfs::filesystem&, const adcontrols::MassSpectrum&, const std::wstring& id, const std::wstring& folder_name );
        static bool save( adfs::filesystem&, const adcontrols::MSCalibrateResult&, const std::wstring& id, const std::wstring& filder_name );
        
        static bool load( adfs::filesystem&, adcontrols::MassSpectrum&, const std::wstring& id, const std::wstring& folder );
        static bool load( adfs::filesystem&, adcontrols::MSCalibrateResult&, const std::wstring& id, const std::wstring& filder );
        
        static bool save_mscalibfile( adfs::filesystem&, const adcontrols::MSCalibrateResult& );
        static bool save_mscalibfile( adfs::filesystem&, const adcontrols::MassSpectrum& );
        static bool load_mscalibfile( adfs::filesystem&, adcontrols::MSCalibrateResult& );
        static bool load_mscalibfile( adfs::filesystem&, adcontrols::MassSpectrum& );

        static bool save( adfs::filesystem&, const adinterface::Method&, const std::wstring& id, const std::wstring& folder );
        static bool load( adfs::filesystem&, adinterface::Method&, const std::wstring& id, const std::wstring& folder );

        //-------------------------------
        static bool saveContents( adfs::filesystem&, const std::wstring&, const portfolio::Portfolio&, const adcontrols::datafile& );
    };

}

#endif // FSIO_HPP
