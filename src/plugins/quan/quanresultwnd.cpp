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

#include "quanresultwnd.hpp"
#include "quandocument.hpp"
#include "quanresultwidget.hpp"
#include "quanresulttable.hpp"
#include "quanconnection.hpp"
#include "quanquery.hpp"
#include "quancmpdwidget.hpp"
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adwplot/dataplot.hpp>
#include <utils/styledbar.h>
#include <coreplugin/minisplitter.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <QBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <boost/uuid/uuid.hpp>

namespace quan {
    namespace detail {

        struct calib_curve {
            boost::uuids::uuid uuid;     // cmpdId
            std::string formula;
            std::wstring description;
            size_t n;
            double min_x;
            double max_x;
            std::string date;
            std::vector< double > coeffs;
            calib_curve() {}
            calib_curve( const calib_curve& t ) = delete;
        };

        struct calib_data {
            std::string formula;
            QVector< QPointF > xy;
            calib_data() {}
            calib_data( const calib_data& t ) = delete;
        };


    }
}

using namespace quan;

QuanResultWnd::QuanResultWnd(QWidget *parent) : QWidget(parent)
                                              , cmpdWidget_( new QuanCmpdWidget )
                                              , respTable_( new QuanResultWidget )
                                              , calibplot_( std::make_shared< adwplot::Dataplot >() )
{
    QwtPlotGrid * grid = new QwtPlotGrid;
    grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
    grid->attach( calibplot_.get() );
    //calibplot_->setAxisTitle( QwtPlot::xBottom, "intensity" );
    //calibplot_->setAxisTitle( QwtPlot::yLeft, "amount" );

    Core::MiniSplitter * splitter = new Core::MiniSplitter;// compound-table | plots
    
    splitter->setOrientation( Qt::Horizontal );
    
    // compound-table on left
    if ( Core::MiniSplitter * wndSplitter = new Core::MiniSplitter ) {
        splitter->addWidget( wndSplitter ); // <<------------ add to splitter

        wndSplitter->setOrientation( Qt::Vertical );  // horizontal line
        wndSplitter->addWidget( respTable_ );

        if ( Core::MiniSplitter  * splitter2 = new Core::MiniSplitter ) { // left pane split top (table) & bottom (time,mass plot)
            splitter2->setOrientation( Qt::Horizontal );        // Plot | Text
            wndSplitter->addWidget( splitter2 );
            
            splitter2->addWidget( calibplot_.get() );
            splitter2->addWidget( new QTextEdit );
        }
    }
    splitter->addWidget( cmpdWidget_ );
    splitter->setStretchFactor( 0, 5 );
    splitter->setStretchFactor( 1, 1 );
    
    auto layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( splitter );

    connect( QuanDocument::instance(), &QuanDocument::onConnectionChanged, this, &QuanResultWnd::handleConnectionChanged );
    connect( &cmpdWidget_->table(), &QuanResultTable::onCurrentChanged, this, &QuanResultWnd::handleCompoundSelected );
}

void
QuanResultWnd::handleConnectionChanged()
{
    if ( auto connection = QuanDocument::instance()->connection() ) {
        respTable_->setConnection ( connection );

        if ( auto query = connection->query() ) {
            if ( query->prepare( std::wstring ( L"SELECT uuid, formula, description FROM QuanCompound" ) ) ) {
                //cmpdWidget_->table().setColumnHide( "uuid" );
                cmpdWidget_->table().prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    cmpdWidget_->table().addRecord( *query );
                }
            }
        }

    }
}

void
QuanResultWnd::handleCompoundSelected( const QModelIndex& index )
{
    boost::uuids::uuid uuid = cmpdWidget_->uuid( index.row() );
    if ( uuid != boost::uuids::uuid() ) {
        if ( auto conn = QuanDocument::instance()->connection() ) {

            auto calib = std::make_shared< detail::calib_curve >();
            auto data = std::make_shared< detail::calib_data >();
            
            adfs::stmt sql( conn->db() );
            if ( sql.prepare("SELECT formula, description, n, min_x, max_x, date, a, b, c, d, e, f \
FROM QuanCalib, QuanCompound WHERE idCmpd = :uuid AND QuanCalib.idCmpd = QuanCompound.uuid") ) {
                sql.bind(1) = uuid;

                while ( sql.step() == adfs::sqlite_row ) {
                    int row = 0;
                    calib->uuid = uuid;
                    calib->formula = sql.get_column_value< std::string >( row++ );
                    calib->description = sql.get_column_value< std::wstring >( row++ );                    
                    calib->n = sql.get_column_value< uint64_t >( row++ );
                    calib->min_x = sql.get_column_value< double >( row++ );
                    calib->max_x = sql.get_column_value< double >( row++ );
                    calib->date = sql.get_column_value< std::string >( row++ );

                    for ( int i = 0; i < 5; ++i ) {
                        if ( sql.is_null_column( row ) )
                            break;
                        calib->coeffs.push_back( sql.get_column_value< double >( row++ ) );
                    }
                }
                calib_curves_[ uuid ] = calib;
            }

            if ( sql.prepare( "\
SELECT QuanSample.name, QuanSample.level, formula, mass, intensity, QuanAmount.amount \
FROM QuanSample, QuanResponse, QuanAmount \
WHERE sampleType = 1 \
AND QuanResponse.idCmpd = :uuid \
AND QuanSample.id = QuanResponse.idSample \
AND QuanAmount.level = QuanSample.level AND QuanAmount.idCmpd = QuanResponse.idCmpd" ) ) {
                sql.bind( 1 ) = uuid;
                
                while( sql.step() == adfs::sqlite_row ) {
                    int row = 0;
                    std::string name = sql.get_column_value< std::string >( row++ );                    
                    uint64_t level = sql.get_column_value< uint64_t >( row++ );
                    std::string formula = sql.get_column_value< std::string >( row++ );
                    double mass = sql.get_column_value< double >( row++ );
                    (void)level;
                    (void)mass;
                    double intensity = sql.get_column_value< double >( row++ );
                    double amount = sql.get_column_value< double >( row++ );
                    data->xy.push_back( QPointF( intensity, amount ) );
                    calib_data_[ uuid ] = data;
                }
            }
            plot_calib_curve_yx( calibplot_.get(), *calib, *data );
        }
    }
}

void
QuanResultWnd::plot_calib_curve_xy( adwplot::Dataplot* plot, const detail::calib_curve& calib, const detail::calib_data& data )
{
    plot->setAxisTitle( QwtPlot::xBottom, "response" );
    plot->setAxisTitle( QwtPlot::yLeft, "amounts" );

    plot->setTitle( QString::fromStdWString( calib.description ) );

    curves_.clear();
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto curve = curves_.back();

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	QPen pen( Qt::red );
	curve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse ), Qt::NoBrush, pen, QSize(5, 5) ) );
	curve->setPen( pen );
    curve->setStyle( QwtPlotCurve::NoCurve );
    curve->setSamples( data.xy );
    curve->attach( plot );

    ///////////////// plot regression ///////////////
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto regression = curves_.back();

    QVector< QPointF > xy;

    if ( calib.coeffs.size() == 1 ) {
        xy.push_back( QPointF( 0, 0 ) );
        xy.push_back( QPointF( calib.max_x, calib.max_x * calib.coeffs[ 0 ] ) );
    }
    else if (calib.coeffs.size() == 2) {
        double y0 = adportable::polfit::estimate_y( calib.coeffs, 0 );
        double y1 = adportable::polfit::estimate_y( calib.coeffs, calib.max_x );
        xy.push_back( QPointF( 0, y0 ) );
        xy.push_back( QPointF( calib.max_x, y1 ) );
    }
    else if ( calib.coeffs.size() >= 3 ) {
        for ( int i = 0; i < 100; ++i ) {
            double x = (calib.max_x * i / 100);
            double y = adportable::polfit::estimate_y( calib.coeffs, x );
            xy.push_back( QPointF( x, y ));
        }
    }
    regression->setSamples( xy );
    regression->attach( plot );

    plot->replot();
}

void
QuanResultWnd::plot_calib_curve_yx( adwplot::Dataplot* plot, const detail::calib_curve& calib, const detail::calib_data& data )
{
    plot->setAxisTitle( QwtPlot::xBottom, "amounts" );
    plot->setAxisTitle( QwtPlot::yLeft, "response" );

    plot->setTitle( QString::fromStdWString( calib.description ) );

    curves_.clear();
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto curve = curves_.back();

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	QPen pen( Qt::red );
	curve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse ), Qt::NoBrush, pen, QSize(5, 5) ) );
	curve->setPen( pen );
    curve->setStyle( QwtPlotCurve::NoCurve );
    do {
        QVector< QPointF > yx;
        for ( auto& xy: data.xy )
            yx.push_back( QPointF( xy.y(), xy.x() ) );
        curve->setSamples( yx ); 
    } while(0);

    curve->attach( plot );

    ///////////////// plot regression ///////////////
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto regression = curves_.back();

    QVector< QPointF > yx;

    if ( calib.coeffs.size() == 1 ) {
        yx.push_back( QPointF( 0, 0 ) );
        yx.push_back( QPointF( calib.max_x * calib.coeffs[ 0 ], calib.max_x ) );
    }
    else if (calib.coeffs.size() == 2) {
        double y0 = adportable::polfit::estimate_y( calib.coeffs, 0 );
        double y1 = adportable::polfit::estimate_y( calib.coeffs, calib.max_x );
        yx.push_back( QPointF( y0, 0 ) );
        yx.push_back( QPointF( y1, calib.max_x ) );
    }
    else if ( calib.coeffs.size() >= 3 ) {
        for ( int i = 0; i < 100; ++i ) {
            double x = (calib.max_x * i / 100);
            double y = adportable::polfit::estimate_y( calib.coeffs, x );
            yx.push_back( QPointF( y, x ));
        }
    }
    regression->setSamples( yx );
    regression->attach( plot );

    plot->replot();
}
