// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include "mainwindow.hpp"
#include "qtwidgets_name.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/baselines.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_same.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/peaktable.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>

#include <coreplugin/minisplitter.h>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QFileDialog>
#include <QShortcut>
#include <QMessageBox>
#include <QMenu>
#include <QPrinter>
#include <QSvgGenerator>
#include <deque>
#include <memory>

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
                                    , dirty_( false ) {

            using adwidgets::PeakTable;

            std::for_each( plots_.begin(), plots_.end(), [&]( auto& plot ){
                    plot = std::make_unique< adplot::ChromatogramWidget >();
                    plot->setMinimumHeight( 80 );
                    plot->setItemLegendEnabled( true );
                    plot->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
                });

            plots_[ 0 ]->link( plots_[ 1 ].get() );

            auto shortcut = new QShortcut( QKeySequence::Copy, p );
            connect( shortcut, &QShortcut::activatedAmbiguously, this, &impl::copy );
            connect( peakTable_, static_cast<void(PeakTable::*)(int)>(&PeakTable::currentChanged), this, &impl::handleCurrentChanged );
            connect( plots_[0].get(), qOverload< const QRectF& >(&adplot::ChromatogramWidget::onSelected), this, &impl::selectedOnChromatogram0 );
            connect( plots_[1].get(), qOverload< const QRectF& >(&adplot::ChromatogramWidget::onSelected), this, &impl::selectedOnChromatogram1 );

            marker_->attach( plots_[ 0 ].get() );
            marker_->visible( true );
            marker_->setYAxis( QwtPlot::yLeft );

        }

        void setData( adcontrols::ChromatogramPtr& ptr ) {
            data_ = ptr;
            plots_[ 0 ]->clear();
            plots_[ 0 ]->setData( ptr );
            auto title = adcontrols::Chromatogram::make_folder_name( ptr->getDescriptions() );
            plots_[ 0 ]->setTitle( QString::fromStdWString( title ) );
            peakResult_.reset();
            if ( ptr->peaks().size() )
                peakResult_ = std::make_shared< adcontrols::PeakResult >( ptr->baselines(), ptr->peaks(), ptr->isCounting() );
        }

        void setData( adcontrols::PeakResultPtr& ptr ) {
            peakResult_ = ptr;
            plots_[ 0 ]->setData( *ptr );
            peakTable_->setData( *ptr );
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

        void addPeak( double t1, double t2 ) {
            if ( !peakResult_ )
                peakResult_ = std::make_shared< adcontrols::PeakResult >();

            if ( data_ && data_->add_manual_peak( *peakResult_, t1, t2 ) ) {
                setData( peakResult_ );
                plots_[ 0 ]->update();
            }
        }

        void selectedOnChromatogram( const QRectF&, int );
        void selectedOnChromatogram0( const QRectF& );
        void selectedOnChromatogram1( const QRectF& );

        void redraw();

        ChromatogramWnd * this_;
        std::array< std::unique_ptr< adplot::ChromatogramWidget >, 2 > plots_;
        adwidgets::PeakTable * peakTable_;
        std::unique_ptr< adplot::PeakMarker > marker_;
        adcontrols::ChromatogramPtr data_;
        adcontrols::PeakResultPtr peakResult_;
        std::wstring idActiveFolium_;
        std::deque< datafolder > overlays_;
        bool dirty_;
    public slots:
        void copy() {
            peakTable_->handleCopyToClipboard();
        }
    };

    //----------------------------//
    template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
        Wnd& wnd_;
        selProcessed( Wnd& wnd ) : wnd_(wnd) {}
        template<typename T> void operator ()( T& ) const { }
        void operator () ( adutils::PeakResultPtr& ptr ) const {  wnd_.draw( ptr );   }
        void operator () ( adutils::ChromatogramPtr& ptr ) const {  wnd_.draw( ptr );   }
    };

    // template<class Wnd> class selChanged : public boost::static_visitor<bool> {
    //     Wnd& wnd_;
    // public:
    //     selChanged( Wnd& wnd ) : wnd_(wnd) { }
    //     template< typename T >
    //     bool operator()( T& ) { return false; };
    // };

    // template< typename T > struct is_same : public boost::static_visitor< bool > {
    //     template<typename U> bool operator()( U& ) const { return false; };
    //     bool operator()( T& ) const { return true; };
    // };

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
ChromatogramWnd::draw1( adutils::MassSpectrumPtr& )
{
}

void
ChromatogramWnd::draw2( adutils::MassSpectrumPtr& )
{
}

void
ChromatogramWnd::draw( adutils::ChromatogramPtr& ptr )
{
    impl_->setData( ptr );
}

void
ChromatogramWnd::draw( adutils::PeakResultPtr& ptr )
{
    impl_->setData( ptr );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * processor )
{
    if ( auto folder = processor->portfolio().findFolder( L"Chromatograms" ) ) {
        for ( auto& folium: folder.folio() ) {

            if ( folium.attribute( L"isChecked" ) == L"true" ) {

                if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                    auto it = std::find_if( impl_->overlays_.begin(), impl_->overlays_.end()
                                            , [&](const auto& a){ return a.id() == folium.uuid(); });
                    if ( it == impl_->overlays_.end() ) {
                        impl_->overlays_.emplace_back( datafolder( processor->filename(), folium ) );
                        impl_->dirty_ = true;
                    }
                }
            }
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelChromatogram )
        return;

    if ( impl_->dirty_ )
        impl_->redraw();
}

void
ChromatogramWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
}

void
ChromatogramWnd::handleProcessed( Dataprocessor* , portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( selProcessed<ChromatogramWnd>(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), contents );
    }
}

void
ChromatogramWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

    if ( ! boost::apply_visitor( adportable::is_same< adutils::ChromatogramPtr >(), data ) )
        return;

    auto datum = datafolder( processor->filename(), folium );

    if ( auto chr = datum.get_chromatogram() ) {
        // if ( auto chr = boost::get< adutils::ChromatogramPtr >( data ) ) { // current selection
        impl_->idActiveFolium_ = folium.id();
        auto& plot = impl_->plots_[ 0 ];

        plot->clear();
        plot->setTitle( datum.display_name() );
        plot->setData( chr, 0, QwtPlot::yLeft );
        if ( auto label = chr->axisLabel( adcontrols::plot::yAxis ) ) {
            plot->setAxisTitle( QwtPlot::yLeft, QwtText( QString::fromStdString( *label ) ) );
        } else {
            plot->setAxisTitle( QwtPlot::yLeft, QwtText( "Intensity (a.u.)" ) );
        }

        portfolio::Folio attachments = folium.attachments();
        for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
            adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
            boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), contents );
        }
    } else {
        return;
    }

    auto it = std::find_if( impl_->overlays_.begin(), impl_->overlays_.end(), [&]( auto& a ){ return folium.id() == a.idFolium_; } );

    if ( folium.attribute( L"isChecked" ) == L"false" ) {
        if ( it != impl_->overlays_.end() ) {
            impl_->overlays_.erase( it );
            impl_->dirty_ = true;
        }
    } else {
        if ( it == impl_->overlays_.end() ) {
            if ( auto chr = datum.get_chromatogram() ) {
                impl_->overlays_.emplace_front( datum );
                impl_->dirty_ = true;
            }
        }
    }

    if ( impl_->dirty_ )
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
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );

	portfolio::Folium folium;
    printer.setDocName( "QtPlatz Chromatogram Report" );
	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        folium = dp->getPortfolio().findFolium( impl_->idActiveFolium_ );
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
        const adcontrols::descriptions& desc = impl_->data_->getDescriptions();
        for ( size_t i = 0; i < desc.size(); ++i ) {
            const adcontrols::description& d = desc[i];
            if ( ! std::string( d.xml() ).empty() ) {
                formattedMethod.append( d.xml() );
            }
        }
    }
    drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
    drawRect.setHeight( printer.height() - drawRect.top() );
    QFont font = painter.font();
    font.setPointSize( 8 );
    painter.setFont( font );
    painter.drawText( drawRect, Qt::TextWordWrap, formattedMethod, &boundingRect );

}

///////////////////////////

void
ChromatogramWnd::impl::selectedOnChromatogram( const QRectF& rect, int index )
{
    double x0 = plots_[ index ]->transform( QwtPlot::xBottom, rect.left() );
	double x1 = plots_[ index ]->transform( QwtPlot::xBottom, rect.right() );

    typedef std::pair < QAction *, std::function<void()> > action_type;

    QMenu menu;
    std::vector < action_type > actions;

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {

        actions.emplace_back( menu.addAction( QString( "Area in range %1 - %2" ).arg( QString::number( rect.left() ), QString::number( rect.right() ) ) )
                           , [=]() { addPeak( adcontrols::Chromatogram::toSeconds( rect.left() ), adcontrols::Chromatogram::toSeconds( rect.right() ) ); } );

    }

    actions.push_back( std::make_pair( menu.addAction( tr("Copy image to clipboard") ), [&] () {
                adplot::plot::copyToClipboard( this->plots_[ index ].get() );
            } ) );

    actions.push_back( std::make_pair( menu.addAction( tr( "Save SVG File" ) ) , [&] () {
                QString name = QFileDialog::getSaveFileName( MainWindow::instance()
                                                             , "Save SVG File"
                                                             , MainWindow::makePrintFilename( idActiveFolium_, L"_" )
                                                             , tr( "SVG (*.svg)" ) );
                if ( ! name.isEmpty() )
                    adplot::plot::copyImageToFile( plots_[ index ].get(), name, "svg" );
            }) );


    if ( auto selected = menu.exec( QCursor::pos() ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [selected] ( const action_type& a ){ return a.first == selected; } );
        if ( it != actions.end() )
            (it->second)();
    }

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
    if ( overlays_.empty() ) {
        plots_[ 1 ]->hide();
    } else {
        auto& plot = plots_[ 1 ];

        plot->clear();
        int idx(0);
        for ( auto& datum: overlays_ ) {
            if ( auto chr = datum.get_chromatogram() ) {
                if ( overlays_.size() == 1 ) {
                    plot->setData( chr, 0, QwtPlot::yLeft );
                    if ( auto label = chr->axisLabel( adcontrols::plot::yAxis ) )
                        plot->setAxisTitle( QwtPlot::yLeft, QwtText( QString::fromStdString( *label ) ) );
                } else {
                    if ( ! datum.overlayChromatogram_ ) {
                        if ((datum.overlayChromatogram_ = std::make_shared< adcontrols::Chromatogram >( *chr ) ) ) {
                            datum.overlayChromatogram_->setBaselines( adcontrols::Baselines() ); // clear baselines
                            datum.overlayChromatogram_->setPeaks( adcontrols::Peaks() );         // clear peaks
                            double yMax = chr->getMaxIntensity() - chr->getMinIntensity();
                            for ( size_t i = 0; i < chr->size(); ++i ) {
                                datum.overlayChromatogram_->setIntensity( i, ( 100. * ( chr->intensity( i ) - chr->getMinIntensity() ) / yMax ) );
                            }
                        }
                    }
                    plot->setData( datum.overlayChromatogram_, idx, QwtPlot::yLeft );
                    // plot->setLegend( idx, QwtText( datum.display_name() ) );
                    plot->setAxisTitle( QwtPlot::yLeft, QwtText( "Intensity (R.A.)" ) );
                    ++idx;
                }
            }
        }
        dirty_ = false;
        plot->show();
    }

}

/////////

#include "chromatogramwnd.moc"
