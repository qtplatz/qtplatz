/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <boost/variant.hpp>

namespace adcontrols {

    struct null_t {};

    // type list decl
    template < typename ... T >  struct type_list_t {};

    ///////////////// return value ////////////////
    typedef boost::variant <  std::shared_ptr< MassSpectrum >
                              , std::shared_ptr< Chromatogram >
                              , std::shared_ptr< PeakResult >
                              , std::shared_ptr< ProcessMethod >
                              , std::shared_ptr< MSCalibrateResult >
                              , std::shared_ptr< MSPeakInfo >
                              , std::shared_ptr< MassSpectra >
                              , std::shared_ptr< Targeting >
                              , std::shared_ptr< QuanSample >
                              , std::shared_ptr< QuanSequence >
                              > data_class_t;

    /////////////////// type list of dataClass'es //////////////
    typedef type_list_t<  MassSpectrum
                          , Chromatogram
                          , PeakResult
                          , ProcessMethod
                          , MSCalibrateResult
                          , MSPeakInfo
                          , MassSpectra
                          , Targeting
                          , QuanSample
                          , QuanSequence
                          , null_t
                          > data_class_tlist;

    // final
    template< typename last_t > struct type_list_t< last_t > {
        data_class_t operator()( const std::wstring& dataClass ) const { return {}; }
        data_class_t operator()( const std::wstring& dataClass, const char * data, size_t size ) const { return {}; }
    };

    //
    template< typename first_t, typename ... args >
    struct type_list_t< first_t, args ... > {
        // factory
        data_class_t operator()( const std::wstring& dataClass ) const {
            if ( dataClass == first_t::dataClass() )
                return std::make_shared< first_t >();
            return type_list_t< args ... >()( dataClass );
        }
    };

    // -- deserialization -- see py_module; file.cpp

} // adcontrols
