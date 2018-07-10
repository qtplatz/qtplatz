/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>

namespace adinterface {

    enum eDeviceCategory {
        device_unknown
        , device_sampler               // autosampler or anything prepare sample (trigger injection)
        , device_gas_chromatograph     // just GC w/ or w/o injector
        , device_liquid_chromatograph  // degasser, pump, gradienter, oven (sync to an injection)
        , device_detector              // LC/GC detectors including MS
    };
    
    class InstInfo {
    public:
        unsigned long index;
        unsigned long unit_number;
        eDeviceCategory category;
        std::wstring modelname;
        std::wstring serial_number;
        std::wstring description;
        InstInfo() : index(0), unit_number(0), category( device_unknown ) {
        }
        InstInfo( const InstInfo& t ) : index(t.index)
                                      , unit_number(t.unit_number)
                                      , category( t.category  )
                                      , modelname( t.modelname )
                                      , serial_number( t.serial_number )
                                      , description( t.description ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >  
        void serialize( Archive& ar, const unsigned int ) {
            ar & index
                & unit_number
                & category
                & modelname
                & serial_number
                & description;
        }
    };
    
    class TimeValue {
    public:
        unsigned long sec_;
        unsigned long usec_;
        TimeValue( unsigned long sec = 0, unsigned long usec = 0 ) : sec_( sec ), usec_( usec ) {
        }
        TimeValue( const TimeValue& t ) : sec_( t.sec_ ), usec_( t.usec_ ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >  
        void serialize( Archive& ar, const unsigned int ) {
            ar & sec_ & usec_;
        }
    };

    // module MethodFunc {
    //     const unsigned long SampleDelay   =    1;  // autosampler sample sequence time for overwrap 
    //     const unsigned long Inject        =    2;  // specify a time to inject
    //     const unsigned long AcqDelay      =    3;  // acquisition delay time 
    //     const unsigned long AcqStop       =    4;  // acquisition stop time
    //     const unsigned long User          = 1024;  // instrument dependent method after this number
    // };
    
    class Method {
    public:
        static const wchar_t * dataClass() { return L"adinterface::Method"; }

        class Line {
        public:
            std::wstring modelname;
            unsigned long index;         // index to instinfo
            unsigned long unitnumber;
            bool isInitialCondition;
            TimeValue time;
            unsigned long funcid;        // MethodFunc
            std::string xdata;   // This holds binary array (not the nil-terminated string)
            Line() : index(0), unitnumber(0), isInitialCondition( true ) {
            }
            Line( const std::wstring& _model
                        , unsigned long _unitnumber = 0
                        , bool isInitial = true
                        , unsigned long _funcid = 0) : modelname( _model )
                                                     , index( 0 )
                                                     , unitnumber( _unitnumber )
                                                     , isInitialCondition( isInitial )
                                                     , funcid( _funcid ) {
            }
            Line( const Line& t ) : modelname( t.modelname )
                , index( t.index )
                , unitnumber( t.unitnumber )
                , isInitialCondition( t.isInitialCondition )
                , time( t.time )
                , funcid( t.funcid )
                , xdata( t.xdata ) {
            }
        private:
            friend class boost::serialization::access;
            template< class Archive >  
            void serialize( Archive& ar, const unsigned int ) {
                ar & modelname
                    & index
                    & unitnumber
                    & isInitialCondition
                    & time
                    & funcid
                    & xdata;
            }
        };
    public:
        std::wstring subject;
        std::wstring description;
        std::vector< InstInfo > iinfo;
        std::vector< Line > lines;
        Method() {
        }
        Method( const Method& t ) : subject( t.subject )
                                  , description( t.description )
                                  , iinfo( t.iinfo )
                                  , lines( t.lines ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >  
        void serialize( Archive& ar, const unsigned int ) {
            ar & subject
                & description
                & iinfo
                & lines;
        }
    };

}
