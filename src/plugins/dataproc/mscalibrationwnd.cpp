/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <adwplot/spectrumwidget.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/utf.hpp>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QPrinter>
#include <qmessagebox.h>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
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
                                                  , calibSummaryWidget_( 0 )
                                                  , centroid_center_( new QwtPlotMarker )
                                                  , centroid_left_( new QwtPlotMarker )
                                                  , centroid_right_( new QwtPlotMarker )
                                                  , centroid_threshold_( new QwtPlotMarker )
                                                  , centroid_baselevel_( new QwtPlotMarker )
                                                  , timeAxis_( false ) {

            centroid_center_->setLineStyle( QwtPlotMarker::VLine );
            centroid_center_->setLinePen( Qt::darkGray, 0, Qt::DashDotLine );
            centroid_center_->attach( processedSpectrum_ );

            centroid_left_->setLineStyle( QwtPlotMarker::VLine );
            centroid_left_->setLinePen( Qt::darkGray, 0, Qt::DotLine );
            centroid_left_->attach( processedSpectrum_ );

            centroid_right_->setLineStyle( QwtPlotMarker::VLine );
            centroid_right_->setLinePen( Qt::darkGray, 0, Qt::DotLine );
            centroid_right_->attach( processedSpectrum_ );

            centroid_threshold_->setLineStyle( QwtPlotMarker::HLine );
            centroid_threshold_->setLinePen( Qt::darkGray, 0, Qt::DashDotLine );
            centroid_threshold_->attach( processedSpectrum_ );

            centroid_baselevel_->setLineStyle( QwtPlotMarker::HLine );
            centroid_baselevel_->setLinePen( Qt::darkGray, 0, Qt::DotLine );
            centroid_baselevel_->attach( processedSpectrum_ );
        }

        adwplot::SpectrumWidget * processedSpectrum_;
        QWidget * calibSummaryWidget_;

        std::unique_ptr< QwtPlotMarker > centroid_center_;
        std::unique_ptr< QwtPlotMarker > centroid_left_;
        std::unique_ptr< QwtPlotMarker > centroid_right_;
        std::unique_ptr< QwtPlotMarker > centroid_threshold_;
        std::unique_ptr< QwtPlotMarker > centroid_baselevel_;

        std::weak_ptr< adcontrols::MassSpectrum > calibCentroid_;
        std::weak_ptr< adcontrols::MassSpectrum > calibProfile_;
        std::weak_ptr< adcontrols::MSCalibrateResult > calibResult_;
        std::weak_ptr< adcontrols::MSPeakInfo > peakInfo_;

        portfolio::Folium folium_;
        bool timeAxis_;

        void restore_state( adcontrols::MassSpectrum& ) {
            centroid_center_->setLinePen( Qt::transparent, 0, Qt::DashDotLine );
        }

        void store_state( adcontrols::MassSpectrum& centroid, size_t fcn, size_t idx ) {

            centroid_center_->setLinePen( Qt::green, 0, Qt::DashDotLine );

            if ( std::shared_ptr< adcontrols::MSPeakInfo > pkInfo = peakInfo_.lock() ) {
                using namespace adcontrols::metric;

                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segments( *pkInfo );
                const adcontrols::MSPeakInfoItem& pk = *(segments[ fcn ].begin() + idx);
                // todo: axis time/mass 
                if ( timeAxis_ ) {
                    centroid_center_->setValue( adcontrols::metric::scale_to_micro( pk.time() ), 0 );
                    centroid_left_->setValue( adcontrols::metric::scale_to_micro( pk.centroid_left( true ) ), 0 );
                    centroid_right_->setValue( adcontrols::metric::scale_to_micro( pk.centroid_right( true ) ), 0 );
                } else {
                    centroid_center_->setValue( pk.mass(), 0 );
                    centroid_left_->setValue( pk.centroid_left(), 0 );
                    centroid_right_->setValue( pk.centroid_right(), 0 );
                }
                centroid_threshold_->setValue( 0, pk.centroid_threshold() );
                centroid_baselevel_->setValue( 0, pk.base_height() );
            } else {
                adcontrols::segment_wrapper<> segments( centroid );
                double mass = segments[ fcn ].getMass( idx );
                centroid_center_->setValue( mass, 0 );
            }
        }
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

    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        // spectrum on top
        splitter->addWidget( pImpl_->processedSpectrum_ );

        // summary table
        pImpl_->calibSummaryWidget_ = adplugin::widget_factory::create( L"qtwidgets2::MSCalibSummaryWidget" );
        if ( QWidget * pSummary = pImpl_->calibSummaryWidget_ ) {
            bool res;
            res = connect( pSummary, SIGNAL( currentChanged( size_t, size_t ) ), this, SLOT( handleSelSummary( size_t, size_t ) ) );
            assert(res);
            res = connect( pSummary, SIGNAL( valueChanged() ), this, SLOT( handleValueChanged() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_recalibration_requested() ), this, SLOT( handle_recalibration_requested() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_reassign_mass_requested() ), this, SLOT( handle_reassign_mass_requested() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_apply_calibration_to_dataset() ), this, SLOT( handle_apply_calibration_to_dataset() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_apply_calibration_to_default() ), this, SLOT( handle_apply_calibration_to_default() ) );
            assert(res);
            
            res = connect( pSummary, SIGNAL( on_add_selection_to_peak_table(const adcontrols::MSPeaks& ))
                           , this, SLOT( handle_add_selection_to_peak_table( const adcontrols::MSPeaks& ) ) );
            assert(res);

            // Make a connection to zoomer in order to sync table in visible range
            res = connect( &pImpl_->processedSpectrum_->zoomer()
                           , SIGNAL( zoomed( const QRectF& ) ), pSummary, SLOT( handle_zoomed( const QRectF& ) ) );
            assert(res);

            res = connect( pImpl_->processedSpectrum_
                           , SIGNAL( onSelected( const QRectF& ) ), pSummary, SLOT( handle_selected( const QRectF& ) ) );
            assert(res);
            (void)res;

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
        pImpl_->calibProfile_.reset();

        portfolio::Folio attachments = folium.attachments();

        // draw profile spectrum
        if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( folium.data() ) ) { 

            auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium );

            auto it = std::find_if( attachments.begin(), attachments.end(), []( portfolio::Folium& f ){
                    return f.name() == Constants::F_DFT_FILTERD;
                });

			QSize sz = pImpl_->processedSpectrum_->canvas()->size();

            if ( it != attachments.end() ) {
                // Select DFT Filterd if exists
                pImpl_->calibProfile_ = portfolio::get< adcontrols::MassSpectrumPtr> ( *it );
                pImpl_->processedSpectrum_->setData( pImpl_->calibProfile_.lock(), idx_profile );

            } else {

                pImpl_->processedSpectrum_->setData( ptr, idx_profile );
                if ( ptr->isCentroid() ) {
                    pImpl_->calibCentroid_ = ptr;
                    pImpl_->processedSpectrum_->setTitle( folium.name() );
                } else {
                    pImpl_->calibProfile_ = ptr;
                }
            }
			sz = pImpl_->processedSpectrum_->canvas()->size();
        }

        if ( ! pImpl_->calibCentroid_.lock() ) {
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
        pImpl_->processedSpectrum_->setData( ptr, idx_profile );
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
    adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
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
        
    size_t nterm = pCalibMethod->polynomialDegree() + 1;
    if ( calibrator.size() < nterm )
        nterm = calibrator.size();
    
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
                pImpl_->processedSpectrum_->setData( profile, idx_profile ); 

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
    QMessageBox::information( 0, "MSCalibrationWnd", "apply calibration to dataset not implementd" );
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
    
    // ---------- calibratin equeatin ----------
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
        font.setPointSize( 8 );
        painter.setFont( font );
        painter.drawText( drawRect, Qt::TextWordWrap, text );
    }
    // ---------- end calibration equestion -----
    
    if ( connect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                  , pImpl_->calibSummaryWidget_, SLOT(handlePrint(QPrinter&, QPainter&)) ) ) {
        emit onPrint( printer, painter );
        bool res = disconnect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                               , pImpl_->calibSummaryWidget_, SLOT(handlePrint(QPrinter&, QPainter&)) );
        assert( res );
    }
}
