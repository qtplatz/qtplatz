// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include "dataprocessor.hpp"
#include "dataprocconstants.hpp"
#include "mainwindow.hpp"
#include "selchanged.hpp"
#include "sessionmanager.hpp"
#include "qtwidgets_name.hpp"
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
#include <adutils/processeddata.hpp>
#include <adwidgets/peaktable.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <qtwrapper/qstring.hpp>
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

using namespace dataproc;

namespace dataproc {

    class ChromatogramWnd::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {
            marker_.reset();
            delete chroWidget_;
            delete peakTable_;
        }
        impl( ChromatogramWnd * p ) : QObject( p )
                                    , this_( p )
                                    , chroWidget_( new adplot::ChromatogramWidget )
                                    , peakTable_(new adwidgets::PeakTable)
                                    , marker_( std::make_shared< adplot::PeakMarker >() ) {

            using adwidgets::PeakTable;

            auto shortcut = new QShortcut( QKeySequence::Copy, p );
            connect( shortcut, &QShortcut::activatedAmbiguously, this, &impl::copy );
            connect( peakTable_, static_cast<void(PeakTable::*)(int)>(&PeakTable::currentChanged), this, &impl::handleCurrentChanged );
            connect( chroWidget_, static_cast< void( adplot::ChromatogramWidget::*)( const QRectF& ) >(&adplot::ChromatogramWidget::onSelected), this, &impl::selectedOnChromatogram );

            marker_->attach( chroWidget_ );
            marker_->visible( true );
            marker_->setYAxis( QwtPlot::yLeft );
            
        }

        void setData( adcontrols::ChromatogramPtr& ptr ) {
            data_ = ptr;
            chroWidget_->clear();
            chroWidget_->setData( ptr );
            std::wstring name = ptr->getDescriptions().make_folder_name( L"^((?!acquire\\.protocol\\.).)*$" );
            if ( !name.empty() )
                chroWidget_->setTitle( QString::fromStdWString( name ) );
            peakResult_.reset();
            if ( ptr->peaks().size() )
                peakResult_ = std::make_shared< adcontrols::PeakResult >( ptr->baselines(), ptr->peaks() );
        }

        void setData( adcontrols::PeakResultPtr& ptr ) {
            peakResult_ = ptr;
            chroWidget_->setData( *ptr );
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
                    chroWidget_->replot();
                }
            }
        }

        void addPeak( double t1, double t2 ) {
            if ( !peakResult_ )
                peakResult_ = std::make_shared< adcontrols::PeakResult >();

            if ( data_ && data_->add_manual_peak( *peakResult_, t1, t2 ) ) {
                setData( peakResult_ );
                chroWidget_->update();
            }
        }

        void selectedOnChromatogram( const QRectF& );

        ChromatogramWnd * this_;
        adplot::ChromatogramWidget * chroWidget_;
        adwidgets::PeakTable * peakTable_;
        std::shared_ptr< adplot::PeakMarker > marker_;
        adcontrols::ChromatogramPtr data_;
        adcontrols::PeakResultPtr peakResult_;
        std::wstring idActiveFolium_;
    public slots:
        void copy() {
            peakTable_->handleCopyToClipboard();
        }
    };

    //----------------------------//
    template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
        selProcessed( Wnd& wnd ) : wnd_(wnd) {}
        template<typename T> void operator ()( T& ) const {
        }
        void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.draw2( ptr );
        }
        void operator () ( adutils::ChromatogramPtr& ptr ) const {
            wnd_.draw( ptr );
        }
        void operator () ( adutils::PeakResultPtr& ptr ) const {
            wnd_.draw( ptr );
        }
        Wnd& wnd_;
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

        splitter->addWidget( impl_->chroWidget_ );
        splitter->addWidget( impl_->peakTable_ );
        splitter->setOrientation( Qt::Vertical );
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
    // impl_->chroWidget_->setData( ptr );
    // impl_->peakTable_->setData( adcontrols::PeakResult( ptr->baselines(), ptr->peaks() ) );
}

void
ChromatogramWnd::draw( adutils::PeakResultPtr& ptr )
{
    impl_->setData( ptr );
	// impl_->chroWidget_->setData( *ptr );
    // impl_->peakTable_->setData( *ptr );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * )
{
	/*
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            //impl_->setData( c, filename );
        }
    }
	*/
}

void
ChromatogramWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
}

void
ChromatogramWnd::handleProcessed( Dataprocessor* , portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( selChanged<ChromatogramWnd>(*this), data );

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

    if ( boost::apply_visitor( selChanged<ChromatogramWnd>(*this), data ) )
        impl_->idActiveFolium_ = folium.id();

    portfolio::Folio attachments = folium.attachments();

    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), contents );
    }

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
	renderer.render( impl_->chroWidget_, &painter, drawRect );

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
ChromatogramWnd::impl::selectedOnChromatogram( const QRectF& rect )
{
    double x0 = chroWidget_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = chroWidget_->transform( QwtPlot::xBottom, rect.right() );

    typedef std::pair < QAction *, std::function<void()> > action_type; 

    QMenu menu; 
    std::vector < action_type > actions;

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {

        actions.emplace_back( menu.addAction( QString( "Area in range %1 - %2" ).arg( QString::number( rect.left() ), QString::number( rect.right() ) ) )
                           , [=]() { addPeak( adcontrols::Chromatogram::toSeconds( rect.left() ), adcontrols::Chromatogram::toSeconds( rect.right() ) ); } );

    }

    actions.push_back( std::make_pair( menu.addAction( tr("Copy image to clipboard") ), [&] () {
                adplot::plot::copyToClipboard( this->chroWidget_ );
            } ) );
    
    actions.push_back( std::make_pair( menu.addAction( tr( "Save SVG File" ) ) , [&] () {
                QString name = QFileDialog::getSaveFileName( MainWindow::instance()
                                                             , "Save SVG File"
                                                             , MainWindow::makePrintFilename( idActiveFolium_, L"_" )
                                                             , tr( "SVG (*.svg)" ) );
                if ( ! name.isEmpty() )
                    adplot::plot::copyImageToFile( chroWidget_, name, "svg" );
            }) );
    
    
    if ( auto selected = menu.exec( QCursor::pos() ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [selected] ( const action_type& a ){ return a.first == selected; } );
        if ( it != actions.end() )
            (it->second)();
    }

}

/////////

#include "chromatogramwnd.moc"
