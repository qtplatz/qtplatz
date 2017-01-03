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

#include "quanresultwnd.hpp"
#include "quancmpdwidget.hpp"
#include "quanconnection.hpp"
#include "quandocument.hpp"
#include "quanplot.hpp"
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
#include <adplot/plot.hpp>
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
#include <workaround/boost/uuid/uuid.hpp>
#include <iomanip>
#include <sstream>

using namespace quan;

QuanResultWnd::~QuanResultWnd()
{
}

QuanResultWnd::QuanResultWnd(QWidget *parent) : QWidget(parent)
                                              , cmpdWidget_( new QuanCmpdWidget )
                                              , respTable_( new QuanResultWidget )
                                              , calibplot_( new adplot::plot )
                                              , dplot_( new QuanPlotWidget( 0, false ) )
                                              , cplot_( new QuanPlotWidget( 0, true ) )
                                              , isCounting_( false )
                                              , isISTD_( false )
{
    QwtPlotGrid * grid = new QwtPlotGrid;
    grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
    grid->attach( calibplot_.get() );

    calibplot_->setMinimumHeight( 80 );
    dplot_->setMinimumHeight( 80 );
    cplot_->setMinimumHeight( 40 );

    Core::MiniSplitter * splitter = new Core::MiniSplitter;// compound-table | plots
    
    splitter->setOrientation( Qt::Horizontal );
    
    // compound-table on left
    if ( Core::MiniSplitter * wndSplitter = new Core::MiniSplitter ) {
        splitter->addWidget( wndSplitter ); // <<------------ add to splitter

        wndSplitter->setOrientation( Qt::Vertical );  // horizontal line
        wndSplitter->addWidget( respTable_ );
        
        if ( Core::MiniSplitter  * splitter2 = new Core::MiniSplitter ) { // left pane split top (table) & bottom (time,mass plot)
            splitter2->setOrientation( Qt::Horizontal );        // Caibration curve plot | Mass spectrum plot
            wndSplitter->addWidget( splitter2 );
            
            splitter2->addWidget( calibplot_.get() );
            splitter2->addWidget( dplot_.get() );
        }
        wndSplitter->addWidget( cplot_.get() ); // Chromatogram
    }
    splitter->addWidget( cmpdWidget_ );
    splitter->setStretchFactor( 0, 5 );
    splitter->setStretchFactor( 1, 1 );
    
    auto layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( splitter );

    connect( QuanDocument::instance(), &QuanDocument::onConnectionChanged, this, &QuanResultWnd::handleConnectionChanged );
    //cmpdWidget_->table().setSelectionMode( QAbstractItemView::MultiSelection );
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

        isCounting_ = isISTD_ = false;
        adfs::stmt sql( connection->db() );
        if ( sql.prepare( "SELECT isCounting, isISTD FROM QuanMethod LIMIT(1)" ) ) {
            while( sql.step() == adfs::sqlite_row ) {
                isCounting_ = bool( sql.get_column_value< int64_t >( 0 ) );
                isISTD_ = bool( sql.get_column_value< int64_t >( 1 ) );
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
        if ( ( publisher = std::make_shared< QuanPublisher >() ) ) {
            if ( (*publisher)(QuanDocument::instance()->connection()) )
                QuanDocument::instance()->publisher( publisher );
            else
                return;
        }
    }
    
    if ( auto curve = publisher->find_calib_curve( uuid ) ) {
        QuanPlot plot( curves_, markers_ );
        plot.plot_calib_curve_yx( calibplot_.get(), *curve );
        // plot::calib_curve_yx( calibplot_.get(), *curve, curves_ ); //, *calib_curves_[ uuid ], *calib_data_[ uuid ] );
    }
}

void
QuanResultWnd::handleResponseSelected( int respId )
{
    if ( auto conn = QuanDocument::instance()->connection() ) {

        auto publisher = QuanDocument::instance()->publisher();
        if ( !publisher ) {
            if ( ( publisher = std::make_shared< QuanPublisher >() ) ) {
                if ( (*publisher)( conn ) )
                    QuanDocument::instance()->publisher( publisher );
                else
                    return;
            }
        }

        std::string query;
        if ( isCounting_ && isISTD_ ) {
            query =
                "SELECT idCmpd, r1.CountRate/r2.CountRate as intensity, amount FROM"
                " (SELECT QuanResponse.idCmpd, idSample, intensity/trigCounts as CountRate, amount"
                "  FROM QuanResponse,QuanCompound WHERE uuid=idCmpd AND QuanResponse.id = ?) r1"
                " LEFT JOIN"
                " (SELECT idSample, intensity/trigCounts as CountRate from QuanResponse,QuanCompound WHERE uuid=idCmpd AND isISTD=1) r2"
                " ON r1.idSample = r2.idSample";
        } else {
            query = "SELECT idCmpd, intensity, amount FROM QuanResponse WHERE id = ?";
        }

        adfs::stmt sql( conn->db() );
        if ( sql.prepare( query ) ) {

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

                        QuanPlot plot( curves_, markers_ );

                        if ( uuid_plot_ != uuid ) {
                            plot.plot_calib_curve_yx( calibplot_.get(), *calib );
                        }
                        plot.plot_response_marker_yx( calibplot_.get(), intensity, amount, std::make_pair( calib->min_x, calib->max_x ) );
                    }
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
        if ( sql.prepare( "SELECT dataGuid,idx,fcn,dataSource "
                          "FROM QuanSample, QuanResponse "
                          "WHERE QuanResponse.id = ? AND QuanSample.id = QuanResponse.idSample" ) ) {
            sql.bind( 1 ) = respId;
            if ( sql.step() == adfs::sqlite_row ) {
                dataGuid = sql.get_column_value< std::wstring >( 0 );
                idx = size_t( sql.get_column_value< uint64_t >( 1 ) );
                fcn = int( sql.get_column_value< int64_t >( 2 ) );
                dataSource = sql.get_column_value< std::wstring >( 3 );
            }
        }
        
        if ( !dataGuid.empty() ) {
            if ( auto d = conn->fetch( dataGuid ) ) {
                //ADDEBUG() << "setData 1( idx=" << idx << ", fcn=" << fcn << ", dataSource=" << dataSource;
                dplot_->setData( d, idx, fcn, dataSource );
                cplot_->setData( d, idx, fcn, dataSource );
            }
            if ( sql.prepare( "SELECT refDataGuid,idx,fcn FROM QuanDataGuids WHERE dataGuid = ?" ) ) {
                sql.bind( 1 ) = dataGuid;
                while ( sql.step() == adfs::sqlite_row ) {
                    auto refDataGuid = sql.get_column_value< std::wstring >( 0 );
                    auto idx = sql.get_column_value< uint64_t >( 1 );
                    auto fcn = sql.get_column_value< uint64_t >( 2 );
                    if ( auto d = conn->fetch( refDataGuid ) ) {
                        //ADDEBUG() << "setData 2( idx=" << idx << ", fcn=" << fcn << ", dataSource=" << dataSource;
                        dplot_->setData( d, idx, fcn, dataSource );
                        cplot_->setData( d, idx, fcn, dataSource );
                    }
                }
            }

        }

    }
}

