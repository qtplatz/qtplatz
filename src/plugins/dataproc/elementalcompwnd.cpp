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

#include "datafolder.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "elementalcompwnd.hpp"
#include "mainwindow.hpp"
#include "sessionmanager.hpp"
#include <adutils/constants.hpp> // clsid for massspectrometer
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/timeutil.hpp>
#include <adlog/logger.hpp>
#include <adplot/peakmarker.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/polfit.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportfolio/folium.hpp>
#include <adutils/processeddata.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_plot.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adwidgets/scanlawdialog.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <QPrinter>

namespace dataproc {
    class ElementalCompWnd::impl {
    public:

        impl( ElementalCompWnd * p ) : isTimeAxis_( false )
                                     , dirty_( false )
                                     , scaleYAuto_( true )
                                     , scaleY_{ 0, 0 } {

            for ( size_t i = 0; i < plots_.size(); ++i ) {
                plots_[ i ] = std::make_unique< adplot::SpectrumWidget >();
                plots_[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
                plots_[ i ]->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
                markers_[ i ] = std::make_unique< adplot::PeakMarker >();
            }
        }

        ~impl() {
        }

        std::map< std::wstring /* folium (profile) Guid (attGuid) */, datafolder  > dataIds_;

        // std::pair< std::wstring, datafolder > profile_;
        std::array< std::unique_ptr< adplot::SpectrumWidget >, 3 > plots_; // profile,processed,reference
        std::array< std::unique_ptr< adplot::PeakMarker >, 3 > markers_;
        bool isTimeAxis_;
        bool dirty_;
        bool scaleYAuto_;
        std::pair< double, double > scaleY_;
        std::wstring idSpectrumFolium_;

        enum idSpectrumWidget { idProfile, idProcessed, idReference };

        template< idSpectrumWidget T > adplot::SpectrumWidget * splot() {
            return plots_.at( T ).get();
        }
        adplot::SpectrumWidget * spectrumWidget( idSpectrumWidget id ) { return plots_.at( id ).get(); }

    private:

    };
}

using namespace dataproc;

ElementalCompWnd::~ElementalCompWnd()
{
    delete impl_;
}

ElementalCompWnd::ElementalCompWnd(QWidget *parent) : QWidget(parent)
                                                    , impl_( new impl( this ) )
{
    init();
}

void
ElementalCompWnd::init()
{
    if ( auto splitter = new Core::MiniSplitter ) {

        for ( size_t i = 0; i < impl_->plots_.size(); ++i ) {
            auto& plot = impl_->plots_[ i ];
            auto& marker = impl_->markers_[ i ];

            connect( plot.get()
                     , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                     , [&plot,this,i]( const QRectF& rc ) { handleSelected( rc, plot.get(), i ); } );

            // plot->enableAxis( QwtPlot::yRight );
            plot->setMinimumHeight( 80 );
            marker->attach( plot.get() );
            marker->visible( true );
            // marker->setYAxis( QwtPlot::yRight );

            if ( i )
                impl_->plots_[ 0 ]->link( plot.get() );

            splitter->addWidget( plot.get() );
        }

        splitter->setOrientation( Qt::Vertical );

        QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
        toolBarAddingLayout->setMargin(0);
        toolBarAddingLayout->setSpacing(0);
        toolBarAddingLayout->addWidget( splitter );
    }
}

void
ElementalCompWnd::onInitialUpdate()
{
    if ( auto w = MainWindow::instance() ) {
        connect( w, &MainWindow::onScaleYChanged, this
                 , [&]( bool autoScale, double base, double height ) {
                     ADDEBUG() << "autoScale: " << autoScale << ", " << std::make_pair( base, height );
                     impl_->scaleYAuto_ = autoScale;
                     impl_->scaleY_ = { base, height };
                     if ( autoScale )
                         impl_->splot< impl::idProfile >()->setYScale( 0, 0, false );
                     else
                         impl_->splot< impl::idProfile >()->setYScale( base + height, base, false );
                     impl_->splot< impl::idProfile >()->replotYScale();
                 });
    }
}

void
ElementalCompWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    impl_->splot< impl::idReference >()->setData( ptr, 0 );
}

void
ElementalCompWnd::estimateScanLaw( const QString& model_name )
{
    std::shared_ptr< adcontrols::MassSpectrum > ptr;
    return ;
    auto annots = ptr->get_annotations();
    std::vector< std::pair< int, std::string >  > ids;
    std::for_each( annots.begin(), annots.end(), [&ids] ( const adcontrols::annotation& a ) {
            if ( a.dataFormat() == adcontrols::annotation::dataFormula && a.index() >= 0 )
                ids.push_back( std::make_pair( a.index(), a.text() ) );
        });

    std::vector< std::pair< double, double > > time_mass_array;
    for ( auto& id : ids ) {
        double time = ptr->getTime( id.first );
        auto sformula = adcontrols::ChemicalFormula::split( id.second );
        double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( sformula ).first;
        time_mass_array.push_back( std::make_pair( time, exactMass ) );
    }

    if ( time_mass_array.empty() )
        return;

    adwidgets::ScanLawDialog dlg;
    if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( model_name.toStdString().c_str() ) ) {
        dlg.setScanLaw( *spectrometer->scanLaw() );
        double fLength, accVoltage, tDelay, mass;
        QString formula;

        if ( document::instance()->findScanLaw( model_name, fLength, accVoltage, tDelay, mass, formula ) ) {
            dlg.setValues( fLength, accVoltage, tDelay, 0 );
            dlg.setMass( mass );
            if ( !formula.isEmpty() )
                dlg.setFormula( formula );
        }
    }

    dlg.setData( time_mass_array );

    if ( dlg.exec() != QDialog::Accepted )
        return;

    document::instance()->saveScanLaw( model_name
                                                , dlg.fLength()
                                                , dlg.acceleratorVoltage()
                                                , dlg.tDelay()
                                                , dlg.mass(), dlg.formula() );
}

void
ElementalCompWnd::handleAxisChanged( unsigned int axis )
{
    using adplot::SpectrumWidget;
    using namespace adcontrols;

    auto __axis = axis == hor_axis_mass ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime;
    for ( auto& plot: impl_->plots_ )
        plot->setAxis( __axis, true );
}

void
ElementalCompWnd::handleSessionAdded( Dataprocessor * )
{
}

void
ElementalCompWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
ElementalCompWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

    if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

        std::wstring display_name = processor->file()->filename() + L"::" + folium.name();

        auto xit = impl_->dataIds_.find( folium.id() );
        datafolder xdata = ( xit == impl_->dataIds_.end() ) ? datafolder( int( impl_->dataIds_.size() ), display_name, folium ) : xit->second;

        if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            impl_->idSpectrumFolium_ = folium.id();
            xdata.profile_ = profile;

            portfolio::Folio atts = folium.attachments();
            auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );
            if ( itCentroid != atts.end() ) {
                xdata.idCentroid_ = itCentroid->id();
                xdata.centroid_ = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
            }
            impl_->dataIds_[ folium.id() ] = xdata;
        }

        if ( MainWindow::instance()->curPage() == MainWindow::idSelElementalComp )
            draw( 0 );

        if ( folium.attribute( L"isChecked" ) == L"false" ) {

            auto it = impl_->dataIds_.find( folium.id() );
            if ( it != impl_->dataIds_.end() )
                impl_->dataIds_.erase( it );

        }

    }
}

void
ElementalCompWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
ElementalCompWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
	printer.setOrientation( QPrinter::Landscape );

    printer.setDocName( "QtPlatz isotope simulation report" );
    printer.setOutputFileName( pdfname );
    // printer.setResolution( resolution );

    QPainter painter( &printer );

    QRectF boundingRect;
    QRectF drawRect( printer.resolution()/2, printer.resolution()/2, printer.width() - printer.resolution(), (12.0/72)*printer.resolution() );

    QString fullpath;
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        fullpath = processor->qfilename();
    }

	painter.drawText( drawRect, Qt::TextWordWrap, fullpath, &boundingRect );

    drawRect.setTop( boundingRect.bottom() );
    drawRect.setHeight( printer.height() - boundingRect.top() - printer.resolution()/2 );
    //drawRect.setWidth( size.width() );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    QRectF rc( drawRect );
	rc.setHeight( drawRect.height() / 4 );
    rc.setWidth( drawRect.width() * 0.6 );

    renderer.render( impl_->splot< impl::idReference >(), &painter, rc );
    rc.moveTo( rc.left(), rc.bottom() );
    renderer.render( impl_->splot< impl::idProcessed >(), &painter, rc );
}

void
ElementalCompWnd::draw( int which )
{
    impl_->plots_[ impl::idProfile ]->clear();  // profile
    impl_->plots_[ impl::idProcessed ]->clear();  // centroid

    QString title;

    for ( auto& data: impl_->dataIds_ ) {
        int idx = data.second.idx_;
        int traceid = idx * 2;

        if ( title.isEmpty() ) {
            title = data.second.display_name();
        } else {
            title += " .";
        }

        QColor color = impl_->plots_[ 0 ]->index_color( idx );

        if ( auto profile = data.second.profile_.lock() ) {
            if ( auto plot = impl_->splot< impl::idProfile >() ) {
                plot->setData( profile, traceid );
                plot->setColor( traceid, color );
                plot->show();
            }
        }

        if ( auto centroid = data.second.centroid_.lock() ) {
            if ( auto plot = impl_->splot< impl::idProcessed >() ) {
                plot->setData( centroid, traceid + 1, true );
                plot->setColor( traceid + 1, color );
                plot->show();
            }
        }

    }

    impl_->plots_[ impl::idProfile ]->setTitle( title );
}

void
ElementalCompWnd::setSimulatedSpectrum( std::shared_ptr< const adcontrols::MassSpectrum > ms )
{
    impl_->splot< impl::idReference >()->setData( ms, 0 );
}

//////////////////////////////////////////

void
ElementalCompWnd::handleSelected( const QRectF& rc, adplot::SpectrumWidget * plot, int id )
{
    auto d = std::abs( plot->transform( QwtPlot::xBottom, rc.left() ) - plot->transform( QwtPlot::xBottom, rc.right() ) );

    if ( d <= 2 ) {

		QMenu menu;
        typedef std::pair < QAction *, std::function<void()> > action_type;
        std::vector < action_type > actions;

        menu.addAction( tr("Copy image to clipboard"), [=](){ adplot::plot::copyToClipboard( plot ); } );
        menu.addAction( tr("Save as SVG File"), [=](){
            QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File"
                                                         , MainWindow::makePrintFilename( impl_->idSpectrumFolium_, L"_" )
                                                         , tr( "SVG (*.svg)" ) );
            if ( ! name.isEmpty() ) {
                adplot::plot::copyImageToFile( plot, name, "svg" );
                MainWindow::addPrintFileToSettings( name );
            }
        });

        if ( id == impl::idProcessed ) {
            menu.addAction( tr( "Dismiss" ), [&](){ impl_->splot< impl::idProcessed >()->hide(); } );
        }
        if ( id == impl::idProfile ) {
            menu.addAction( tr( "Dismiss" ), [&](){ impl_->splot< impl::idProfile >()->hide(); } );
        }

        //--------------
        std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
        for ( auto model : models ) {
            auto a = menu.addAction( QString( "Estimate scan law based on %1" ).arg( QString::fromStdWString( model ) )
                                     , [=](){
                                         estimateScanLaw( QString::fromStdWString( model ) );
                                     } );
            // if ( !centroid_.lock() )
            a->setEnabled( false );
        }

        //-------------------------//

        menu.exec( QCursor::pos() );
    }

}
