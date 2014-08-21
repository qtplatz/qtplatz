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
#include "quancmpdwidget.hpp"
#include "quanconnection.hpp"
#include "quandocument.hpp"
#include "quanplotwidget.hpp"
#include "quanpublisher.hpp"
#include "quanquery.hpp"
#include "quanresultwidget.hpp"
#include "quanresulttable.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
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
#include <QColor>
#include <boost/uuid/uuid.hpp>
#include <iomanip>
#include <sstream>

namespace quan {

    struct plot : QObject {
        static void calib_curve_yx( adwplot::Dataplot *, const QuanPublisher::calib_curve&, std::vector< std::shared_ptr< QwtPlotCurve > >& );
    };

}

using namespace quan;

QuanResultWnd::~QuanResultWnd()
{
}

QuanResultWnd::QuanResultWnd(QWidget *parent) : QWidget(parent)
                                              , cmpdWidget_( new QuanCmpdWidget )
                                              , respTable_( new QuanResultWidget )
                                              , calibplot_( new adwplot::Dataplot )
                                              , dplot_( new QuanPlotWidget )
{
    QwtPlotGrid * grid = new QwtPlotGrid;
    grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
    grid->attach( calibplot_.get() );

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
            splitter2->addWidget( dplot_.get() );
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
    cmpdWidget_->table().setSelectionMode( QAbstractItemView::MultiSelection );
    cmpdWidget_->table().setSelectionBehavior( QAbstractItemView::SelectRows );
    connect( &cmpdWidget_->table(), &QuanResultTable::onCurrentChanged, this, &QuanResultWnd::handleCompoundSelected );
    connect( cmpdWidget_->table().selectionModel(), &QItemSelectionModel::selectionChanged, this, &QuanResultWnd::handleCompoundSelectionChanged );
    connect( respTable_, &QuanResultWidget::onResponseSelected, this, &QuanResultWnd::handleResponseSelected );
}

void
QuanResultWnd::handleConnectionChanged()
{
    if ( auto connection = QuanDocument::instance()->connection() ) {
        respTable_->setConnection ( connection );

        // make compounds table (right bar)
        if ( auto query = connection->query() ) {
            if ( query->prepare( std::wstring ( L"SELECT uuid, formula, description FROM QuanCompound" ) ) ) {
                cmpdWidget_->table().setColumnHide( "uuid" );
                cmpdWidget_->table().prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    cmpdWidget_->table().addRecord( *query );
                }
            }
        }

    }
}

void
QuanResultWnd::handleCompoundSelectionChanged( const QItemSelection&, const QItemSelection& )
{
    std::set< boost::uuids::uuid> cmpds;

    QModelIndexList indecies = cmpdWidget_->table().selectionModel()->selectedIndexes();
    for ( auto& index : indecies )
        cmpds.insert( cmpdWidget_->uuid( index.row() ) );
    respTable_->setCompoundSelected( cmpds );
}

void
QuanResultWnd::handleCompoundSelected( const QModelIndex& index )
{
    boost::uuids::uuid uuid = cmpdWidget_->uuid( index.row() );

    auto publisher = QuanDocument::instance()->publisher();
    if ( !publisher ) {
        if ( publisher = std::make_shared< QuanPublisher >() ) {
            if ( (*publisher)(QuanDocument::instance()->connection()) )
                QuanDocument::instance()->publisher( publisher );
            else
                return;
        }
    }
    
    if ( auto curve = publisher->find_calib_curve( uuid ) ) {
        plot::calib_curve_yx( calibplot_.get(), *curve, curves_ ); //, *calib_curves_[ uuid ], *calib_data_[ uuid ] );
    }
}

void
QuanResultWnd::handleResponseSelected( int respId )
{
    if ( auto conn = QuanDocument::instance()->connection() ) {

        auto publisher = QuanDocument::instance()->publisher();
        if ( !publisher ) {
            if ( publisher = std::make_shared< QuanPublisher >() ) {
                if ( (*publisher)( conn ) )
                    QuanDocument::instance()->publisher( publisher );
                else
                    return;
            }
        }

        adfs::stmt sql( conn->db() );
        if ( sql.prepare( "SELECT idCmpd, intensity, amount from QuanResponse WHERE id = ?" ) ) {

            sql.bind( 1 ) = respId;
            while ( sql.step() == adfs::sqlite_row ) {
                int row = 0;
                try {
                    boost::uuids::uuid uuid = sql.get_column_value< boost::uuids::uuid >( row++ );
                    double intensity = sql.get_column_value< double >( row++ );
                    double amount = 0;
                    if ( !sql.is_null_column( row ) )
                        amount = sql.get_column_value< double >( row );
                    ++row;
                    
                    if ( auto calib = publisher->find_calib_curve( uuid ) ) {
                        if ( uuid_plot_ != uuid )
                            plot::calib_curve_yx( calibplot_.get(), *calib, curves_ );
                        plot_response_marker_yx( calibplot_.get(), intensity, amount );
                    }
                    // if ( calib_curves_.find( uuid ) != calib_curves_.end() && calib_data_.find( uuid ) != calib_data_.end() ) {
                    //     if ( uuid_plot_ != uuid )
                    //         plot_calib_curve_yx( calibplot_.get(), *calib_curves_[ uuid ], *calib_data_[ uuid ] );
                    //     plot_response_marker_yx( calibplot_.get(), intensity, amount );
                    // }
                }
                catch ( std::bad_cast& ) {
                    // amount can be null
                }

            }
        }
        std::wstring dataGuid;
        std::wstring dataSource;
        size_t idx;
        int fcn;
        if ( sql.prepare( "SELECT dataGuid,idx,fcn,dataSource FROM QuanSample, QuanResponse WHERE QuanResponse.id = ? AND QuanSample.id = QuanResponse.idSample" ) ) {
            sql.bind( 1 ) = respId;
            if ( sql.step() == adfs::sqlite_row ) {
                dataGuid = sql.get_column_value< std::wstring >( 0 );
                idx = size_t( sql.get_column_value< uint64_t >( 1 ) );
                fcn = int( sql.get_column_value< int64_t >( 2 ) );
                dataSource = sql.get_column_value< std::wstring >( 3 );
            }
        }
        if ( auto d = conn->fetch( dataGuid ) ) {
            dplot_->setData( d, idx, fcn, dataSource );
        }
    }
}

void
QuanResultWnd::plot_response_marker_yx( adwplot::Dataplot* plot, double intensity, double amount )
{
    markers_.clear();

    auto marker = std::make_shared< QwtPlotMarker >();
    markers_.push_back( marker );
    marker->attach ( plot );
    
    marker->setValue( amount, intensity );
    //marker->setLinePen( QColor( 0x0, 0x7f, 0, 0x80 ), 1.0, Qt::DashLine );
    marker->setLinePen( Qt::red, 0.0, Qt::DashLine );

    if ( adportable::compare<double>::approximatelyEqual( 0.0, amount ) )
        marker->setLineStyle( QwtPlotMarker::HLine );
    else
        marker->setLineStyle( QwtPlotMarker::Cross );

    plot->replot();
}

void
plot::calib_curve_yx( adwplot::Dataplot* plot
                      , const QuanPublisher::calib_curve& calib
                      , std::vector< std::shared_ptr< QwtPlotCurve > >& curves )
{
    plot->setAxisTitle( QwtPlot::xBottom, tr( "amounts" ) );
    plot->setAxisTitle( QwtPlot::yLeft, tr( "response" ) );
    plot->setTitle( QString( tr( "Calibration curve for %1" ) ).arg( QString::fromStdWString( calib.description ) ) );

    std::ostringstream o;
    o << tr( "Amounts = " ).toStdString();
    if ( calib.coeffs.size() == 1 )
        o << std::setprecision( 6 ) << calib.coeffs[ 0 ] << "&times;I";
    else if ( calib.coeffs.size() >= 2 ) { 
        o << std::setprecision( 6 ) << calib.coeffs[ 0 ] << "+" << calib.coeffs[ 1 ] << "&times;<i>I</i>";
        for ( int i = 2; i < calib.coeffs.size(); ++i )
            o << std::setprecision( 6 ) << "+" << calib.coeffs[ i ] << "&times;I<sup>" << i << "</sup>";
    }
    o << tr( "&nbsp;&nbsp;where I is the response" ).toStdString();
    
    plot->setFooter( o.str() );

    curves.clear();
    curves.push_back( std::make_shared< QwtPlotCurve >() );
    auto curve = curves.back();

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	QPen pen( Qt::red );
	curve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse ), Qt::NoBrush, pen, QSize(5, 5) ) );
	curve->setPen( pen );
    curve->setStyle( QwtPlotCurve::NoCurve );
    do {
        QVector< QPointF > yx;
        for ( auto& xy: calib.xy )
            yx.push_back( QPointF( xy.second, xy.first ) );
        curve->setSamples( yx ); 
    } while(0);

    curve->attach( plot );

    ///////////////// plot regression ///////////////
    curves.push_back( std::make_shared< QwtPlotCurve >() );
    auto regression = curves.back();

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
