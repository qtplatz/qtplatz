/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace acqrsinterpreter {

    class DataReader_index;

    class DataReader : public adcontrols::DataReader {
        //DataReader( const DataReader& ) = delete;  // noncopyable
        //DataReader& operator = (const DataReader&) = delete;
    public:
        ~DataReader( void );
        DataReader( const char * traceid );
        
        static std::vector< std::string > traceid_list();        

        // <===== adcontrols::DataReader 

        bool initialize( adfs::filesystem&, const boost::uuids::uuid& objid, const std::string& objtxt ) override;
        void finalize() override;

        const boost::uuids::uuid& objuuid() const override;
        const std::string& objtext() const override;
        int64_t objrowid() const override;

        size_t fcnCount() const override;
        std::shared_ptr< const adcontrols::Chromatogram > TIC( int fcn ) const override;

        const_iterator begin() const override;
        const_iterator end() const override;
        const_iterator findPos( double seconds, bool closest = false, TimeSpec ts = ElapsedTime ) const override;
        
        double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const override;

        // =============================> Iterator reference methods
        int64_t next( int64_t rowid ) const override;
        int64_t pos( int64_t rowid ) const override;
        int64_t elapsed_time( int64_t rowid ) const override;
        double time_since_inject( int64_t rowid ) const override;
        int fcn( int64_t rowid ) const override;
        // <============================
        std::shared_ptr< const adcontrols::MassSpectrum > getSpectrum( int64_t rowid ) const override;

    private:
        friend class DataReader_index;
        void loadTICs();
        std::unique_ptr< adcontrols::DataInterpreter > interpreter_;
        std::weak_ptr< adfs::sqlite > db_;
        boost::uuids::uuid objid_;
        std::string objtext_;
        int64_t objrowid_;
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > tics_;

        struct index {
            int64_t rowid; int64_t pos; int64_t elapsed_time; int fcn;
            index( int64_t _0 = 0, int64_t _1 = 0, int64_t _2 = 0, int _3 = 0 ) : rowid( _0 ), pos( _1 ), elapsed_time( _2 ), fcn( _3 ) {}
        };

        std::vector< index > indecies_;
    };

}


