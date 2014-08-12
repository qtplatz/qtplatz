/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "mscalibrationwnd.hpp"
#include "mainwindow.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include "qtwidgets_name.hpp"
#include "mass_calibrator.hpp"
#include "sessionmanager.hpp"
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/computemass.hpp>
#include <adwidgets/mscalibratesummarytable.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adwplot/peakmarker.hpp>
#include <adwplot/plot_stderror.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/utf.hpp>
#include <qtwrapper/font.hpp>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QPrinter>
#include <qmessagebox.h>
#include <adportable/configuration.hpp>
#include <adlog/logger.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/xml_serializer.hpp> // for quick print

#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>

#include <boost/any.hpp>
#include <boost/format.hpp>
#include <stack>
#include <tuple>

using namespace dataproc;

namespace dataproc {

    enum { idx_profile = 9, idx_centroid = 0 };

    class MSCalibrationWndImpl {
    public:
        ~MSCalibrationWndImpl() {}
        MSCalibrationWndImpl( QWidget * parent ) : processedSpectrum_( new adwplot::SpectrumWidget( parent ) )
                                                 , summaryTable_( 0 )
                                                 , centroid_marker_( std::make_shared< adwplot::PeakMarker >() )
                                                 , timeAxis_( false ) {

            processedSpectrum_->enableAxis( QwtPlot::yRight ); // enable y-axis for profile overlay
            centroid_marker_->attach( processedSpectrum_ );
            centroid_marker_->setYAxis( QwtPlot::yRight );
        }

        adwplot::SpectrumWidget * processedSpectrum_;
        adwidgets::MSCalibrateSummaryTable * summaryTable_;
        std::shared_ptr< adwplot::PeakMarker > centroid_marker_;
        std::weak_ptr< adcontrols::MassSpectrum > calibCentroid_;
        std::weak_ptr< adcontrols::MSCalibrateResult > calibResult_;
        std::weak_ptr< adcontrols::MSPeakInfo > peakInfo_;
        adwplot::plot_stderror plot_stderror_;

        portfolio::Folium folium_;
        bool timeAxis_;

        void restore_state( adcontrols::MassSpectrum& ) {
            centroid_marker_->visible( false );
        }

        void store_state( adcontrols::MassSpectrum& centroid, size_t fcn, size_t idx ) {
            centroid_marker_->visible( true );

            if ( std::shared_ptr< adcontrols::MSPeakInfo > pkInfo = peakInfo_.lock() ) {
                using namespace adcontrols::metric;

                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segments( *pkInfo );
                const adcontrols::MSPeakInfoItem& pk = *(segments[ fcn ].begin() + idx);
                centroid_marker_->setPeak( pk, timeAxis_, adcontrols::metric::micro );
            } else {
                adcontrols::segment_wrapper<> segments( centroid );
                double mass = segments[ fcn ].getMass( idx );
                centroid_marker_->setValue( adwplot::PeakMarker::idPeakCenter, mass, 0 );
            }
        }
    };

    struct draw_stderror {
    };
}

MSCalibrationWnd::MSCalibrationWnd( QWidget * parent ) : QWidget( parent )
{
    init();
}

void
MSCalibrationWnd::init()
{
    pImpl_ = std::make_shared< MSCalibrationWndImpl >( this );

    pImpl_->plot_stderror_.title( "&delta;<i>m/z</i>" );

    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        // spectrum on top
        splitter->addWidget( pImpl_->processedSpectrum_ );

        // summary table
        pImpl_->summaryTable_ = new adwidgets::MSCalibrateSummaryTable; //adplugin::widget_factory::create( L"qtwidgets2::MSCalibSummaryWidget" );
        if ( auto pSummary = pImpl_->summaryTable_ ) {

            typedef adwidgets::MSCalibrateSummaryTable ST;
            connect( pSummary, static_cast<void(ST::*)(size_t, size_t)>(&ST::currentChanged), this, &MSCalibrationWnd::handleSelSummary );
            connect( pSummary, &ST::valueChanged, this, &MSCalibrationWnd::handleValueChanged );
            connect( pSummary, &ST::on_recalibration_requested, this, &MSCalibrationWnd::handle_recalibration_requested );
            connect( pSummary, &ST::on_reassign_mass_requested, this, &MSCalibrationWnd::handle_reassign_mass_requested );
            connect( pSummary, &ST::on_apply_calibration_to_dataset, this, &MSCalibrationWnd::handle_apply_calibration_to_dataset );
            connect( pSummary, &ST::on_apply_calibration_to_default, this, &MSCalibrationWnd::handle_apply_calibration_to_default );
            connect( pSummary, &ST::on_add_selection_to_peak_table, this, &MSCalibrationWnd::handle_add_selection_to_peak_table );

            // Make a connection to zoomer in order to sync table in visible range
            connect( &pImpl_->processedSpectrum_->zoomer(), &adwplot::Zoomer::zoomed, pSummary, &ST::handle_zoomed );

            typedef adwplot::SpectrumWidget SW;
            connect( pImpl_->processedSpectrum_, static_cast<void(SW::*)(const QRectF&)>(&SW::onSelected), pSummary, &ST::handle_selected );


            adplugin::LifeCycleAccessor accessor( pSummary );
            adplugin::LifeCycle * p = accessor.get();
            if ( p )
                p->OnInitialUpdate();

            connect( this
                     , SIGNAL( onSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) )
                     , pSummary, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );

            splitter->addWidget( pSummary );
        }

        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );

    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSCalibrationWnd::handleSessionAdded( Dataprocessor * )
{
}

void
MSCalibrationWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSCalibrationWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    Q_UNUSED(processor);
    using portfolio::Folium;
    using portfolio::Folio;
	
    portfolio::Folder folder = folium.getParentFolder();
    
    if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibCentroid_.lock() )
        pImpl_->restore_state( *centroid );

    if ( folder && folder.name() == L"MSCalibration" ) {
        
        pImpl_->folium_ = folium;

        pImpl_->processedSpectrum_->clear();
        pImpl_->peakInfo_.reset();
        pImpl_->calibCentroid_.reset();
        // pImpl_->calibProfile_.reset();

        portfolio::Folio attachments = folium.attachments();

        auto it = std::find_if( attachments.begin(), attachments.end(), []( portfolio::Folium& f ){
                return f.name() == Constants::F_CENTROID_SPECTRUM;
            });
        if ( it != attachments.end() ) {
            pImpl_->calibCentroid_ = portfolio::get< adcontrols::MassSpectrumPtr >( *it );
            if ( auto fpki 
                 = portfolio::find_first_of( it->attachments(), []( portfolio::Folium& a ){
                         return portfolio::is_type< std::shared_ptr< adcontrols::MSPeakInfo > >( a ); } 
                     ) ) {
                pImpl_->peakInfo_ = portfolio::get< std::shared_ptr< adcontrols::MSPeakInfo > >( fpki );
            }
        }
        
        // calib result
        if ( auto fcalibResult = portfolio::find_first_of( attachments, []( portfolio::Folium& f ){
                    return portfolio::is_type< adcontrols::MSCalibrateResultPtr >(f);
                }) ) {
            pImpl_->calibResult_ = portfolio::get< adcontrols::MSCalibrateResultPtr >( fcalibResult );
            
            if ( const adcontrols::ProcessMethodPtr method = Dataprocessor::findProcessMethod( fcalibResult ) )
                MainWindow::instance()->setProcessMethod( *method );
        }
        
        auto result = pImpl_->calibResult_.lock();
        auto centroid = pImpl_->calibCentroid_.lock();

        if ( result && centroid )
            emit onSetData( *result, *centroid );
        
        if ( centroid )
            pImpl_->processedSpectrum_->setData( centroid, 1 );

        if ( result && centroid ) {
            QVector< QPointF > errors;
            adcontrols::ComputeMass< adcontrols::ScanLaw > mass_calculator( centroid->scanLaw(), result->calibration() );

            for ( auto a: result->assignedMasses() ) {
                double mass = mass_calculator( a.time(), a.mode() );
                errors.push_back( QPointF( a.exactMass(), ( mass - a.exactMass() ) * 1000 ) );
            }
            pImpl_->plot_stderror_( errors, *pImpl_->processedSpectrum_ );
            pImpl_->processedSpectrum_->replot();
        }
    }
}

void
MSCalibrationWnd::handleAxisChanged( int axis )
{
    pImpl_->timeAxis_ = ( axis == adwplot::SpectrumWidget::HorizontalAxisTime );
	pImpl_->processedSpectrum_->setAxis( static_cast< adwplot::SpectrumWidget::HorizontalAxis >( axis ) );

    // replot profile
    boost::any& data = pImpl_->folium_;
    if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( data ) ) { 
        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( data );
        pImpl_->processedSpectrum_->setData( ptr, idx_profile, true ); // on yRight axis
    }

    if ( auto centroid = pImpl_->calibCentroid_.lock() )
        pImpl_->processedSpectrum_->setData( centroid, idx_centroid );
}

void
MSCalibrationWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSCalibrationWnd::handleSelSummary( size_t idx, size_t fcn )
{
	if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibCentroid_.lock() ) {
        pImpl_->restore_state( *centroid );
        pImpl_->store_state( *centroid, fcn, idx );
        pImpl_->processedSpectrum_->setData( centroid, 1 );
    }
}

bool
MSCalibrationWnd::readCalibSummary( adcontrols::MSAssignedMasses& assigned )
{
    adplugin::LifeCycleAccessor accessor( pImpl_->summaryTable_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        std::shared_ptr< adcontrols::MSAssignedMasses > ptr( new adcontrols::MSAssignedMasses );
        boost::any any( ptr );
        if ( p->getContents( any ) ) {
            assigned = *ptr;
            return true;
        }
    }
    return false;
}

bool
MSCalibrationWnd::calibPolynomialFit( adcontrols::MSCalibrateResult& calibResult, const adcontrols::MSProperty& prop )
{
    adcontrols::ProcessMethod pm;
    const adcontrols::MSCalibrateMethod * pCalibMethod = 0;
    MainWindow::instance()->getProcessMethod( pm );
    if ( ! ( pCalibMethod = pm.find< adcontrols::MSCalibrateMethod >() ) )
        return false;

    // recalc polinomials
	mass_calibrator calibrator( calibResult.assignedMasses(), prop );
        
    unsigned int nterm = pCalibMethod->polynomialDegree() + 1;
    if ( calibrator.size() < nterm )
        nterm = static_cast<int>( calibrator.size() );
    
    adcontrols::MSCalibration calib;
    if ( calibrator.polfit( calib, nterm ) ) {

        calibResult.calibration( calib );
        
        return true;
    }

    return false;
}

void
MSCalibrationWnd::handleValueChanged()
{
    std::shared_ptr< adcontrols::MSCalibrateResult > calibResult = pImpl_->calibResult_.lock();
    if ( ! calibResult )
        return;

    std::shared_ptr< const adcontrols::MassSpectrum > calibSpectrum = pImpl_->calibCentroid_.lock();
    if ( ! calibSpectrum )
        return;

    const adcontrols::MSProperty& prop = calibSpectrum->getMSProperty();

    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {

        calibResult->assignedMasses( assigned );

        if ( calibPolynomialFit( *calibResult, prop ) ) {
            pImpl_->processedSpectrum_->setFooter( calibResult->calibration().formulaText() );
        }

        if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibCentroid_.lock() ) {
            
            if ( DataprocHandler::doAnnotateAssignedPeaks( *centroid, assigned ) )
                pImpl_->processedSpectrum_->setData( centroid, 1 ); 
            
            emit onSetData( *calibResult, *centroid );
        }

    }
}


void
MSCalibrationWnd::handle_reassign_mass_requested()
{
    std::shared_ptr< adcontrols::MSCalibrateResult > calibResult = pImpl_->calibResult_.lock();
    if ( ! calibResult )
        return;
	std::shared_ptr< adcontrols::MassSpectrum > calibSpectrum = pImpl_->calibCentroid_.lock();
	if ( ! calibSpectrum )
		return;

	// const adcontrols::massspectrometer::ScanLaw& scanLaw = calibSpectrum->scanLaw();

    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {

        calibResult->assignedMasses( assigned );

        if ( calibPolynomialFit( *calibResult, calibSpectrum->getMSProperty() ) ) {
            const adcontrols::MSCalibration& calib = calibResult->calibration();

            // update m/z for MSAssignedMasses
            for ( auto& it : calibResult->assignedMasses() )
				it.mass ( calib.compute_mass( it.time() ) ); // / scanLaw.fLength( it.mode() ) ) );

            if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibCentroid_.lock() ) {

                // profile
                adcontrols::MassSpectrumPtr profile = boost::any_cast< adcontrols::MassSpectrumPtr >( pImpl_->folium_.data() );
                if ( profile ) {
                    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *profile );
                    for ( auto& ms: segments ) {
						ms.setCalibration( calib );
                        for ( size_t i = 0; i < ms.size(); ++i )
                            ms.setMass( i, calib.compute_mass( ms.getTime( i ) ) ); //getNormalizedTime( i ) ) );
                    }
                }
                pImpl_->processedSpectrum_->setData( profile, idx_profile, true ); 

				// centroid (override profile that has better visibility)
                adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *centroid );
                for ( auto& ms: segments ) {
					ms.setCalibration( calib );
                    for ( size_t i = 0; i < ms.size(); ++i )
						ms.setMass( i, calib.compute_mass( ms.getTime( i ) ) ); //ms.getNormalizedTime( i ) ) );
                }
                if ( std::shared_ptr< adcontrols::MSPeakInfo > pkInfo = pImpl_->peakInfo_.lock() ) {
                    adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segments( *pkInfo );
                    for ( auto& info: segments ) {
                        for ( adcontrols::MSPeakInfoItem& item: info ) {
							double mass = calib.compute_mass( item.time() );
							double centroid_left = calib.compute_mass( item.centroid_left( true ) );
							double centroid_right = calib.compute_mass( item.centroid_right( true ) );
							double hhL = calib.compute_mass( item.hh_left_time() );
							double hhR = calib.compute_mass( item.hh_right_time() );
							item.assign_mass( mass, centroid_left, centroid_right, hhL, hhR );
                        }
                    }
				}

                // update annotation 
                DataprocHandler::doAnnotateAssignedPeaks( *centroid, assigned );

                pImpl_->processedSpectrum_->setData( centroid, idx_centroid ); 

                emit onSetData( *calibResult, *centroid );
            }
        }
    }
}

void
MSCalibrationWnd::handle_add_selection_to_peak_table( const adcontrols::MSPeaks& peaks )
{
    MainWindow::instance()->handle_add_mspeaks( peaks );
}


void
MSCalibrationWnd::handle_recalibration_requested()
{
    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) )
        MainWindow::instance()->applyCalibration( assigned );
}

void
MSCalibrationWnd::handle_apply_calibration_to_dataset()
{
    auto result = pImpl_->calibResult_.lock();
    auto ms = pImpl_->calibCentroid_.lock();

    if ( result && ms ) {
        if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
            std::wstring clsid = adportable::utf::to_wstring( ms->getMSProperty().dataInterpreterClsid() );
            processor->applyCalibration( clsid, *result );
        } else {
            QMessageBox::warning( 0, "Dataproc", "apply calibration to dataset: no active dataset" );
        }
    } else {
        QMessageBox::warning( 0, "Dataproc", "apply calibration to dataset: has no calibration" );
    }

}

void
MSCalibrationWnd::handle_apply_calibration_to_default()
{
    handle_reassign_mass_requested();
	Dataprocessor::saveMSCalibration( pImpl_->folium_ );
}

void
MSCalibrationWnd::handlePrintCurrentView( const QString& pdfname )
{
    const portfolio::Folium& folium = pImpl_->folium_;
    if ( ! folium )
        return;

    adcontrols::MSCalibrateResultPtr calibResult;
    do {
        portfolio::Folio attachments = folium.attachments();
        portfolio::Folio::iterator it = portfolio::Folium::find<adcontrols::MSCalibrateResultPtr>( attachments.begin(), attachments.end() );
        if ( it != attachments.end() )
            calibResult = boost::any_cast< adcontrols::MSCalibrateResultPtr >( *it );
    } while ( 0 );

	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    //QSizeF sizeMM( 180, 80 );
	QSizeF sizeMM( 260, 160 );

    int resolution = 300;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;
    double margin_left = 0.2 /* inch */ * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
	printer.setOrientation( QPrinter::Landscape );
    
    printer.setDocName( "QtPlatz Calibration Report" );
    printer.setOutputFileName( pdfname );
    // printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setResolution( resolution );

    QPainter painter( &printer );
    
    QRectF boundingRect;
    QRectF drawRect( margin_left, 0.0, printer.width(), (18.0/72)*resolution );
    
    painter.drawText( drawRect, Qt::TextWordWrap, folium.fullpath().c_str(), &boundingRect );
    
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
    
    drawRect.setTop( boundingRect.bottom() );
    drawRect.setHeight( size.height() );
    drawRect.setWidth( size.width() );
    renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );
    // ---------- spectrum rendered ----------
    
    // ---------- calibratin equation ----------
    if ( calibResult ) {
        const adcontrols::MSCalibration& calib = calibResult->calibration();
        QString text = "Calibration eq.: sqrt(m/z) = ";
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            if ( i == 0 )
                text += ( boost::format( "%.14le" ) % calib.coeffs()[0] ).str().c_str();
            else
                text += ( boost::format( "\t+ %.14le * x^%d" ) % calib.coeffs()[i] % i ).str().c_str();
        }
        drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
        drawRect.setHeight( printer.height() - drawRect.top() );
        QFont font = painter.font();
		qtwrapper::font::setSize( font, qtwrapper::fontSizeSmall );
        painter.setFont( font );
        painter.drawText( drawRect, Qt::TextWordWrap, text );
    }
    // ---------- end calibration equestion -----
    
    if ( connect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                  , pImpl_->summaryTable_, SLOT(handlePrint(QPrinter&, QPainter&)) ) ) {
        emit onPrint( printer, painter );
        bool res = disconnect( this, SIGNAL( onPrint(QPrinter&, QPainter&) ), pImpl_->summaryTable_, SLOT(handlePrint(QPrinter&, QPainter&)) );
        assert( res );
    }
}
