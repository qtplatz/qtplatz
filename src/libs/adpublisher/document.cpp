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

#include "document.hpp"

#if defined Q_OS_WIN32
#  include "msxml_transformer.hpp"
#else 
# if defined Q_OS_MAC
#  include "libxslt_transformer.hpp"
# endif
#endif

#include <xmlparser/pugixml.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/filesystem.hpp>
#include <QApplication>
#include <QFileInfo>
//#include <QXmlQuery>

using namespace adpublisher;

document::document() : doc_( std::make_shared< pugi::xml_document >() )
{
    if ( auto comment = doc_->append_child( pugi::node_comment ) ) {
        comment.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );
    }
    if ( auto article = doc_->append_child( "article" ) ) {

        if ( auto title = article.append_child( "title" ) ) {
            title.text().set( "Qualitative and quantitative accuracy on LC/TOF-MS with newly developped targeting algorihm" );
        }

        if ( auto author = article.append_child( "author" ) ) {
            author.text().set( "Kristie C. Cloos, Katherine M. Schroeder and Toshinobu Hondo" );
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "section_1";

            if ( auto title = sec.append_child( "title" ) )
                title.text().set( "Introduction" );

            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.text().set( "\
A single LC/TOFMS offers the advantage of a full scan which, by allowing all analytes to reach the detector, gives way for multiple applications. However, analyzing the data acquired from a full scan can quickly become a limiting factor in high-throughput (HT) analysis. Accurate results in qualitative and quantitative high-performance liquid chromatographic (HPLC) analysis are fundamentally dependant on the degree of separation obtained for the component peak. In a full scan, analytes can experience suppression as they might still contain contaminates which can lead to inaccurate intensities and co-eluted compounds. Another disadvantage is the inability to avoid drift which makes it difficult to target the analytes of choice using current targeting software where a time range must be entered for each compound. \
The work presented here shows how using a newly developed targeting algorithm allows an isolated mass spectrum to be targeted from a complex sample matrix. This technique allows a single LC/TOFMS to become an instrument that can be used for quantitative HT analysis." );
            }
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "section_2";

            if ( auto title = sec.append_child( "title" ) )
                title.text().set( "Methods" );

            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.text().set( "MS Conditions:" );
            }

            if ( auto table = sec.append_child( "table" ) ) {
                table.append_attribute( "id" ) = "table_1";
                if ( auto tabular = table.append_child( "tabular" ) ) {
                    tabular.append_attribute( "dataClass" ) = "ControlMethod";
                }
            }

            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.text().set( "Quantitative analysis method:" );
            }

            if ( auto table = sec.append_child( "table" ) ) {
                table.append_attribute( "id" ) = "table_2";
                if ( auto tabular = table.append_child( "tabular" ) ) {
                    tabular.append_attribute( "dataClass" ) = "QuanMethod";
                }
            }
        }
        
        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "section_3";
            if ( auto title = sec.append_child( "title" ) )
                title.text().set( "Results" );
            
            if ( auto ssec = sec.append_child( "subsection" ) ) {
                if ( auto title = ssec.append_child( "title" ) )
                    title.text().set( "Summary Report" ); 

                if ( auto table = ssec.append_child( "table" ) ) {
                    if ( auto tabular = table.append_child( "tabular" ) )
                        tabular.append_attribute( "dataClass" ) = "SummaryReport";
                }
            }

            if ( auto ssec = sec.append_child( "subsection" ) ) {
                if ( auto title = ssec.append_child( "title" ) )
                    title.text().set( "Quantitative Analysis Report" );

                if ( auto table = ssec.append_child( "table" ) ) {

                    if ( auto foreach = table.append_child( "foreach" ) ) {

                        foreach.append_attribute( "Sample" ) = "*";
                        foreach.append_attribute( "SampleType" ) = 0; // UNK

                        if ( auto compound = foreach.append_child( "foreach" ) ) {

                            compound.append_attribute( "Compound" ) = "*";

                            auto grid = compound.append_child( "tabular" );
                            grid.append_attribute("dataClass") = "PeakInfoItem";

                            auto svg1 = compound.append_child( "figure" );
                            svg1.append_attribute( "dataClass" ) = "MassSpectrum";
                            svg1.append_attribute( "format" ) = "svg";

                            auto svg2 = compound.append_child( "figure" );
                            svg2.append_attribute( "dataClass" ) = "CalibrationCurve";
                            svg2.append_attribute( "format" ) = "svg";

                        }
                    }                        
                }
            }
        }

    }
}

bool
document::save_file( const char * filepath ) const
{
    return doc_->save_file( filepath );
}

bool
document::load_file( const char * filepath )
{
    return doc_->load_file( filepath );
}

bool
document::save( std::ostream& o ) const
{
    doc_->save( o );
    return true;
}

bool
document::save( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    doc_->save( device );
    return true;
}

bool
document::load( const char * xml )
{
    return doc_->load( xml );
}

std::shared_ptr< pugi::xml_document >
document::xml_document()
{
    return doc_;
}

//static
bool
document::apply_template( const char * xmlfile, const char * xsltfile, QString& output, QString& method )
{
#if defined Q_OS_WIN32
    using namespace msxml;
#else
    using namespace libxslt;
#endif

    if ( !boost::filesystem::exists( xmlfile ) )
        return false;

    boost::filesystem::path xslt( xsltfile );
    if ( !xslt.is_absolute() )
        transformer::xsltpath( xslt, xsltfile );

    if ( !boost::filesystem::exists( xslt ) )
        return false;

    pugi::xml_document doc;
    if ( doc.load_file( xsltfile ) ) {
        if ( auto node = doc.select_single_node( "//xsl:output[@method]" ) )
            method = node.node().attribute( "method" ).value();
    }

    return transformer::apply_template( xmlfile, xslt.string().c_str(), output );
}

