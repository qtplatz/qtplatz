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

#include "quanpublisher.hpp"
#include "quanconnection.hpp"
#include "quandocument.hpp"
#include "quanplotdata.hpp"
#include "quanprogress.hPP"
#include "quansvgplot.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/idaudit.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quansequence.hpp>
#include <adfs/sqlite3.h>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <QCoreApplication>

namespace quan {
    namespace detail {

        struct append_class {

            template< class T > pugi::xml_node operator()( pugi::xml_node& node, const T& data ) const {
                auto child = node.append_child( "classdata" );
                child.append_attribute( "decltype" ) = typeid(data).name();
                pugi::xmlhelper helper( data );
                child.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
                return child;
            }
        };

        struct append_process_method : public boost::static_visitor<bool> {
            pugi::xml_node& node;
            append_process_method( pugi::xml_node& n ) : node( n ){}
            template<class T> bool operator()( const T& data ) const {
                append_class()( node, data );
                return true;
            }
        };
        
        struct append_column {
            pugi::xml_node& row;
            append_column( pugi::xml_node& n ) : row( n ) {}

            template<typename T> pugi::xml_node operator()( const char * typnam, const char * name, const T& value ) const {
                auto node = row.append_child( "column" );
                node.append_attribute( "name" ) = name;
                node.append_attribute( "decltype" ) = typnam;
                node.text() = value;
                return node;
            }

            template<> pugi::xml_node operator()( const char * typnam, const char * name, const std::string& value ) const {
                auto node = row.append_child( "column" );
                node.append_attribute( "name" ) = name;
                node.append_attribute( "decltype" ) = typnam;
                // std::string encoded = xmlparser::encode( value );
                node.text() = value.c_str(); // it seems that pugi automatically escape <>&
                return node;
            }

            pugi::xml_node operator()( const adfs::stmt& sql, int nCol, bool dropNull = false ) const {

                if ( sql.is_null_column( nCol ) && dropNull )
                    return pugi::xml_node();

                switch ( sql.column_type( nCol ) ) {
                case SQLITE_INTEGER:
                    return (*this)("int64_t", sql.column_name( nCol ).c_str(), sql.get_column_value< int64_t >( nCol ) );
                    break;
                case SQLITE_FLOAT:
                    return (*this)("double", sql.column_name( nCol ).c_str(), sql.get_column_value< double >( nCol ) );
                    break;
                case SQLITE_TEXT:
                    return (*this)("text", sql.column_name( nCol ).c_str(), sql.get_column_value< std::string >( nCol ).c_str() );
                    break;
                case SQLITE_BLOB: {
                    try {
                        auto uuid = sql.get_column_value< boost::uuids::uuid >( nCol );
                        return (*this)("uuid", sql.column_name( nCol ).c_str(), boost::lexical_cast<std::string>(uuid).c_str() );
                    } catch ( boost::bad_lexical_cast& ) {
                    }
                    break;
                }
                case SQLITE_NULL:
                    return (*this)("null", sql.column_name( nCol ).c_str(), 0);
                }
                return pugi::xml_node();
            }
        };
    }
}

using namespace quan;

QuanPublisher::QuanPublisher() : bProcessed_( false )
{
}

QuanPublisher::QuanPublisher( const QuanPublisher& t ) : bProcessed_( t.bProcessed_ )
                                                       , conn_( t.conn_ )
                                                       , xmldoc_( t.xmldoc_ )
                                                       , filepath_( t.filepath_ )
                                                       , calib_curves_( t.calib_curves_ )
                                                       , resp_data_( t.resp_data_ )
{
}

bool
QuanPublisher::prepare_document()
{
    xmldoc_ = std::make_shared< pugi::xml_document >();
    auto decl = xmldoc_->prepend_child( pugi::node_declaration );

    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute( "standalone" ) = "no";

    return true;
}

bool
QuanPublisher::operator()( QuanConnection * conn, std::function<void(int)> progress )
{
    if ( !(conn_ = conn->shared_from_this() ) )
        return false;

    prepare_document();

    if ( auto doc = xmldoc_->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        adcontrols::idAudit id;
        detail::append_class()(doc, id);
        
        int step = 1;
        if ( appendSampleSequence( doc ) ) {
            progress( step++ );

            if ( appendProcessMethod( doc ) ) {
                progress( step++ );

                if ( appendQuanResponseUnk( doc ) ) {
                    progress( step++ );

                    if ( appendQuanResponseStd( doc ) ) {
                        progress( step++ );

                        if ( appendQuanCalib( doc ) ) {
                            progress( step++ );

                            boost::filesystem::path path( conn->filepath() );
                            path.replace_extension( ".published.xml" );
                            filepath_ = path.string();
                            bProcessed_ = true;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool
QuanPublisher::operator()( QuanConnection * conn )
{
    if ( !(conn_ = conn->shared_from_this() ) )
        return false;

    prepare_document();

    if ( auto doc = xmldoc_->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        adcontrols::idAudit id;
        detail::append_class()(doc, id);

        if ( appendSampleSequence( doc ) &&
             appendProcessMethod( doc ) &&
             appendQuanResponseUnk( doc ) &&
             appendQuanResponseStd( doc ) &&
             appendQuanCalib( doc ) ) {
            
            boost::filesystem::path path( conn->filepath() );
            path.replace_extension( ".published.xml" );
            filepath_ = path.string();

            bProcessed_ = true;

            return true;
        }
    }
    return false;
}


bool
QuanPublisher::appendMSPeakInfo( pugi::xml_node& dst, const adcontrols::MSPeakInfo& pkInfo, int idx, int fcn )
{
    if ( auto info = pkInfo.findProtocol( fcn ) ) {
        if ( info->size() > idx ) {
            const adcontrols::MSPeakInfoItem& item = *(info->begin() + idx);
            detail::append_class()( dst, item );
            return true;
        }
    }
    return false;
}

bool
QuanPublisher::appendTraceData( pugi::xml_node& dst, const pugi::xml_node& response )
{
    auto respid = response.select_single_node( "column[@name='id']" ).node().text().as_int();

    if ( auto node = dst.append_child( "dataSource" ) )
        node.text() = response.select_single_node( "column[@name='dataSource']" ).node().text().as_string();

    if ( auto node = dst.append_child( "description" ) )
        node.text() = response.select_single_node( "column[@name='description']" ).node().text().as_string();

    if ( auto node = dst.append_child( "formula" ) )
        node.text() = response.select_single_node( "column[@name='formula' and @decltype='richtext']" ).node().text().as_string();

    int idx = response.select_single_node( "column[@name='idx']" ).node().text().as_int();
    int fcn = response.select_single_node( "column[@name='fcn']" ).node().text().as_int();

    dst.append_attribute( "idx" ) = idx;
    dst.append_attribute( "fcn" ) = fcn;
    dst.append_attribute( "formula" ) = response.select_single_node( "column[@name='formula' and @decltype='text']" ).node().text().as_string();
    dst.append_attribute( "dataGuid" ) = response.select_single_node( "column[@name='dataGuid']" ).node().text().as_string();
    dst.append_attribute( "respId" ) = respid;
                
    std::wstring dataGuid = pugi::as_wide( response.select_single_node( "column[@name='dataGuid']" ).node().text().as_string() );

    if ( auto data = conn_->fetch( dataGuid ) ) {

        //detail::append_class()(dst, data->profile->getMSProperty());
        detail::append_class()(dst, data->profile->getDescriptions());
        //appendMSPeakInfo( dst, *data->pkinfo, idx, fcn );

        QuanSvgPlot svg;
        //auto gnode = dst.append_child( "traces" );

        if ( svg.plot( *data, idx, fcn, response.select_single_node( "column[@name='dataSource']" ).node().text().as_string() ) ) {
            pugi::xml_document dom;
            if ( dom.load( svg.data(), unsigned int( svg.size() ) ) ) {
                auto trace = dst.append_child( "trace" );
                trace.append_attribute( "contents" ) = "resp_spectrum";
                trace.append_copy( dom.select_single_node( "/svg" ).node() );
            }
        }
        
        auto it = resp_data_.find( respid );
        if ( it != resp_data_.end() ) {
            if ( auto calib = find_calib_curve( it->second->cmpId ) ) {
                if ( svg.plot( *it->second, *calib ) ) {
                    pugi::xml_document dom;
                    if ( dom.load( svg.data(), unsigned int( svg.size() ) ) ) {
                        auto trace = dst.append_child( "trace" );
                        trace.append_attribute( "contents" ) = "resp_calib";
                        trace.append_copy( dom.select_single_node( "/svg" ).node() );
                    }
                }
            }
        }
    }
    
    return true;
}

bool
QuanPublisher::appendTraceData( ProgressHandler& progress )
{
    if ( bProcessed_ && conn_ ) {
        
        size_t nTask = std::count_if( resp_data_.begin(), resp_data_.end(), [] ( const decltype(*resp_data_.begin())& d ){ return d.second->sampType == 0; } );
        progress.setProgressRange( 0, int(nTask) );

        if ( auto doc = xmldoc_->select_single_node( "/qtplatz_document" ).node() ) {

            auto spectra = doc.append_child( "PlotData" );

            auto unks = doc.select_nodes( "QuanResponse[@sampleType='UNK']/row" );
            for ( auto& unk : unks ) {
                appendTraceData( spectra.append_child( "PeakResponse" ), unk.node() );
                progress();
            }
        }
        return true;        
    }
    return false;
}


bool
QuanPublisher::save_file( const char * filepath ) const
{
    if ( bProcessed_ )
        return xmldoc_->save_file( filepath );
    return false;
}

const boost::filesystem::path&
QuanPublisher::filepath() const
{
    return filepath_;
}

const QuanPublisher::calib_curve *
QuanPublisher::find_calib_curve( const boost::uuids::uuid& cmpid )
{
    auto it = calib_curves_.find( cmpid );
    if ( it != calib_curves_.end() )
        return it->second.get();
    return 0;
}

bool
QuanPublisher::appendSampleSequence( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "SampleSequence" ) ) {
        
        if ( auto p = conn_->quanSequence() ) {
            pugi::xmlhelper helper( *p );
            if ( auto xnode = node.append_child( "classdata" ) ) {
                xnode.append_attribute( "decltype" ) = typeid(*p).name();
                xnode.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
            }
        }
        return true;
    }
    return false;
}

bool
QuanPublisher::appendProcessMethod( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "ProcessMethod" ) ) {
        if ( auto pm = conn_->processMethod() ) {

            if ( auto xnode = node.append_child( "classdata" ) ) {
                xnode.append_attribute( "decltype" ) = typeid(*pm).name();
                detail::append_class()(xnode, pm->ident()); // idAudit

                for ( auto& m : *pm )
                    boost::apply_visitor( detail::append_process_method( xnode ), m );
            }
        }
        return true;
    }
    return false;
}

bool
QuanPublisher::appendQuanResponseUnk( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "QuanResponse" ) ) {

        node.append_attribute( "sampleType" ) = "UNK";

        if ( auto cmpds = conn_->processMethod()->find< adcontrols::QuanCompounds >() ) {
            for ( auto& cmpd : *cmpds ) {

                adfs::stmt sql( conn_->db() );
                if ( sql.prepare( "\
SELECT QuanCompound.uuid as cmpid, QuanResponse.id, QuanSample.name, sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass , QuanCompound.mass - QuanResponse.mass AS 'error(Da)'\
, intensity, QuanResponse.amount, QuanCompound.description, dataSource, dataGuid, fcn, idx \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanCompound.uuid = ? AND sampleType = 0 AND QuanResponse.idCmpd = QuanCompound.uuid AND QuanSample.id = QuanResponse.idSample" ) ) {
                    sql.bind( 1 ) = cmpd.uuid();
                    int nSelected = 0;
                    while ( sql.step() == adfs::sqlite_row ) {
                        auto rnode = node.append_child( "row" );
                        ++nSelected;
                        detail::append_column append( rnode );
                        for ( int col = 0; col < sql.column_count(); ++col ) {
                            append( sql, col );
                            if ( sql.column_name( col ) == "formula" ) {
                                auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                append( "richtext", "formula", text );
                            }
                        }
                        auto d = std::make_shared< resp_data >();
                        int row = 0;
                        d->cmpId      = cmpd.uuid(); row++;
                        d->respId     = sql.get_column_value< int64_t >( row++ );                        
                        d->name       = sql.get_column_value< std::string >( row++ );                        
                        d->sampType   = sql.get_column_value< int64_t >( row++ );                        
                        d->formula    = sql.get_column_value< std::string >( row++ ); 
                        row++; // sikip 'exact mass'
                        d->mass       = sql.get_column_value< double >( row++ );                        
                        d->mass_error = sql.get_column_value< double >( row++ );
                        d->intensity  = sql.get_column_value< double >( row++ );
                        d->amount     = sql.get_column_value< double >( row++ );
                        row++; // skip compound 'description'
                        d->dataSource = sql.get_column_value< std::string >( row++ );
                        d->dataGuid   = sql.get_column_value< std::string >( row++ );
                        d->fcn        = int( sql.get_column_value< int64_t >( row++ ) );
                        d->idx        = int( sql.get_column_value< int64_t >( row++ ) );
                        d->level = 0;
                        resp_data_[ d->respId ] = d;
                        rnode.append_attribute( "id" ) = d->respId;
                    }
                }
            } // for
        } //if
        return true;
    } //if
    return false;
}

bool
QuanPublisher::appendQuanResponseStd( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "QuanResponse" ) ) {

        node.append_attribute( "sampleType" ) = "STD";

        if ( auto cmpds = conn_->processMethod()->find< adcontrols::QuanCompounds >() ) {
            for ( auto& cmpd : *cmpds ) {

                adfs::stmt sql( conn_->db() );
                if ( sql.prepare( "\
SELECT QuanCompound.uuid as cmpid, QuanResponse.id, QuanSample.name, sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass , QuanCompound.mass - QuanResponse.mass AS 'error(Da)'\
, intensity, QuanSample.level, QuanResponse.amount, QuanCompound.description, dataSource, dataGuid, fcn, idx \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanCompound.uuid = ? AND sampleType = 1 AND QuanResponse.idCmpd = QuanCompound.uuid AND QuanSample.id = QuanResponse.idSample ORDER BY QuanSample.level" ) ) {
                    sql.bind( 1 ) = cmpd.uuid();
                    int nSelected = 0;
                    while ( sql.step() == adfs::sqlite_row ) {
                        auto rnode = node.append_child( "row" );
                        ++nSelected;
                        detail::append_column append( rnode );
                        for ( int col = 0; col < sql.column_count(); ++col ) {
                            append( sql, col );
                            if ( sql.column_name( col ) == "formula" ) {
                                auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                append( "richtext", "formula", text );
                            }
                        }
                        auto d = std::make_shared< resp_data >();
                        int row = 0;
                        d->cmpId      = cmpd.uuid(); row++;
                        d->respId     = sql.get_column_value< int64_t >( row++ );                        
                        d->name       = sql.get_column_value< std::string >( row++ );                        
                        d->sampType   = sql.get_column_value< int64_t >( row++ );                        
                        d->formula    = sql.get_column_value< std::string >( row++ );                        
                        d->mass       = sql.get_column_value< double >( row++ );     
                        row++; // skip 'exact mass'
                        d->mass_error = sql.get_column_value< double >( row++ );
                        d->intensity  = sql.get_column_value< double >( row++ );
                        d->level      = int( sql.get_column_value< int64_t >( row++ ) );
                        row++; // amount
                        row++; // skip 'description'
                        d->dataSource = sql.get_column_value< std::string >( row++ );
                        d->dataGuid   = sql.get_column_value< std::string >( row++ );
                        d->fcn        = int( sql.get_column_value< int64_t >( row++ ) );
                        d->idx        = int( sql.get_column_value< int64_t >( row++ ) );
                        resp_data_[ d->respId ] = d;
                        rnode.append_attribute( "id" ) = d->respId;
                    }
                }
            } // for
        } //if
        return true;
    } //if
    return false;
}

bool
QuanPublisher::appendQuanCalib( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "QuanCalib" ) ) {

        if ( auto cmpds = conn_->processMethod()->find< adcontrols::QuanCompounds >() ) {
            for ( auto& cmpd : *cmpds ) {
                
                adfs::stmt sql( conn_->db() );
                if ( sql.prepare( "SELECT QuanCompound.uuid as cmpid, formula, description, date, min_x, max_x, n, a, b, c, d, e, f \
                                          FROM QuanCalib, QuanCompound WHERE QuanCompound.uuid = ? AND idCompound = QuanCompound.id" ) ) {
                    sql.bind( 1 ) = cmpd.uuid();
                    int nSelected = 0;
                    while ( sql.step() == adfs::sqlite_row ) {
                        auto rnode = node.append_child( "row" );
                        ++nSelected;
                        detail::append_column append( rnode );
                        for ( int col = 0; col < sql.column_count(); ++col ) {
                            append( sql, col, true );  // drop null column
                            if ( sql.column_name( col ) == "formula" ) {
                                auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                append( "richtext", "formula", text );
                            }
                        }
                        
                        auto calib = std::make_shared< calib_curve >();
                        if ( calib ) {
                            int row = 0;
                            calib->uuid = sql.get_column_value< boost::uuids::uuid >( row++ ); // cmpid
                            calib->formula = sql.get_column_value< std::string >( row++ );
                            calib->description = sql.get_column_value< std::wstring >( row++ );
                            calib->date = sql.get_column_value< std::string >( row++ );
                            calib->min_x = sql.get_column_value< double >( row++ );
                            calib->max_x = sql.get_column_value< double >( row++ );
                            calib->n = sql.get_column_value< uint64_t >( row++ );

                            for ( int i = 'a'; i <= 'f'; ++i ) {
                                if ( sql.is_null_column( row ) )
                                    break;
                                calib->coeffs.push_back( sql.get_column_value< double >( row++ ) );
                            }
                            
                            calib_curves_[ cmpd.uuid() ] = calib;
                        }
                        
                        adfs::stmt sql2( conn_->db() );
                        if ( sql2.prepare("\
SELECT QuanAmount.level, QuanAmount.amount, QuanResponse.intensity, QuanResponse.formula, QuanSample.dataGuid, QuanResponse.id \
FROM QuanCompound, QuanSample, QuanAmount, QuanResponse \
WHERE QuanCompound.uuid = :uuid AND QuanCompound.uuid = QuanAmount.idCmpd AND QuanCompound.uuid = QuanResponse.idCmpd \
AND sampleType = 1 AND QuanResponse.idSample = QuanSample.id AND QuanAmount.level = QuanSample.level \
ORDER BY QuanAmount.level" ) ) {

                            auto response_node = rnode.append_child( "response" );

                            sql2.bind( 1 ) = cmpd.uuid();
                            
                            while ( sql2.step() == adfs::sqlite_row ) {
                                auto row_node = response_node.append_child( "row" );
                                detail::append_column append2( row_node );
                                for ( int col = 0; col < sql2.column_count(); ++col )
                                    append2( sql2, col ); // don't drop null column

                                int level = int( sql2.get_column_value<int64_t>( 0 ) );
                                double amount = sql2.get_column_value<double>( 1 );
                                double intens = sql2.get_column_value<double>( 2 );
                                calib->std_amounts[ level ] = amount;
                                calib->respIds.push_back( std::make_pair( level, sql2.get_column_value<int64_t>( 5 ) ) );
                                calib->xy.push_back( std::make_pair( intens, amount ) );
                            }
                        }
                    } // for
                } //if
            }
            return true;
        }
    }
    return false;
}

