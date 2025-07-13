/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <adcontrols/datareader.hpp>
#include <adplugin/plugin.hpp>
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace adfs { class sqlite; }
namespace adcontrols { class MassSpectrometer; }
namespace socfpga { namespace dgmod { struct advalue; } }

namespace socfpgainterpreter {

    class DataInterpreter;

    class DataReader : public adcontrols::DataReader {

    public:
        virtual ~DataReader( void );
        DataReader( const char * traceid );

        static std::vector< std::string > traceid_list();

        // <===== adcontrols::DataReader

        bool initialize( std::shared_ptr< adfs::sqlite >, const boost::uuids::uuid& objid, const std::string& objtxt ) override;
        void finalize() override;

        const boost::uuids::uuid& objuuid() const override;
        const std::string& objtext() const override;
        int64_t objrowid() const override;
        const std::string& display_name() const override;

        size_t fcnCount() const override;
        std::shared_ptr< const adcontrols::Chromatogram > TIC( int fcn ) const override;

        const_iterator begin( int fcn ) const override;
        const_iterator end() const override;
        const_iterator findPos( double seconds, int fcn = (-1), bool closest = false, TimeSpec ts = ElapsedTime ) const override;
        size_t size( int fcn ) const override;

        double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const override;

        // =============================> Iterator reference methods
        int64_t next( int64_t rowid ) const override;
        int64_t next( int64_t rowid, int fcn ) const override;
        int64_t pos( int64_t rowid ) const override;
        int64_t elapsed_time( int64_t rowid ) const override;
        double time_since_inject( int64_t rowid ) const override;
        int fcn( int64_t rowid ) const override;
        // <============================
        boost::any getData( int64_t rowid ) const override;
        std::shared_ptr< adcontrols::MassSpectrum > getSpectrum( int64_t rowid ) const override                                   { return nullptr; }
        std::shared_ptr< adcontrols::Chromatogram > getChromatogram( int fcn, double time, double width ) const override;
        std::shared_ptr< adcontrols::MassSpectrum > readSpectrum( const const_iterator& ) const override                          { return nullptr; }
        std::shared_ptr< adcontrols::MassSpectrum > coaddSpectrum( const_iterator&& begin, const_iterator&& end ) const override  { return nullptr; }
        std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer() const override                                         { return nullptr; }
        std::shared_ptr< adcontrols::Chromatogram >  getChromatogram( int idx ) const override;
        adcontrols::DataInterpreter * dataInterpreter() const override;

    private:
        std::weak_ptr< adfs::sqlite > db_;
        std::string objtext_;
        std::string display_name_;
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > tics_;
        std::vector< socfpga::dgmod::advalue > data_;
        boost::uuids::uuid objid_;
        int64_t objrowid_;
        size_t fcnCount_;
        int64_t elapsed_time_origin_;
        std::unique_ptr< DataInterpreter > interpreter_;
        class impl;
        impl * impl_;
    };

}
