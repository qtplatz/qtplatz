// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "chromatogramwnd.hpp"
#include "datafolder.hpp"
#include "dataprocconstants.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "make_filename.hpp"
#include "mainwindow.hpp"
#include "qtwidgets_name.hpp"
#include "sessionmanager.hpp"
#include "utility.hpp"
#include <adcontrols/baselines.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peaks.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_same.hpp>
#include <adportable/is_type.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata.hpp>
#include <adutils/processeddata_t.hpp>
#include <adwidgets/peaktable.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>

#include <coreplugin/minisplitter.h>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QSvgGenerator>

#include <memory>
#include <deque>

using namespace dataproc;

namespace dataproc {

    class ChromatogramWnd::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {
            delete peakTable_;
        }

        impl( ChromatogramWnd * p ) : QObject( p )
                                    , this_( p )
                                    , peakTable_( new adwidgets::PeakTable )
                                    , marker_( std::make_unique< adplot::PeakMarker >() )
                                    , yScale_{ true, 0,  100.0, false }
                                    , xScale_{ true, 0, 1000.0, false }
                                    , dirty_( false ) {

            using adwidgets::PeakTable;
            size_t n(0);
            std::for_each( plots_.begin(), plots_.end(), [&]( auto& plot ) {
                plot = std::make_unique< adplot::ChromatogramWidget >();
                plot->setObjectName( QString("ChromatogramWnd.%1").arg( QString::number(n++) ) );
                plot->setMinimumHeight( 80 );
                plot->setItemLegendEnabled( false );
                plot->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
            });

            plots_[ 0 ]->link( plots_[ 1 ].get() );

            connect( peakTable_, static_cast<void(PeakTable::*)(int)>(&PeakTable::currentChanged), this, &impl::handleCurrentChanged );

            connect( peakTable_, &PeakTable::valueChanged, [&]( int pid, int cid, const QModelIndex& index ){
                if ( cid == 0 && index.column() == adwidgets::peaktable::c_name ) {
                    handlePeakTableChanged( pid, index.data().toString().toStdString() );
                }
            });

            connect( peakTable_, &PeakTable::peaksAboutToBeRemoved, this, &impl::handlePeaksAboutToBeRemoved );

            connect( plots_[0].get(), qOverload< const QRectF& >(&adplot::ChromatogramWidget::onSelected), this, &impl::selectedOnChromatogram0 );
            connect( plots_[1].get(), qOverload< const QRectF& >(&adplot::ChromatogramWidget::onSelected), this, &impl::selectedOnChromatogram1 );

            marker_->attach( plots_[ 0 ].get() );
            marker_->visible( true );
            marker_->setYAxis( QwtPlot::yLeft );
        }

        void setChromatogram( adcontrols::ChromatogramPtr& ptr ) {
            // clear existing data
            plots_[ 0 ]->clear();
            plots_[ 0 ]->setTitle( QString{} );
            peakResult_.reset();
            if (( data_ = ptr )) {
                plots_[ 0 ]->setData( ptr, 0, QwtPlot::yLeft );
                auto title = ptr->make_title();
                plots_[ 0 ]->setTitle( QString::fromStdString( title ) );
                if ( ptr->peaks().size() ) {
                    peakResult_ = std::make_shared< adcontrols::PeakResult >( ptr->baselines(), ptr->peaks(), ptr->isCounting() );
                }
            }
        }

        void setPeakResult( adcontrols::PeakResultPtr ptr ) {
            if (( peakResult_ = ptr )) {
                plots_[ 0 ]->setPeakResult( *ptr, QwtPlot::yLeft );
                peakTable_->setData( *ptr );
            } else { // clear
                peakTable_->setData( adcontrols::PeakResult{} );
            }
        }

        void handlePeakTableChanged( int pid, const std::string& name ) {
            if ( auto pkres = datum_.get_peakResult() ) {
                auto peaks = pkres->peaks();
                if ( peaks.size() > pid ) {
                    auto it = peaks.begin() + pid;
                    ADDEBUG() << "peak name: " << it->name() << " --> " << name << "\t" << datum_.folium_.id() << ", " << datum_.folium_.uuid();
                    it->setName( name );
                    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                        dp->setPeakName( datum_.folium_, pid, name );
                    }
                }
            }
        }

        void handlePeaksAboutToBeRemoved( const std::vector< std::pair< int, int > >& ids ) {
            std::vector< int > pids;
            std::for_each( ids.begin(), ids.end(), [&](const auto& a){ if ( a.first == 0 ) pids.emplace_back( a.second ); } );
            if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                dp->removePeaks( datum_.folium_, std::move( pids ) );
            }
        }

        void handleCurrentChanged( int peakId ) {
            using adcontrols::Peak;
            if ( peakResult_ ) {
                const auto& peaks = peakResult_->peaks();
                auto it = std::find_if( peaks.begin(), peaks.end(), [peakId] ( const Peak& pk ){
                        return pk.peakId() == peakId;
                    } );
                if ( it != peaks.end() ) {
                    marker_->setPeak( *it );
                    plots_[ 0 ]->replot();
                }
            }
        }

        void addPeak( double t1, double t2, bool horBase = false ) {
            auto data = data_ ? data_ : plots_[ 0 ]->getData( 0 );
            if ( data ) {
                if ( !peakResult_ )
                    peakResult_ = std::make_shared< adcontrols::PeakResult >();
                if ( data->add_manual_peak( *peakResult_, t1, t2, horBase, 0.0 ) ) {
                    setPeakResult( peakResult_ );
                    plots_[ 0 ]->replot();
                }
            }
        }

        void addFIPeak( double t1, double t2 ) {
            if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( datum_.id() );
                dp->findSinglePeak( folium, { t1, t2 } );
            }
        }

        void addOverlay( datafolder&& datum ) {
            while ( overlays_.size() >= 12 )
                overlays_.pop_back();
            auto it = std::remove_if( overlays_.begin(), overlays_.end()
                                      , [&](const auto& a){ return a.idFolium_ == datum.idFolium_ || a.idfolium_ == datum.idfolium_; });
            if ( it != overlays_.end() )
                overlays_.erase( it, overlays_.end() );
            overlays_.emplace_front( std::move( datum ) );
            dirty_ = true;
        }

        void eraseOverlay( const portfolio::Folium& folium ) {
            auto it = std::remove_if( overlays_.begin(), overlays_.end()
                                      , [&](const auto& a){ return a.idFolium_ == folium.id() || a.idfolium_ == folium.uuid(); });
            if ( it != overlays_.end() ) {
                overlays_.erase( it, overlays_.end() );
                dirty_ = true;
            }
        }

        void addOverlays( std::deque< datafolder >&& data ) {
            overlays_ = std::move( data );
            dirty_    = true;
        }

        void selectedOnChromatogram( const QRectF&, int );
        void selectedOnChromatogram0( const QRectF& );
        void selectedOnChromatogram1( const QRectF& );

        void redraw();

        ChromatogramWnd * this_;
        std::array< std::unique_ptr< adplot::ChromatogramWidget >, 2 > plots_; // 0 := current data; 1 := reference
        adwidgets::PeakTable * peakTable_;
        std::unique_ptr< adplot::PeakMarker > marker_;
        // plot[0] data
        adcontrols::ChromatogramPtr data_;
        adcontrols::PeakResultPtr peakResult_;
        std::pair< boost::uuids::uuid, std::wstring > selected_folder_;
        datafolder datum_; // current data <-- replacement of data_
        std::deque< datafolder > overlays_;
        std::tuple< bool, double, double, bool > yScale_;
        std::tuple< bool, double, double, bool > xScale_;
        bool dirty_;
    public slots:

    };

    //----------------------------//
    template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
        Wnd& wnd_;
        selProcessed( Wnd& wnd ) : wnd_(wnd) {}
        template<typename T> void operator ()( T& ) const {}
        void operator () ( adutils::PeakResultPtr& ptr ) const   { wnd_.draw( ptr ); }
        void operator () ( adutils::ChromatogramPtr& ptr ) const { wnd_.draw( ptr ); }
    };
}

ChromatogramWnd::~ChromatogramWnd()
{
}

ChromatogramWnd::ChromatogramWnd( QWidget *parent ) : QWidget(parent)
                                                    , impl_( new impl( this ) )
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;

    if ( splitter ) {
        splitter->addWidget( impl_->plots_[ 0 ].get() );
        splitter->addWidget( impl_->plots_[ 1 ].get() );
        splitter->addWidget( impl_->peakTable_ );
        splitter->setOrientation( Qt::Vertical );
        impl_->plots_[ 1 ]->hide();
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );

    impl_->peakTable_->OnInitialUpdate();
}

void
ChromatogramWnd::draw( adutils::ChromatogramPtr& ptr )
{
    impl_->setChromatogram( ptr );
}

void
ChromatogramWnd::draw( adutils::PeakResultPtr& ptr )
{
    impl_->setPeakResult( ptr );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * processor )
{
    if ( auto folder = processor->portfolio().findFolder( L"Chromatograms" ) ) {
        for ( auto& folium: folder.folio() ) {

            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                datafolder datum( processor->filename(), folium );
                impl_->addOverlay( std::move( datum ) );
            } else {
                impl_->eraseOverlay( folium );
            }
        }
    }
    if ( impl_->dirty_ )
        impl_->redraw();
}

void
ChromatogramWnd::handleRemoveSession( Dataprocessor * processor )
{
    auto it = std::remove_if( impl_->overlays_.begin()
                              , impl_->overlays_.end()
                              , [&](const auto& a){ return a.filename_ == processor->filename(); });
    if ( it != impl_->overlays_.end() )
        impl_->overlays_.erase( it, impl_->overlays_.end() );

    if ( impl_->datum_.filename_ == processor->filename() ) {
        impl_->data_.reset();
        impl_->peakResult_.reset();
        impl_->setChromatogram( impl_->data_ );
        impl_->setPeakResult( impl_->peakResult_ );
        impl_->datum_ = {};
    }

    impl_->redraw();
}

void
ChromatogramWnd::handleSessionRemoved( const QString& filename )
{
}

void
ChromatogramWnd::handleCheckStateChanged( Dataprocessor * dp, portfolio::Folium& folium, bool isChecked )
{
    if ( !isChecked ) {
        impl_->eraseOverlay( folium );
        impl_->redraw();
    }
}

void
ChromatogramWnd::handleProcessed( Dataprocessor* , portfolio::Folium& folium )
{
    using dataTuple = std::tuple< std::shared_ptr< adcontrols::PeakResult >
                                  , std::shared_ptr< adcontrols::Chromatogram >
                                  , std::shared_ptr< adcontrols::MassSpectrum > >;

    if ( auto var = adutils::to_variant< dataTuple >()(static_cast< boost::any& >( folium )) ) {
        boost::apply_visitor( selProcessed<ChromatogramWnd>(*this), *var );  // draw data
    } else {
        ADDEBUG() << "######## variant not found for " << static_cast< boost::any& >( folium ).type().name();
    }
    // ------>
    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        if ( auto var = adutils::to_variant< dataTuple >()(static_cast< boost::any& >( *it )) ) {
            boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), *var );
        } else {
            ADDEBUG() << "\tattachment variant not found for " << static_cast< boost::any& >( folium ).type().name();
        }
    }
    // <------
    impl_->redraw();
}

void
ChromatogramWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    try {
        using dataTuple = std::tuple< std::shared_ptr< adcontrols::Chromatogram > >;

        if ( auto var = adutils::to_variant< dataTuple >()(static_cast< boost::any& >( folium )) ) {
            boost::apply_visitor( selProcessed<ChromatogramWnd>(*this), *var );  // draw data
        } else {
            return;
        }
    } catch ( boost::exception& ex ) {
        ADDEBUG() << ex;
        for( const auto& a: folium.attributes() )
            ADDEBUG() << a;
    }

    auto datum = datafolder( processor->filename(), folium );

    if ( auto chr = datum.get_chromatogram() ) {

        impl_->selected_folder_ = { folium.uuid(), folium.id() }; // current selection

        auto& plot = impl_->plots_[ 0 ];

        plot->clear();
        plot->setTitle( datum.display_name() );
        plot->setData( chr, 0, QwtPlot::yLeft );
        if ( auto label = chr->axisLabel( adcontrols::plot::yAxis ) ) {
            plot->setAxisTitle( QwtPlot::yLeft, QwtText( QString::fromStdString( *label ) ) );
        } else {
            plot->setAxisTitle( QwtPlot::yLeft, QwtText( "Intensity (a.u.)" ) );
        }

        impl_->setPeakResult( datum.get_peakResult() );
        impl_->datum_ = datum;
#if 0
        if ( folium.attribute( L"isChecked" ) == L"false" ) {
            impl_->eraseOverlay( folium );
        } else {
            impl_->addOverlay( std::move( datum ) );
        }
#endif
        impl_->redraw();
    }
}

void
ChromatogramWnd::handleSelections( Dataprocessor* processor, const std::vector< portfolio::Folium >& folio )
{
    std::deque< datafolder > data;
    for ( const auto& folium: folio )
        data.emplace_front( datafolder{ processor->filename(), folium } );
    if ( !data.empty() )
        data.emplace_front( impl_->datum_ );

    impl_->addOverlays( std::move( data ) );
    impl_->redraw();
}

void
ChromatogramWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
ChromatogramWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QSizeF sizeMM( 180, 80 );

    int resolution = 85;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPageSize( QPageSize( QPageSize::A4 ) );
    printer.setFullPage( false );

	portfolio::Folium folium;
    printer.setDocName( "QtPlatz Chromatogram Report" );
	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        folium = dp->getPortfolio().findFolium( impl_->datum_.id() ); // std::get< 1 >( impl_->selected_folder_ ) );
        // folium = dp->getPortfolio().findFolium( std::get< 1 >( impl_->selected_folder_ ) );
    }

    printer.setOutputFileName( pdfname );
    printer.setResolution( resolution );

    QPainter painter( &printer );

	QRectF boundingRect;
	QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*printer.resolution() );

	painter.drawText( drawRect, Qt::TextWordWrap, folium.fullpath().c_str(), &boundingRect );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

	drawRect.setTop( boundingRect.bottom() );
	drawRect.setHeight( size.height() );
	drawRect.setWidth( size.width() );
	renderer.render( impl_->plots_[ 0 ].get(), &painter, drawRect );

	QString formattedMethod;

    portfolio::Folio attachments = folium.attachments();
    portfolio::Folio::iterator it
        = portfolio::Folium::find<adcontrols::ChromatogramPtr>( attachments.begin(), attachments.end() );
    if ( it != attachments.end() ) {
        adutils::MassSpectrumPtr ms = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        const adcontrols::descriptions& desc = impl_->data_->descriptions();
        for ( size_t i = 0; i < desc.size(); ++i ) {
            const adcontrols::description& d = desc[i];
#if 0
            if ( d.encode() == adcontrols::Encode_XML ) {
                formattedMethod.append( QString::fromStdString( d.text<char>() ) );
            }
#endif
        }
    }
    drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
    drawRect.setHeight( printer.height() - drawRect.top() );
    QFont font = painter.font();
    font.setPointSize( 8 );
    painter.setFont( font );
    painter.drawText( drawRect, Qt::TextWordWrap, formattedMethod, &boundingRect );
}


void
ChromatogramWnd::handleChromatogramYScale( bool checked, double bottom, double top ) const
{
    impl_->yScale_ = { checked, bottom, top, std::get< 0 >( impl_->yScale_ ) != checked };
    impl_->redraw();
}

void
ChromatogramWnd::handleChromatogramXScale( bool checked, double left, double right ) const
{
    impl_->xScale_ = { checked, left, right, std::get< 0 >( impl_->xScale_ ) != checked };
    impl_->redraw();
}

///////////////////////////

void
ChromatogramWnd::impl::selectedOnChromatogram( const QRectF& rect, int index )
{
    double x0 = plots_[ index ]->transform( QwtPlot::xBottom, rect.left() );
	double x1 = plots_[ index ]->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu;

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {
        auto rstr = QString::fromStdString( ( boost::format("%.2f - %2f") % rect.left() % rect.right() ).str() );
        menu.addAction( QString( "Find single peak (FI; DI-PTR) in %1" ).arg( rstr )
                        , [rect,this]() {
                            addFIPeak( rect.left(), rect.right() );
                        } );

        menu.addAction( QString( "Area in range %1 - %2" ).arg( rstr )
                        , [rect,this]() {
                            addPeak( rect.left(), rect.right() );
                        } );
    } else {
        auto rc = plots_[ index ]->zoomRect();
        menu.addAction( tr( "Find flow injection peak" )
                        , [rc,this]() { addFIPeak( rc.left(), rc.right() );  } );
    }

    menu.addAction( tr("Copy image to clipboard")
                    , [&] () {
                        adplot::plot::copyToClipboard( this->plots_[ index ].get() );
                    } );

    menu.addAction( tr( "Save SVG File" ), [index,this](){
        std::wstring idFolium;
        if ( index == 1 && !overlays_.empty() ) {
            idFolium = overlays_.at( 0 ).idFolium_;
        }
        if ( index == 0 && datum_ ) {
            idFolium = datum_.idFolium();
        }
        utility::save_image_as< SVG >()( plots_[ index ].get(), idFolium );
    });

    menu.addAction( tr("Low pass filter"), [&] () {
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            auto folium = dp->getPortfolio().findFolium( selected_folder_.second );
            if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                dp->dftFilter( folium, MainWindow::instance()->processMethod() );
            }
        }
    });

    menu.exec( QCursor::pos() );

}

void
ChromatogramWnd::impl::selectedOnChromatogram0( const QRectF& rect )
{
    selectedOnChromatogram(rect, 0);
}

void
ChromatogramWnd::impl::selectedOnChromatogram1( const QRectF& rect )
{
    selectedOnChromatogram(rect, 1);
}

void
ChromatogramWnd::impl::redraw()
{
    if ( std::get< 3 >( yScale_ ) || std::get< 3 >( xScale_ ) ) { // scale auto flag changed
        auto& plot = plots_[ 0 ];
        plot->setYScale( std::make_tuple( std::get<0>(yScale_),std::get<1>(yScale_),std::get<2>(yScale_)), false );
        plot->setXScale( std::make_tuple( std::get<0>(xScale_),std::get<1>(xScale_),std::get<2>(xScale_)), true );
    }

    if ( overlays_.empty() ) {

        plots_[ 1 ]->hide();

    } else {

        auto& plot = plots_[ 1 ];
        plot->clear();
        plot->setNormalizedY( QwtPlot::yLeft, std::get< 0 >( yScale_ ) && (overlays_.size() > 1) );

        int idx(0);
        for ( auto& datum: overlays_ ) {
            if ( auto chr = datum.get_chromatogram() ) {

                if ( ! datum.overlayChromatogram_ ) {
                    // copy for rescale
                    datum.overlayChromatogram_ = std::make_shared< adcontrols::Chromatogram >( *chr );
                }
                plot->setChromatogram( {idx, chr, datum.get_peakResult()}, QwtPlot::yLeft );

                if ( idx == 0 ) {
                    if ( auto label = chr->axisLabel( adcontrols::plot::yAxis ) )
                        plot->setAxisTitle( QwtPlot::yLeft, QwtText( QString::fromStdString( *label ) ) );
                }

                if (( datum.id() != datum_.id() ) && ( datum.idFolium_ != datum_.idFolium() )) { // this is not currently focused chromatogram
                    if ( auto pks = datum.get_peakResult() ) {
                        peakTable_->addData( adcontrols::PeakResult{ pks->baselines(), pks->peaks(), chr->isCounting() }, idx + 1, false );
                        datum.idx_ = idx + 1;
                    }
                }
                ++idx;
            }
        }
        plot->setAxisTitle( QwtPlot::yLeft, std::get<0>( yScale_ ) ? QwtText( "Intensity (R.A.)" ) : QwtText( "Intensity (a.u.)" ) );
        if ( std::get< 0 >( yScale_ ) ) {
            if ( ( std::get< 2 >(yScale_) - std::get< 1 >(yScale_) ) > 0.1 ) {
                plot->setAxisScale( QwtPlot::yLeft, std::get< 1 >( yScale_ ), std::get< 2 >( yScale_ ) );
            }
        }
        plot->setYScale( std::make_tuple( std::get<0>(yScale_),std::get<1>(yScale_),std::get<2>(yScale_)), false );
        plot->setXScale( std::make_tuple( std::get<0>(xScale_),std::get<1>(xScale_),std::get<2>(xScale_)), true );
        plot->show();
    }
    dirty_ = false;
    std::get< 3 >( yScale_ ) = false;
    std::get< 3 >( xScale_ ) = false;
}

/////////

#include "chromatogramwnd.moc"
