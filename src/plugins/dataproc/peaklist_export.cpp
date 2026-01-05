/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
``** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "peaklist_export.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include "datafolder.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata_t.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <fstream>
#include <iomanip>

using namespace dataproc;

void
peaklist_export::text_export( const std::filesystem::path& path )
{
    std::ofstream outf( path.string() );

    for ( auto& session : *SessionManager::instance() ) {
        if ( auto processor = session.processor() ) {
            auto spectra = processor->getPortfolio().findFolder( L"Spectra" );

            for ( auto& folium: spectra.folio() ) {
                if ( folium.attribute( L"isChecked" ) == L"true" ) {
                    if ( folium.empty() )
                        processor->fetch( folium );

                    // output filename
                    outf << adportable::utf::to_utf8( processor->filename() ) << std::endl;

                    portfolio::Folio atts = folium.attachments();
                    auto itCentroid = std::find_if( atts.begin(), atts.end(), []( portfolio::Folium& f ) {
                            return f.name() == Constants::F_CENTROID_SPECTRUM;
                        });

                    if ( itCentroid != atts.end() ) {

                        // output spectrum(centroid) name
                        outf << adportable::utf::to_utf8( folium.name() + L",\t" + itCentroid->name() ) << std::endl;

                        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {
                            adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *centroid );
                            int fcn = 0;
                            for ( auto& ms: segments ) {
                                const adcontrols::annotations& annots = ms.annotations();
                                for ( size_t n = 0; n < ms.size(); ++n ) {
                                    outf << fcn << ",\t" << n << ",\t"
                                         << std::scientific << std::setprecision( 15 ) << ms.time( n ) << ",\t"
                                         << std::fixed << std::setprecision( 13 ) << ms.mass( n ) << ",\t"
                                         << std::scientific << std::setprecision(7) << ms.intensity( n );

                                    auto it = std::find_if( annots.begin(), annots.end()
                                                            , [=]( const adcontrols::annotation& a ){ return a.index() == int(n); } );
                                    while ( it != annots.end() ) {
                                        outf << ",\t" << it->text();
                                        it = std::find_if( ++it, annots.end()
                                                           , [=]( const adcontrols::annotation& a ){ return a.index() == int(n); } );
                                    }
                                    outf << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

namespace {

    struct peaklist_writer {
        std::shared_ptr< adfs::sqlite > db_;
        const std::wstring filename_;
        int64_t fileid_;

        peaklist_writer( std::shared_ptr< adfs::sqlite > db, const std::wstring& filename ) : db_( db ), filename_( filename ), fileid_(-1) {
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT OR IGNORE INTO dataSource (filename) VALUES (?)" );
            sql.bind( 1 ) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                fileid_ = db_->last_insert_rowid();
            sql.commit();
        }

        static void create_table( std::shared_ptr< adfs::sqlite > db ) {
            adfs::stmt sql( *db );
            sql.exec( "CREATE TABLE dataSource ("
                      "id INTEGER PRIMARY KEY"
                      ",filename TEXT"
                      ",UNIQUE( filename )"
                      ")" );

            sql.exec( "CREATE TABLE spectrum ("
                      "id INTEGER PRIMARY KEY"
                      ",fileid INTEGER"
                      ",spname TEXT"
                      ",sptype TEXT"
                      ",UNIQUE( spname, id )"
                      ",FOREIGN KEY ( fileid ) REFERENCES dataSource ( id )"
                      ")" );

            sql.exec( "CREATE TABLE chromatogram ("
                      "id INTEGER PRIMARY KEY"
                      ",fileid  INTEGER"
                      ",name    TEXT"
                      ",injTime REAL"
                      ",proto   INTEGER"
                      ",time_of_inject  TEXT"
                      ",FOREIGN KEY ( fileid ) REFERENCES dataSource ( id )"
                      ")" );

            sql.exec( "CREATE TABLE speak ("
                      "spid     INTEGER"
                      ",proto   INTEGER"
                      ",mode    INTEGER"
                      ",time    REAL"
                      ",intensity REAL"
                      ",mass    REAL"
                      ",height  REAL"
                      ",width_m REAL"
                      ",width_t REAL"
                      ",formula TEXT"
                      ",FOREIGN KEY ( spid ) REFERENCES spectrum ( id )"
                      ")" );

            sql.exec( "CREATE TABLE cpeak ("
                      "chroid   INTEGER"
                      ",name    TEXT"
                      ",tR      REAL"
                      ",area    REAL"
                      ",height  REAL"
                      ",width   REAL"
                      ",ntp     REAL"
                      ",Rs      REAL"
                      ",k       REAL"
                      ",Tf      REAL"
                      ",FOREIGN KEY ( chroid ) REFERENCES chromatogram ( id )"
                      ")" );
        }

        std::shared_ptr< adcontrols::MassSpectrum > findCentroid( const portfolio::Folium& folium ) {
            auto atts = folium.attachments();
            auto it = std::find_if( atts.begin()
                                    , atts.end()
                                    , []( portfolio::Folium& f ) { return f.name() == Constants::F_CENTROID_SPECTRUM; });
            if ( it != atts.end() )
                return portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( *it );
            return {};
        }

        void write( adcontrols::MSPeakInfo& t, const portfolio::Folium& f ) {
            int64_t spid(0);
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT INTO spectrum ( fileid, spname, sptype ) SELECT id,?,? FROM dataSource WHERE filename = ?" );
            sql.bind(1) = f.name();
            sql.bind(2) = std::string("");
            sql.bind(3) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                spid = db_->last_insert_rowid();
            else
                ADDEBUG() << sql.errmsg();
            sql.commit();

            //                    proto, index
            std::map< std::pair< size_t, size_t >, std::string > atext;

            // override formua in the peakInfo
            if ( auto ms = findCentroid( f ) ) {
                int proto(0);
                for ( const auto& msp: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >(*ms) ) {
                    if ( auto info = t.findProtocol( proto ) ) {

                        auto annots = msp.annotations();
                        for ( auto anno: msp.annotations() ) {

                            if ( anno.dataFormat() == adcontrols::annotation::dataText ||
                                 anno.dataFormat() == adcontrols::annotation::dataFormula ) {
                                if ( atext.find( std::make_pair( proto, anno.index() ) ) == atext.end() )
                                    atext[ std::make_pair( proto, anno.index() ) ] = anno.text();
                            }
                            if ( anno.dataFormat() == adcontrols::annotation::dataMOL ) {
                                // target.id candidate
                                auto mol = boost::json::value_to< adcontrols::annotation::reference_molecule >( anno.value() );
                                atext[ std::make_pair( proto, anno.index() ) ] = mol.display_text_;
                            }
                        }
                        ++proto;
                    }
                }
            }


            sql.prepare( "INSERT INTO speak(spid,proto,mode,time,intensity,mass,height,width_m,width_t,formula ) VALUES (?,?,?,?,?,?,?,?,?,?)" );
            uint32_t proto(0);
            for ( const auto& info: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( t ) ){
                size_t index(0);
                for ( const auto& pk: info ) {
                    sql.reset();
                    uint32_t id(1);
                    sql.bind(id++) = spid;
                    sql.bind(id++) = proto;
                    sql.bind(id++) = info.mode();
                    sql.bind(id++) = pk.time();
                    sql.bind(id++) = pk.area();
                    sql.bind(id++) = pk.mass();
                    sql.bind(id++) = pk.height();
                    sql.bind(id++) = pk.widthHH();
                    sql.bind(id++) = pk.widthHH( true );
                    auto it = atext.find( std::make_pair( proto, index ) );
                    if ( it != atext.end() ) {
                        sql.bind( id++ ) = it->second;
                    } else {
                        sql.bind( id++ ) = adfs::null{};
                    }
                    if ( sql.step() != adfs::sqlite_done )
                        ADDEBUG() << sql.errmsg() << "\n" << sql.expanded_sql();
                    ++index;
                }
                ++proto;
            }
        }

        void write( const adcontrols::MassSpectrum& t, const portfolio::Folium& f ) {
            int64_t spid(0);
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT INTO spectrum ( fileid, spname, sptype ) SELECT id,?,? FROM dataSource WHERE filename = ?" );
            sql.bind(1) = f.name();
            sql.bind(2) = std::string("");
            sql.bind(3) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                spid = db_->last_insert_rowid();
            else
                ADDEBUG() << sql.errmsg();
            sql.commit();

            // ADDEBUG() << "-------- exporting MassSpectrum spid=" << spid << ", name: " << f.name();

            sql.prepare( "INSERT INTO speak(spid,proto,mode,time,intensity,mass,formula ) VALUES (?,?,?,?,?,?,?)" );

            uint32_t proto(0);
            for ( const auto& ms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( t ) ){
                auto a = ms.annotations();
                for ( size_t i = 0; i < ms.size(); ++i ) {
                    auto it = std::find_if( a.begin(), a.end()
                                            , [&]( const auto& x ){
                                                  return x.dataFormat() == adcontrols::annotation::dataFormula && x.index() == int(i);
                                              });
                    auto formula = ( it != a.end() ) ? it->text() : std::string();
                    sql.reset();
                    uint32_t id(1);
                    sql.bind(id++) = spid;
                    sql.bind(id++) = proto;
                    sql.bind(id++) = ms.mode();
                    sql.bind(id++) = ms.time( i );
                    sql.bind(id++) = ms.intensity( i );
                    sql.bind(id++) = ms.mass( i );
                    sql.bind(id++) = formula;
                    if ( sql.step() != adfs::sqlite_done )
                        ADDEBUG() << sql.errmsg();
                }
                ++proto;
            }
        }

        void __write( const adcontrols::Peaks& peaks, int64_t chroid ) {
            adfs::stmt sql( *db_ );
            if ( peaks.size() == 0 ) { // insert null peak
                sql.prepare( "INSERT INTO cpeak(chroid,name,tR,area,height) VALUES (?,?,?,?,?)" );
                sql.bind(1) = chroid;
                sql.bind(2) = std::string("n/a");
                sql.bind(3) = -1.;
                sql.bind(4) = 0.;
                sql.bind(5) = 0.;
                if ( sql.step() != adfs::sqlite_done )
                    ADDEBUG() << sql.errmsg();
            } else {
                sql.prepare( "INSERT INTO cpeak(chroid,name,tR,area,height,width,ntp,Rs,k,Tf ) VALUES (?,?,?,?,?,?,?,?,?,?)" );
                for ( auto pk: peaks ){
                    sql.reset();
                    int id(1);
                    sql.bind(id++) = chroid;
                    sql.bind(id++) = std::string( pk.name() );
                    sql.bind(id++) = static_cast< double >( pk.peakTime() );
                    sql.bind(id++) = pk.peakArea();
                    sql.bind(id++) = pk.peakHeight();
                    sql.bind(id++) = pk.peakWidth();
                    sql.bind(id++) = pk.theoreticalPlate().ntp();
                    sql.bind(id++) = pk.resolution().resolution();
                    sql.bind(id++) = pk.capacityFactor();
                    sql.bind(id++) = pk.asymmetry().asymmetry();
                    if ( sql.step() != adfs::sqlite_done )
                        ADDEBUG() << sql.errmsg();
                }
            }
        }

        void write( const adcontrols::Chromatogram& t, const portfolio::Folium& f ) {
            int64_t chroid(0);
            adfs::stmt sql( *db_ );
            sql.begin();
            sql.prepare( "INSERT INTO chromatogram (fileid,name,injTime,proto,time_of_inject) SELECT id,?,?,?,? FROM dataSource WHERE filename = ?" );
            sql.bind(1) = f.name();
            sql.bind(2) = double(std::chrono::duration_cast< std::chrono::milliseconds >( t.time_of_injection().time_since_epoch() ).count()) / 1000.0;
            sql.bind(3) = t.protocol();
            sql.bind(4) = t.time_of_injection_iso8601();
            sql.bind(5) = filename_;
            if ( sql.step() == adfs::sqlite_done )
                chroid = db_->last_insert_rowid();
            else
                ADDEBUG() << sql.errmsg();
            sql.commit();

            ADDEBUG() << "-------- exporting Chromatogram chroid=" << chroid << ", name: " << f.name();

            using dataTuple = std::tuple< std::shared_ptr< adcontrols::PeakResult > >;
            for ( auto& a: f.attachments() ) {
                if ( auto var = adutils::to_variant< dataTuple >()(static_cast< const boost::any& >( a ) ) ) {
                    if ( auto pkresult = boost::get< std::shared_ptr< adcontrols::PeakResult > >( *var ) )  {
                        __write( pkresult->peaks(), chroid );
                        return;
                    }
                }
            }
            // if not handled in within above loop
            __write( t.peaks(), chroid );
        }

        template< typename T >
        bool write( portfolio::Folium& f, portfolio::Folium& folium ) {
            if ( auto data = portfolio::get< std::shared_ptr< T > >( f ) ) {
                write( *data, folium );
                return true;
            }
            return false;
        }
    };
}


void
peaklist_export::sqlite_export( const std::filesystem::path& path )
{
    auto db = std::make_shared< adfs::sqlite >();
    if ( db->open( path.string().c_str(), adfs::flags::opencreate ) ) {
        adfs::stmt sql( *db );
        sql.exec( "PRAGMA synchronous = OFF" );
        sql.exec( "PRAGMA journal_mode = MEMORY" );
        sql.exec( "PRAGMA FOREIGN_KEYS = ON" );
        peaklist_writer::create_table( db );

        ADDEBUG() << "-------- sqlite_export -------";

        for ( auto& session : *SessionManager::instance() ) {
            if ( auto processor = session.processor() ) {

                peaklist_writer writer( db, processor->filename() );

                auto sfolio = processor->getPortfolio().findFolder( L"Spectra" );

                for ( auto& folium: sfolio.folio() ) {
                    if ( folium.attribute( L"isChecked" ) == L"true" ) {
                        if ( folium.empty() )
                            processor->fetch( folium );
                        portfolio::Folio atts = folium.attachments();
                        auto it = std::find_if( atts.begin(), atts.end()
                                                , []( portfolio::Folium& f ) { return f.name() == Constants::F_CENTROID_SPECTRUM; });
                        if ( it != atts.end() ) {
                            portfolio::Folio a2 = it->attachments();
                            auto it2 = std::find_if( a2.begin(), a2.end()
                                                     , []( portfolio::Folium& f ) { return f.name() == Constants::F_MSPEAK_INFO; });
                            auto success = ( it2 != a2.end() ) && writer.write<adcontrols::MSPeakInfo>( *it2, folium );
                            if ( ! success )
                                writer.write< adcontrols::MassSpectrum >( *it, folium );
                        }
                    }
                }


                using dataTuple = std::tuple< std::shared_ptr< adcontrols::PeakResult > >; //, std::shared_ptr< adcontrols::Chromatogram > >;

                bool success( false );
                auto cfolio = processor->getPortfolio().findFolder( L"Chromatograms" );
                for ( auto& folium: cfolio.folio() ) {
                    if ( folium.attribute( L"isChecked" ) == L"true" ) {
                        if ( folium.empty() ) {
                            processor->fetch( folium );
                        }
                        writer.write< adcontrols::Chromatogram >( folium, folium );
                    }
                }
            }
        }
    }
}
