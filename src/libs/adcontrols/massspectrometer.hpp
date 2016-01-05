// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "adcontrols_global.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace adcontrols {

    class DataInterpreter;
	class MSProperty;
    class MSCalibrateResult;
	class MSCalibration;
    class ProcessMethod;
    class datafile;
    class ScanLaw;

	class ADCONTROLSSHARED_EXPORT MassSpectrometer {
        MassSpectrometer( const MassSpectrometer& ) = delete;  // noncopyable
        MassSpectrometer& operator = (const MassSpectrometer&) = delete;
    public:
        MassSpectrometer( void );
        MassSpectrometer( adcontrols::datafile * );
        virtual ~MassSpectrometer(void);
        
        virtual const wchar_t * name() const;
        virtual const ScanLaw& getScanLaw() const;
        //virtual const DataInterpreter& getDataInterpreter() const;
		virtual std::shared_ptr<ScanLaw> scanLaw( const adcontrols::MSProperty& ) const;

		virtual void setCalibration( int mode, const adcontrols::MSCalibrateResult& );

        virtual const std::shared_ptr< adcontrols::MSCalibrateResult > getCalibrateResult( size_t idx ) const;
        virtual const adcontrols::MSCalibration * findCalibration( int mode ) const;
        virtual adcontrols::datafile * datafile() const;
        virtual void setDebugTrace( const char * logfile, int level );
        virtual void setProcessMethod( const std::shared_ptr< adcontrols::ProcessMethod > ) { return; }

        // static methods 
        //static std::shared_ptr< MassSpectrometer > create( const wchar_t * dataInterpreterClsid );
        //static std::shared_ptr< MassSpectrometer > create( const char * dataInterpreterClsid );
        //static std::shared_ptr< MassSpectrometer > create( const wchar_t * dataInterpreterClsid, adcontrols::datafile * );

        static const MassSpectrometer* find( const wchar_t * dataInterpreterClsid );
        static const MassSpectrometer* find( const char * dataInterpreterClsid );

        static const MassSpectrometer& get( const wchar_t * dataInterpreterClsid );
		static const MassSpectrometer& get( const char * dataInterpreterClsid );
        static std::vector< std::wstring > get_model_names();

        static void register_default_spectrometers();

    protected:

        const MassSpectrometer * proxy_instance_;
        adcontrols::datafile * datafile_;
#if _MSC_VER
# pragma warning(disable:4251)
#endif
        std::map< int, std::shared_ptr< adcontrols::MSCalibrateResult > > mode_calib_map_;
    };

}

