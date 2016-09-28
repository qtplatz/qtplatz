/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappeddataframe.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <boost/any.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <QApplication>

namespace adprocessor { class dataprocessor; }


namespace counting2d {
    // See malpix/malpix/mpxcontrols/constants.hpp    
    // malpix_observer = name_generator( "{6AE63365-1A4D-4504-B0CD-38AE86309F83}" )( "1.image.malpix.ms-cheminfo.com" )

    static const boost::uuids::uuid malpix_observer = boost::uuids::string_generator()( "{62ede8f7-dfa3-54c3-a034-e012173e2d10}" );
}

using namespace counting2d;

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

document::document() : QObject( 0 )
{
}

bool
document::setDataprocessor( std::shared_ptr< adprocessor::dataprocessor > dp )
{
    if ( auto rawfile = dp->rawdata() ) {
        
        if ( rawfile->dataformat_version() >= 3 ) {

            // is this contains MALPIX image data?
            adfs::stmt sql( *dp->db() );
            sql.prepare( "SELECT COUNT(*) FROM AcquiredConf WHERE objuuid = ?" );
            sql.bind( 1 ) = malpix_observer;
            while ( sql.step() == adfs::sqlite_row ) {
                // has MALPIX raw image (dataFrame) for each trigger
                if ( auto malpixReader = rawfile->dataReader( malpix_observer ) ) {
                    processor_ = dp;
                    //malpixReader->TIC(0);
                    return true;
                }
            }
        }
    }
    return false;
}

bool
document::fetch()
{
    if ( auto dp = processor_ ) {
        if ( auto reader = dp->rawdata()->dataReader( malpix_observer ) ) {
	  
            for ( auto it = reader->begin(); it != reader->end(); ++it ) {
                boost::any a = reader->getData( it->rowid() );
                if ( auto ptr = boost::any_cast< std::shared_ptr< adcontrols::MappedDataFrame > >( a ) ) {
                    if ( ! ptr->empty() ) {
                        mappedDataFrame_.emplace_back( ptr );
                        emit dataChanged();
                        QApplication::processEvents();
                    }
                }
#if 0
                if ( auto map = reader->getMappedSpectra( it->rowid() ) ) {
                    if ( !map->empty() ) {
                        mappedSpectra_.emplace_back( map );
                        emit dataChanged();
                        QApplication::processEvents();
                    }
                }
#endif
            }
        }
    }
}
