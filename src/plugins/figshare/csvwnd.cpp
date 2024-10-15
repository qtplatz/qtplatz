/**************************************************************************
** Copyright (C) 2014-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "csvwnd.hpp"
#include "csvplot.hpp"
#include "csvwidget.hpp"
#include "csvtodata.hpp"
#include <QtCore/qjsondocument.h>
#include <QtCore/qnamespace.h>
#include <QtGui/qtextdocument.h>
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/csv_reader.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/create_widget.hpp>
#if HAVE_RDKit
# include <GraphMol/SmilesParse/SmilesParse.h>
# include <GraphMol/RDKitBase.h>
# include <GraphMol/SmilesParse/SmilesParse.h>
# include <GraphMol/SmilesParse/SmilesWrite.h>
# include <GraphMol/Descriptors/MolDescriptors.h>
# include <GraphMol/FileParsers/MolSupplier.h>
# include <GraphMol/inchi.h>
#endif

#include <QBoxLayout>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSplitter>
#include <QSvgRenderer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QWidget>

#include <boost/algorithm/string/trim.hpp>
#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <map>
#include <memory>

// ---------------------- overloads -------------------->
namespace {
    // helper type for the visitor #4
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}
//<---------------------- overloads --------------------

namespace {

    struct csv_parser {
        std::vector< adportable::csv::list_string_type > operator()( const QByteArray& ba ) const {
            std::istringstream in(ba.toStdString() );
            std::vector< adportable::csv::list_string_type > res;

            adportable::csv::list_string_type alist; // pair<variant,string>
            adportable::csv::csv_reader reader;
            while ( reader.read( in, alist ) && in.good() ) {
                for ( const auto& value: alist )
                    res.emplace_back( alist );
            }
            return res;
        }
    };

}

namespace figshare {

    class CSVWnd::impl {
    public:
    };
}

using namespace figshare;

CSVWnd::~CSVWnd()
{
    delete impl_;
}

CSVWnd::CSVWnd( QWidget * parent ) : QWidget( parent )
                                   , impl_( new impl{} )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {
        topLayout->setContentsMargins( {} );
        topLayout->setSpacing( 0 );
        if ( auto hsplitter = adwidgets::add_widget( topLayout, adwidgets::create_widget< QSplitter >( "hor" ) ) ) {
            hsplitter->setOrientation( Qt::Horizontal );

            if ( auto vsplitter = adwidgets::add_widget( hsplitter, adwidgets::create_widget< QSplitter >( "ver" ) ) ) {
                vsplitter->setOrientation( Qt::Vertical );
                if ( auto tw = adwidgets::add_widget( vsplitter, adwidgets::create_widget< QTabWidget >( "tabWidget" ) ) ) {
                    tw->setDocumentMode( true );
                    if ( auto widget = adwidgets::create_widget< CSVWidget >( "CSVWidget" ) )
                        tw->addTab( widget, "CSV.1" );
                }
            }
            if ( auto form2 = adwidgets::add_widget( hsplitter, adwidgets::create_widget< CSVPlot >( "csvPlot" ) ) ) {
            }
        }
    }
}

void
CSVWnd::handleReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWnd::handleFigshareReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWnd::handleDownloadReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWnd::handleCSVReply( const QByteArray& ba, const QString& url, size_t id )
{
    auto vlist = csv_parser()( ba );

    if ( auto tw = findChild< QTabWidget * >( "tabWidget" ) ) {
        if ( id == 0 )
            tw->clear();

        if ( auto widget = adwidgets::create_widget< CSVWidget >( std::format( "CSVWidget.{}", id).c_str() ) ) {
            tw->addTab( widget, QString( "CSV.%1" ).arg( id + 1 ) );
            widget->setData( vlist );
        }
    }

    if ( auto plot = findChild< CSVPlot * >( "csvPlot" ) ) {

        if ( id == 0 )
            plot->clear();

        auto datum = csv::toData()( vlist );
        std::visit( overloaded{
                [&]( std::shared_ptr< adcontrols::MassSpectrum > t ) { plot->setData( t, id );  }
                    , [&]( std::shared_ptr< adcontrols::Chromatogram > t ) { }
                    , [&]( std::shared_ptr< QwtSeriesData< QPointF> > t ) {  }
                    }, datum );
    }
}

void
CSVWnd::setData( const QByteArray& ba )
{
    std::istringstream in(ba.toStdString() );

    adportable::csv::list_string_type alist; // pair<variant,string>
    adportable::csv::csv_reader reader;
     while ( reader.read( in, alist ) && in.good() ) {
         QStringList list;
         for ( const auto& value: alist )
             list << QString::fromStdString( std::get<1>(value) );
         qDebug() << list;
     }
}
