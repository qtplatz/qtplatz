/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANDATAWRITER_HPP
#define QUANDATAWRITER_HPP

#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <string>
#include <mutex>
#include <functional>
#include <tuple>

namespace boost { namespace uuids { struct uuid; } }
namespace adcontrols {
    class Chromatogram; class MassSpectrum; class PeakResult;
    class ProcessMethod; class QuanSequence; class QuanSample; class QuanCompounds;
    class QuanMethod; class idAudit;
}

namespace adprocessor { class dataprocessor; }

namespace adfs { class stmt; }

namespace quan {

    class QuanChromatograms;

    class QuanDataWriter  {
        QuanDataWriter( const QuanDataWriter& ) = delete;
    public:
        ~QuanDataWriter();
        QuanDataWriter( const std::wstring& path );

        bool open();
        void remove( const std::wstring& title, const wchar_t * directory );
        adfs::file write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle );
        adfs::file write( const adcontrols::ProcessMethod& );
        adfs::file write( const adcontrols::QuanSequence& );
        adfs::file write( const adcontrols::QuanSample& );
        adfs::file write( const adcontrols::Chromatogram&, const std::wstring& title );
        adfs::file write( const adcontrols::Chromatogram&, const wchar_t * dataSource, const std::wstring& title );

        // bool write( std::shared_ptr< QuanChromatograms > chro, const std::wstring& title, std::vector< std::wstring >& );

        bool drop_table();
        bool create_table();
        bool create_counting_tables();

        bool insert_table( const adcontrols::QuanMethod& );
        bool insert_table( const adcontrols::QuanSequence& );
        bool insert_table( const adcontrols::QuanCompounds& );
        bool insert_table( const adcontrols::QuanSample& );
        [[deprecated]] bool insert_table( const std::wstring& dataGuid, const std::vector< std::tuple<std::wstring, uint32_t, uint32_t> >& dataGuids );
        bool insert_reference( const boost::uuids::uuid& dataGuid, const boost::uuids::uuid& refGuid, int32_t idx, int32_t proto );

        bool addCountingResponse( const boost::uuids::uuid& dataGuid // chromatogram file id
                                  , const adcontrols::QuanSample& sample
                                  , const adcontrols::Chromatogram& );

        bool addMSLock( const adcontrols::QuanSample& sample
                        , const std::vector< std::pair< double, std::vector< double > > >& lkms ); // time, coeffs

        bool addMSLock( std::shared_ptr< adprocessor::dataprocessor> dp
                        , const std::vector< std::pair< double, std::vector< double > > >& lkms ); // time, coeffs

        static bool insert_table( adfs::stmt&, const adcontrols::idAudit&, const std::string& what );

        template< class T> adfs::file attach( adfs::file& file, const T& t, const std::wstring& name ) {
            auto afile = file.addAttachment( adfs::create_uuid() );
            afile.dataClass( T::dataClass() );
            if ( !name.empty() )
                afile.setAttribute( L"name", name );
            if ( afile.save( t ) )
                afile.commit();
            return afile;
        }

    private:
        std::wstring path_;
        adfs::filesystem fs_;
        std::string uuidQuanCompound_;
    };

}

#endif // QUANDATAWRITER_HPP
