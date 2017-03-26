/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "quanprogress.hpp"
#include "quansvgplot.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
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
#include <adportable/debug.hpp>
#include <adpublisher/document.hpp>
#include <adfs/sqlite3.h>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106000
#include <boost/uuid/uuid_io.hpp>
#endif
#include <QCoreApplication>

namespace quan {
    namespace detail {

        struct append_class {

            template< class T > pugi::xml_node operator()( pugi::xml_node& node, const T& data, const char * decl ) const {

                pugi::xmlhelper helper;
                if ( helper( data ) ) {
                    auto leaf = helper.doc().select_single_node( "/boost_serialization" ).node();
                    leaf.append_attribute( "decltype" ) = decl;
                    node.append_copy( leaf );
                    return leaf;
                } else {
                    auto leaf = node.append_child( "boost_serialization" );
                    leaf.append_attribute( "decltype" ) = decl;
                    return leaf; // empty node to be returned
                }
            }
        };

        struct append_column {
            pugi::xml_node& row;
            append_column( pugi::xml_node& n ) : row( n ) {}

            template<typename T> pugi::xml_node operator()( const char * decl, const char * name, const T& value ) const {
                auto node = row.append_child( "column" );
                node.append_attribute( "name" ) = name;
                node.append_attribute( "decltype" ) = decl;
                node.text() = value;
                return node;
            }
            // template<> pugi::xml_node operator()( const char * decl, const char * name, const int64_t& value ) const;

            pugi::xml_node operator()( const adfs::stmt& sql, int nCol, bool dropNull = false ) const {
            
                if ( sql.is_null_column( nCol ) && dropNull )
                    return pugi::xml_node();
            
                switch ( sql.column_type( nCol ) ) {
                case SQLITE_INTEGER:
#if defined __GNUC__ && defined __linux
                    return (*this)("int64_t", sql.column_name( nCol ).c_str(), static_cast< long long >( sql.get_column_value< int64_t >( nCol ) ) );
#else
                    return (*this)("int64_t", sql.column_name( nCol ).c_str(), sql.get_column_value< int64_t >( nCol ) );
#endif
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

        template<> pugi::xml_node append_column::operator()( const char * typnam, const char * name, const std::string& value ) const {
            auto node = row.append_child( "column" );
            node.append_attribute( "name" ) = name;
            node.append_attribute( "decltype" ) = typnam;
            node.text() = value.c_str(); // it seems that pugi automatically escape <>&
            return node;
        }

        //-----
    }
}

using namespace quan;

QuanPublisher::QuanPublisher() : bProcessed_( false )
{
}

QuanPublisher::QuanPublisher( const QuanPublisher& t ) : bProcessed_( t.bProcessed_ )
                                                       , conn_( t.conn_ )
                                                       , filepath_( t.filepath_ )
                                                       , calib_curves_( t.calib_curves_ )
                                                       , resp_data_( t.resp_data_ )
{
}

bool
QuanPublisher::prepare_document()
{
    xmloutput_ = std::make_shared< pugi::xml_document >(); // reset
    auto decl = xmloutput_->prepend_child( pugi::node_declaration );

    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute( "standalone" ) = "no";

    return true;
}

bool
QuanPublisher::operator()( QuanConnection * conn, std::function<void(int)> progress, pugi::xml_document * article )
{
    if ( !(conn_ = conn->shared_from_this() ) )
        return false;

    std::shared_ptr< pugi::xml_document > xmloutput;
    prepare_document();

    if ( auto doc = xmloutput_->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        adcontrols::idAudit id;
        detail::append_class()(doc, id, "class adcontrols::idAudit");

        if ( article ) {
            doc.append_copy( article->select_single_node( "/article" ).node() );
        }
        
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
        
                            if ( appendQuanDataGuids( doc ) ) {
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
    }
    return false;
}

bool
QuanPublisher::operator()( QuanConnection * conn )
{
    if ( !(conn_ = conn->shared_from_this() ) )
        return false;

    prepare_document();

    if ( auto doc = xmloutput_->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        adcontrols::idAudit id;
        detail::append_class()(doc, id, "class adcontrols::idAudit");

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
QuanPublisher::appendMSPeakInfo( pugi::xml_node& dst, const adcontrols::MSPeakInfo& pkInfo, size_t idx, int fcn )
{
    if ( auto info = pkInfo.findProtocol( fcn ) ) {
        if ( info->size() > idx ) {
            const adcontrols::MSPeakInfoItem& item = *(info->begin() + idx);
            detail::append_class()( dst, item, "class adcontrols::MSPeakInfoItem" );
            return true;
        }
    }
    return false;
}

bool
QuanPublisher::appendTraceData( pugi::xml_node dst, const pugi::xml_node& response, const std::string& refGuid, int refidx, int reffcn )
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
    std::string formula = response.select_single_node( "column[@name='formula' and @decltype='text']" ).node().text().as_string();

    dst.append_attribute( "idx" ) = idx;
    dst.append_attribute( "fcn" ) = fcn;
    dst.append_attribute( "formula" ) = response.select_single_node( "column[@name='formula' and @decltype='text']" ).node().text().as_string();
    dst.append_attribute( "dataGuid" ) = response.select_single_node( "column[@name='dataGuid']" ).node().text().as_string();
    dst.append_attribute( "respId" ) = respid;

    // ADDEBUG() << "appendTrace( idx,fcn, formula )=" << idx << ", " << fcn << ", " << formula;

    std::pair< double, double > range( 0, 0 );
    double exact_mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );
    if ( exact_mass > 0.5 )
        range = std::make_pair( exact_mass - 0.05, exact_mass + 0.05 );
                
    std::wstring dataGuid = pugi::as_wide( response.select_single_node( "column[@name='dataGuid']" ).node().text().as_string() );

    if ( auto data = conn_->fetch( dataGuid ) ) {

        if ( data->chromatogram ) {

            detail::append_class()( dst, data->chromatogram->getDescriptions(), "class adcontrols::descriptions" );
            QuanSvgPlot svg;

            if ( svg.plot( *data, idx, fcn, response.select_single_node( "column[@name='dataSource']" ).node().text().as_string() ) ) {
                pugi::xml_document dom;
                if ( dom.load( svg.data(), static_cast<unsigned int>( svg.size() ) ) ) {
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
                        if ( dom.load( svg.data(), static_cast<unsigned int>( svg.size() ) ) ) {
                            auto trace = dst.append_child( "trace" );
                            trace.append_attribute( "contents" ) = "resp_calib";
                            trace.append_copy( dom.select_single_node( "/svg" ).node() );
                        }
                    }
                }
            }
            
        } else if ( data->profile ) {

            detail::append_class()( dst, data->profile->getDescriptions(), "class adcontrols::descriptions" );

            QuanSvgPlot svg;

            if ( svg.plot( *data, idx, fcn, response.select_single_node( "column[@name='dataSource']" ).node().text().as_string(), range ) ) {
                pugi::xml_document dom;
                if ( dom.load( svg.data(), static_cast<unsigned int>( svg.size() ) ) ) {
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
                        if ( dom.load( svg.data(), static_cast<unsigned int>( svg.size() ) ) ) {
                            auto trace = dst.append_child( "trace" );
                            trace.append_attribute( "contents" ) = "resp_calib";
                            trace.append_copy( dom.select_single_node( "/svg" ).node() );
                        }
                    }
                }
            }
        }
    }
    if ( !refGuid.empty() ) {

        if ( auto data = conn_->fetch( pugi::as_wide( refGuid ) ) ) {

            if ( data->profile ) {

                detail::append_class()( dst, data->profile->getDescriptions(), "class adcontrols::descriptions" );

                QuanSvgPlot svg;
                
                if ( svg.plot( *data, refidx, reffcn, "data source tba" ) ) {
                    pugi::xml_document dom;
                    if ( dom.load( svg.data(), static_cast<unsigned int>( svg.size() ) ) ) {
                        auto trace = dst.append_child( "trace" );
                        trace.append_attribute( "contents" ) = "resp_reference_spectrum";
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
        
        size_t nTask = std::count_if( resp_data_.begin(), resp_data_.end()
                                      , [] ( decltype(*resp_data_.begin())& d ){ return d.second->sampType == 0; } );
        progress.setProgressRange( 0, int(nTask) );


        if ( auto doc = xmloutput_->select_single_node( "/qtplatz_document" ).node() ) {

            // list reference data guids
            std::map< std::string, std::tuple< std::string, int, int > > refData;
            auto guids = doc.select_nodes( "QuanDataGuids/row" );
            for ( auto& guid : guids ) {
                auto dataGuidStr = guid.node().select_single_node( "column[@name='dataGuid']" ).node().text().as_string();
                refData[ dataGuidStr ] =
                    std::make_tuple( guid.node().select_single_node( "column[@name='refDataGuid']" ).node().text().as_string()
                                     , guid.node().select_single_node( "column[@name='idx']" ).node().text().as_int()
                                     , guid.node().select_single_node( "column[@name='fcn']" ).node().text().as_int() );
            }
            //---

            auto spectra = doc.append_child( "PlotData" );

            auto stds = doc.select_nodes( "QuanResponse[@sampleType='STD']/row" );
            for ( auto& std : stds ) {
                std::string refGuid;
                int fcn(0), idx(0);
                auto it = refData.find( std.node().select_single_node( "column[@name='dataGuid']" ).node().text().as_string() );
                if ( it != refData.end() ) {
                    refGuid = std::get<0>(it->second);
                    idx = std::get<1>(it->second);
                    fcn = std::get<2>(it->second);                    
                }
                
                appendTraceData( spectra.append_child( "PeakResponse" ), std.node(), refGuid, idx, fcn );
                progress();
            }

            auto unks = doc.select_nodes( "QuanResponse[@sampleType='UNK']/row" );
            for ( auto& unk : unks ) {
                std::string refGuid;
                int fcn(0), idx(0);                
                auto it = refData.find( unk.node().select_single_node( "column[@name='dataGuid']" ).node().text().as_string() );
                if ( it != refData.end() ) {
                    refGuid = std::get<0>(it->second);
                    idx = std::get<1>(it->second);
                    fcn = std::get<2>(it->second);                                        
                }
                
                appendTraceData( spectra.append_child( "PeakResponse" ), unk.node(), refGuid, idx, fcn );
                progress();
            }

        }
        return true;        
    }
    return false;
}

bool
QuanPublisher::save( std::ostream& os ) const
{
    if ( bProcessed_ ) {
        xmloutput_->save( os );
        return true;
    }
    return false;
}

bool
QuanPublisher::save_file( const char * filepath ) const
{
    if ( bProcessed_ )
        return xmloutput_->save_file( filepath );
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
        
        if ( auto p = conn_->quanSequence() ) 
            detail::append_class()(node, *p, "class adcontrols::QuanSequence");
        return true;
    }
    return false;
}

bool
QuanPublisher::appendProcessMethod( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "ProcessMethod" ) ) {
        if ( auto pm = conn_->processMethod() ) {
            detail::append_class()(node, *pm, "class adcontrols::ProcessMethod");
            return true;
        }
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
                        rnode.append_attribute( "id" ) = int( d->respId );
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
                        rnode.append_attribute( "id" ) = int( d->respId );
                    }
                }
            } // for
        } //if
        return true;
    } //if
    return false;
}

bool
QuanPublisher::appendCountingCalib( pugi::xml_node& doc, bool isISTD )
{
    std::string query =
        "SELECT idCmpd, level, amount, r1.CR/r2.CR as intensity, formula, dataGuid, id FROM "
        "(SELECT QuanResponse.idCmpd,QuanAmount.level, QuanAmount.amount, QuanResponse.intensity/QuanResponse.trigCounts as CR"
        ", QuanResponse.formula, dataGuid, QuanResponse.id, idSample"
        " FROM QuanCompound, QuanSample, QuanAmount, QuanResponse"
        " WHERE QuanCompound.uuid = QuanResponse.idCmpd "
        " AND QuanCompound.uuid = QuanAmount.idCmpd"
        " AND QuanCompound.uuid = QuanResponse.idCmpd"
        " AND sampleType = 1 "
        " AND QuanResponse.idSample = QuanSample.id"
        " AND QuanAmount.level = QuanSample.level AND isISTD=0 ) r1"
        " LEFT JOIN"
        "(SELECT idSample,QuanResponse.intensity/QuanResponse.trigCounts as CR"
        " FROM QuanResponse,QuanCompound"
        " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) r2"
        " ON r1.idSample=r2.idSample ORDER by level, r1.idSample";

    if ( auto node = doc.append_child( "QuanCalib" ) ) {

        // load calibration from db
        adfs::stmt sql( conn_->db() );
        if ( sql.prepare( "SELECT QuanCalib.idCmpd, formula, description, date, min_x, max_x, n, a, b, c, d, e, f"
                          " FROM QuanCalib,QuanCompound where QuanCalib.idCmpd = QuanCompound.uuid" ) ) {
            while ( sql.step() == adfs::sqlite_row ) {
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
                    calib_curves_[ calib->uuid ] = calib;
                }
            }
        }

        if ( sql.prepare( query ) ) {
            while ( sql.step() == adfs::sqlite_row ) {
                auto cmpId = sql.get_column_value< boost::uuids::uuid >( 0 );
                auto it = calib_curves_.find( cmpId );
                if ( it != calib_curves_.end() ) {
                    if ( auto calib = it->second ) {
                        int level = int( sql.get_column_value<int64_t>( 1 ) );
                        double amount = sql.get_column_value<double>( 2 );
                        double intens = sql.get_column_value<double>( 3 );
                        int64_t respid = int( sql.get_column_value< int64_t >( 6 ) );
                        calib->std_amounts[ level ] = amount;
                        calib->respIds.push_back( std::make_pair( level, respid ) );
                        calib->xy.push_back( std::make_pair( intens, amount ) );
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool
QuanPublisher::appendQuanCalib( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "QuanCalib" ) ) {

        bool isCounting( false );
        bool isISTD( false );
        {
            adfs::stmt sql( conn_->db() );
            sql.prepare( "SELECT isCounting,isISTD FROM QuanMethod LIMIT(1)" );
            while ( sql.step() == adfs::sqlite_row ) {
                isCounting = bool( sql.get_column_value<int64_t>( 0 ) );
                isISTD = bool( sql.get_column_value<int64_t>( 1 ) );
            }
            if ( isCounting && isISTD )
                return appendCountingCalib( doc, isISTD );
        }

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
                        std::string query =
                            "SELECT QuanAmount.level, QuanAmount.amount, QuanResponse.intensity"
                            ", QuanResponse.formula, dataGuid, QuanResponse.id" // <- respId
                            " FROM QuanCompound, QuanSample, QuanAmount, QuanResponse"
                            " WHERE QuanCompound.uuid = :uuid "
                            " AND QuanCompound.uuid = QuanAmount.idCmpd"
                            " AND QuanCompound.uuid = QuanResponse.idCmpd"
                            " AND sampleType = 1 "
                            " AND QuanResponse.idSample = QuanSample.id"
                            " AND QuanAmount.level = QuanSample.level"
                            " ORDER BY QuanAmount.level";

                        if ( sql2.prepare( query ) ) {
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

bool
QuanPublisher::appendQuanDataGuids( pugi::xml_node& doc )
{
    if ( auto node = doc.append_child( "QuanDataGuids" ) ) {

        adfs::stmt sql( conn_->db() );
        if ( sql.prepare(
                 "SELECT id, QuanResponse.dataGuid, refDataGuid,QuanDataGuids.idx,QuanDataGuids.fcn"
                 " FROM QuanResponse, QuanDataGuids WHERE QuanResponse.dataGuid = QuanDataGuids.dataGuid" ) ) {

            int nSelected = 0;
            while ( sql.step() == adfs::sqlite_row ) {
                auto rnode = node.append_child( "row" );
                detail::append_column append( rnode );
                for ( int col = 0; col < sql.column_count(); ++col ) {
                    append( sql, col );
                }
            } // for
        } //if
        return true;
    } //if
    return false;
}
