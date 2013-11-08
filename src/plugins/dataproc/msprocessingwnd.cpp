/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "msprocessingwnd.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adportable/xml_serializer.hpp>
#include <adutils/processeddata.hpp>
#include <adwplot/picker.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <qtwrapper/xmlformatter.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qapplication.h>
#include <qsvggenerator.h>
#include <qprinter.h>
#include <QBoxLayout>
#include <QMenu>
#include <qclipboard.h>
#include <boost/variant.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "selchanged.hpp"
#include <sstream>

using namespace dataproc;

namespace dataproc {
    class MSProcessingWndImpl {
    public:
        ~MSProcessingWndImpl() {}
        MSProcessingWndImpl() : ticPlot_(0)
                              , profileSpectrum_(0)
                              , processedSpectrum_(0) {
        }
        
        adwplot::ChromatogramWidget * ticPlot_;
        adwplot::SpectrumWidget * profileSpectrum_;
        adwplot::SpectrumWidget * processedSpectrum_;
    };
    
    //---------------------------------------------------------
    template<class Wnd> struct selProcessed : public boost::static_visitor<bool> {
        Wnd& wnd_;
        portfolio::Folder& folder_;
        selProcessed( Wnd& wnd, portfolio::Folder& folder ) : wnd_( wnd )
                                                            , folder_( folder ) {
        }
        template<typename T> bool operator ()( T& ) const {
            adportable::debug(__FILE__, __LINE__) << "Unhandled data: " << typeid(T).name() << " received.";
			return false;
        }
        
        bool operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.idSpectrumFolium( folder_.id() );
            wnd_.draw2( ptr );
			return true;
        }
        
        bool operator () ( adutils::ChromatogramPtr& ptr ) const {
            wnd_.idChromatogramFolium( folder_.id() );
            wnd_.draw( ptr );
			return true;
        }
        
        bool operator () ( adutils::PeakResultPtr& ptr ) const {
            wnd_.draw( ptr );
			return true;
        }
        
    };
}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void
MSProcessingWnd::init()
{
    pImpl_.reset( new MSProcessingWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( ( pImpl_->ticPlot_ = new adwplot::ChromatogramWidget(this) ) ) {
            pImpl_->ticPlot_->setMinimumHeight( 80 );
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnChromatogram( const QPointF& ) ) );
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnChromatogram( const QRectF& ) ) );
        }
	
        if ( ( pImpl_->profileSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->profileSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->profileSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProfile( const QPointF& ) ) );
			connect( pImpl_->profileSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProfile( const QRectF& ) ) );
        }

        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProcessed( const QPointF& ) ) );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProcessed( const QRectF& ) ) );
        }
 
		pImpl_->ticPlot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->profileSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );

		splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );
        splitter->setOrientation( Qt::Vertical );

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

        pImpl_->processedSpectrum_->setContextMenuPolicy( Qt::CustomContextMenu );
		connect( pImpl_->processedSpectrum_, SIGNAL( customContextMenuRequested( QPoint ) )
                 , this, SLOT( handleCustomMenuOnProcessedSpectrum( const QPoint& ) ) );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSProcessingWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    pProfileSpectrum_ = ptr;
    pImpl_->profileSpectrum_->setData( ptr, drawIdx1_++ );
	std::wostringstream title;
	for ( auto text: ptr->getDescriptions() )
		title << text.text() << L", ";
	pImpl_->profileSpectrum_->setTitle( title.str() );
    pImpl_->processedSpectrum_->clear();
	drawIdx2_ = 0;
}

void
MSProcessingWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    pProcessedSpectrum_ = ptr;
    pImpl_->processedSpectrum_->setData( ptr, drawIdx2_++ );
}

void
MSProcessingWnd::draw( adutils::ChromatogramPtr& ptr )
{
    adcontrols::Chromatogram& c = *ptr;
    pImpl_->ticPlot_->setData( c );
}

void
MSProcessingWnd::draw( adutils::PeakResultPtr& ptr )
{
    pImpl_->ticPlot_->setData( *ptr );
}

void
MSProcessingWnd::idSpectrumFolium( const std::wstring& id )
{
    idSpectrumFolium_ = id;
}

void
MSProcessingWnd::idChromatogramFolium( const std::wstring& id )
{
    idChromatogramFolium_ = id;
}

void
MSProcessingWnd::handleSessionAdded( Dataprocessor * processor )
{
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    portfolio::Portfolio portfolio = processor->getPortfolio();

    if ( dset ) {
		size_t nfcn = dset->getFunctionCount();

		portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );
		if ( folder.nil() )
			folder = processor->getPortfolio().addFolder( L"Chromatograms" );

		for ( size_t fcn = 0; fcn < nfcn; ++fcn ) {
            std::wstring title = ( boost::wformat( L"TIC.%1%" ) % (fcn + 1) ).str();

			portfolio::Folium folium = folder.findFoliumByName( title );
            if ( folium.nil() ) {   // add TIC if not yet added
				adcontrols::Chromatogram c;
                if ( dset->getTIC( fcn, c ) ) {
                    if ( c.isConstantSampledData() )
                        c.getTimeArray();
                    c.addDescription( adcontrols::Description( L"origin", title ) );
                    adcontrols::ProcessMethod m;
                    MainWindow::instance()->getProcessMethod( m );
                    folium = processor->addChromatogram( c, m );
				}
            }
		}
        portfolio::Folium folium = folder.findFoliumByName( L"TIC.1" );
        if ( folium.empty() )
            processor->fetch( folium );
        handleSelectionChanged( processor, folium );
    }

    // show first spectrum on tree by default
    portfolio::Folder spectra = portfolio.findFolder( L"Spectra" );
    if ( !spectra.nil() ) {
        portfolio::Folio folio = spectra.folio();
        if ( !folio.empty() ) {
            processor->fetch( folio[0] );
            handleSelectionChanged( processor, folio[0] );
        }
    }
}


void
MSProcessingWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    drawIdx1_ = 0;
    drawIdx2_ = 0;

    portfolio::Folder folder = folium.getParentFolder();
    if ( folder && ( folder.name() == L"Spectra" || folder.name() == L"Chromatograms" ) ) {

        auto data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
        
        if ( boost::apply_visitor( selChanged<MSProcessingWnd>(*this), data ) ) {

            idActiveFolium_ = folium.id();
            
            portfolio::Folio attachments = folium.attachments();
            std::sort( attachments.begin(), attachments.end(), [](const portfolio::Folium& a, const portfolio::Folium& ) {
                    return a.name() == Constants::F_CENTROID_SPECTRUM;  // centroid result on top
                });
            for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
                auto contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
                if ( boost::apply_visitor( selProcessed<MSProcessingWnd>( *this, folder ), contents ) ) {
                    
                    if ( const adcontrols::ProcessMethodPtr method = Dataprocessor::findProcessMethod( *it ) )
                        MainWindow::instance()->setProcessMethod( *method );
				}
            }
        }
    }
}

void
MSProcessingWnd::handleAxisChanged( int axis )
{
    using adwplot::SpectrumWidget;

    axis_ = axis;
    pImpl_->profileSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime );
    pImpl_->processedSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime );
    if ( adcontrols::MassSpectrumPtr profile = pProfileSpectrum_.lock() ) {
        pImpl_->profileSpectrum_->setData( profile, 0 ); // todo, set draw index as well
    }
    if ( adcontrols::MassSpectrumPtr processed = pProcessedSpectrum_.lock() ) {
        pImpl_->processedSpectrum_->setData( processed, 0 ); // todo, set draw index as well
    }
}

void
MSProcessingWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSProcessingWnd::handleCustomMenuOnProcessedSpectrum( const QPoint& )
{
	// This is conflicting with picker's action, so it has moved to range selection slots
}

void
MSProcessingWnd::selectedOnChromatogram( const QPointF& pos )
{
    DataprocPlugin::instance()->onSelectTimeRangeOnChromatogram( pos.x(), pos.x() ); 
}

void
MSProcessingWnd::selectedOnChromatogram( const QRectF& rect )
{
    DataprocPlugin::instance()->onSelectTimeRangeOnChromatogram( rect.x(), rect.x() + rect.width() ); 
}

void
MSProcessingWnd::selectedOnProfile( const QPointF& )
{
}

void
MSProcessingWnd::selectedOnProfile( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {
        // todo: chromatogram creation by m/z|time range

	} else {
		std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
        using adportable::utf;

        if ( ! models.empty() ) {
            QMenu menu;

            std::vector< QAction * > actions;
            for ( auto model: models ) {
                actions.push_back( menu.addAction( 
                                       ( boost::format( "Re-assign masses using %1% scan law" ) % utf::to_utf8( model ) ).str().c_str() ) );
            }

            QAction * selectedItem = menu.exec( QCursor::pos() );
            auto it = std::find_if( actions.begin(), actions.end(), [selectedItem]( const QAction *item ){
                    return item == selectedItem; }
                );

            if ( it != actions.end() ) {
                const std::wstring& model_name = models[ std::distance( actions.begin(), it ) ];
                assign_masses_to_profile( model_name );
                pImpl_->profileSpectrum_->replot();
            }
        }
    }
}

void
MSProcessingWnd::selectedOnProcessed( const QPointF& pos )
{
    adportable::debug(__FILE__, __LINE__) << "MSProcessingWnd::selectedOnProcessed: " << pos.x() << ", " << pos.y();
}

void
MSProcessingWnd::selectedOnProcessed( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {
        // todo: chromatogram creation from base peak in range

        if ( auto ptr = pProcessedSpectrum_.lock() ) {
#if defined BASEPEAK_SELECTION
            // find base peak
			auto idx = adcontrols::segments_helper::base_peak_index( *ptr, rect.left(), rect.right() );
            double mass = adcontrols::segments_helper::get_mass( *ptr, idx );
			std::vector< std::tuple< int, double, double > > ranges( 1, std::make_tuple( idx.second, mass, 0.05 ) );
			Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
			DataprocessWorker::instance()->createChromatograms( processor, ranges );
#else
            std::vector< std::pair< int, int > > indecies;
            if ( adcontrols::segments_helper::selected_indecies( indecies, *ptr, rect.left(), rect.right(), rect.top() ) ) {
                std::vector< std::tuple< int, double, double > > ranges;
                for ( auto& index: indecies ) {
                    double mass = adcontrols::segments_helper::get_mass( *ptr, index );
                    ranges.push_back( std::make_tuple( index.second, mass, 0.05 ) );
                }
                Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
                DataprocessWorker::instance()->createChromatograms( processor, ranges );
            }
#endif
        }

    } else {
        QMenu menu;

        std::vector< QAction * > actions;
        actions.push_back( menu.addAction( "Copy to clipboard" ) );
        actions.push_back( menu.addAction( "Create chromatograms" ) );

        QAction * selectedItem = menu.exec( QCursor::pos() );
        auto it = std::find_if( actions.begin(), actions.end(), [selectedItem]( const QAction *item ){
                return item == selectedItem; }
            );
        if ( it != actions.end() ) {
            QRectF rc = pImpl_->processedSpectrum_->zoomRect();
            if ( *it == actions[ 0 ] ) {
                QClipboard * clipboard = QApplication::clipboard();
                QwtPlotRenderer renderer;
                QImage img( pImpl_->processedSpectrum_->size(), QImage::Format_ARGB32 );
                QPainter painter(&img);
                renderer.render( pImpl_->processedSpectrum_, &painter, pImpl_->processedSpectrum_->rect() );
                clipboard->setImage( img );
            } else if ( *it == actions[ 1 ] ) {

                if ( adcontrols::MassSpectrumPtr ptr = pProcessedSpectrum_.lock() ) {
					// create chromatograms for all peaks in current zoomed scope
					Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
					DataprocessWorker::instance()->createChromatograms( processor, ptr, rc.left(), rc.right() );
				}
            }
        }
    }
}

void
MSProcessingWnd::handlePrintCurrentView( const QString& pdfname )
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
    printer.setDocName( "QtPlatz Process Report" );
	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        folium = dp->getPortfolio().findFolium( idActiveFolium_ );
    }
    printer.setOutputFileName( pdfname );
    // printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setResolution( resolution );

    QPainter painter( &printer );

	QRectF boundingRect;
	QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*resolution );

	painter.drawText( drawRect, Qt::TextWordWrap, folium.fullpath().c_str(), &boundingRect );
	
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

	drawRect.setTop( boundingRect.bottom() );
	drawRect.setHeight( size.height() );
	drawRect.setWidth( size.width() );
	renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );

	drawRect.setTop( drawRect.bottom() );
	drawRect.setHeight( size.height() );
    renderer.render( pImpl_->profileSpectrum_, &painter, drawRect );

	QString formattedMethod;

	// if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

    // portfolio::Folium folium = dp->getPortfolio().findFolium( idActiveFolium_ );

    portfolio::Folio attachments = folium.attachments();
    portfolio::Folio::iterator it
        = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
    if ( it != attachments.end() ) {
        adutils::MassSpectrumPtr ms = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        const adcontrols::Descriptions& desc = ms->getDescriptions();
        for ( size_t i = 0; i < desc.size(); ++i ) {
            const adcontrols::Description& d = desc[i];
            if ( ! d.xml().empty() ) {
                formattedMethod.append( d.xml().c_str() ); // boost::serialization does not close xml correctly, so xmlFormatter raise an exception.
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

bool
MSProcessingWnd::assign_masses_to_profile( const std::wstring& model_name )
{
    const adcontrols::MassSpectrometer& model = adcontrols::MassSpectrometer::get( model_name.c_str() );
    const adcontrols::ScanLaw& law = model.getScanLaw();
    adportable::debug(__FILE__, __LINE__ ) << model_name;

	std::pair< double, double > mass_range;

	if ( auto x = this->pProfileSpectrum_.lock() ) {

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *x );

		for ( auto& ms: segments ) {
			for ( size_t idx = 0; idx < ms.size(); ++idx ) {
				double m = law.getMass( ms.getTime( idx ), 0 );
                ms.setMass( idx, m );
				if ( idx == 0 )
					mass_range.first = std::min( mass_range.first, m );
				if ( idx == ms.size() - 1 )
					mass_range.second = std::max( mass_range.second, m );
			}
		}
		x->setAcquisitionMassRange( mass_range.first, mass_range.second );
	}
	
	return true;
}

