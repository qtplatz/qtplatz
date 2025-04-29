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

#include "csvplot.hpp"
#include "csvwidget.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/create_widget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chartview.hpp>
#include <adplot/plot.hpp>

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
#include <boost/format.hpp>
#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <map>
#include <memory>
#include <format>

namespace figshare {

    class CSVPlot::impl {
    public:
        std::vector< std::unique_ptr< adplot::plot > > plots_;
    };
}

using namespace figshare;

CSVPlot::~CSVPlot()
{
    delete impl_;
}

CSVPlot::CSVPlot( QWidget * parent ) : QWidget( parent )
                                     , impl_( new impl{} )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {
        topLayout->setContentsMargins( {} );
        topLayout->setSpacing( 0 );

        if ( auto splitter = adwidgets::add_widget( topLayout, adwidgets::create_widget< QSplitter >( "splitter" ) ) ) {
            splitter->setOrientation( Qt::Vertical );

            impl_->plots_.emplace_back( adwidgets::add_widget( splitter, adwidgets::create_widget< adplot::SpectrumWidget >( "x" ) ) );
        }

        // plots_.emplace_back( std::make_shared< adplot::plot >() );
    }
}

void
CSVPlot::clear()
{
    impl_->plots_.clear();    // <-- this will delete all the widgets holded by QSplitter
}

void
CSVPlot::setData( const std::vector< adportable::csv::list_string_type >& alist, size_t id )
{
    if ( auto splitter = findChild< QSplitter * >( "splitter" ) ) {
#if defined __GNUC__ && __GNUC__ <= 12
        const auto tag = (boost::format( "CSV.%1%" ) % id ).str();
#else
        const auto tag = std::format( "CSV.{}", id );
#endif
        auto plot = adwidgets::add_widget( splitter, adwidgets::create_widget< adplot::plot >( tag.c_str() ) );
        impl_->plots_.emplace_back( plot );
        plot->setTitle( tag );
    }
}

void
CSVPlot::setData( const std::shared_ptr< adcontrols::MassSpectrum > ms, size_t id )
{
    if ( auto splitter = findChild< QSplitter * >( "splitter" ) ) {
#if defined __GNUC__ && __GNUC__ <= 12
        const auto tag = ( boost::format("CSV.%d" ) % id ).str();
#else
        const auto tag = std::format( "CSV.{}", id );
#endif
        auto plot = adwidgets::add_widget( splitter, adwidgets::create_widget< adplot::SpectrumWidget >( tag.c_str() ) );
        impl_->plots_.emplace_back( plot );
        plot->setTitle( tag );
        plot->setData( ms, 0, QwtPlot::yLeft );

        if ( id > 0 ) {
            if ( auto prev = qobject_cast< adplot::SpectrumWidget * >( splitter->widget( id - 1 ) ) ) {
                prev->link( plot );
            }
        }
    }
}

void
CSVPlot::setData( const std::shared_ptr< adcontrols::Chromatogram >, size_t id )
{
    ADDEBUG() << "--------------- setData(" << id << ")";
}

void
CSVPlot::setData( const std::shared_ptr< QwtSeriesData< QPointF > >, size_t id )
{
    ADDEBUG() << "--------------- setData(" << id << ")";
}
