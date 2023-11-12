/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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
#include "document.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include "qtwidgets_name.hpp"
#include "mass_calibrator.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/computemass.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adlog/logger.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/plot_stderror.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportable/xml_serializer.hpp> // for quick print
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/lifecycle.hpp>
#include <adwidgets/mscalibratesummarytable.hpp>
#include <coreplugin/minisplitter.h>
#include <qtwrapper/font.hpp>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QStandardPaths>

#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <stack>
#include <tuple>

using namespace dataproc;

namespace dataproc {

    enum { idx_profile = 9, idx_centroid = 0 };

    class MSCalibrationWndImpl {
    public:
        ~MSCalibrationWndImpl() {}
        MSCalibrationWndImpl( QWidget * parent ) : processedSpectrum_( new adplot::SpectrumWidget( parent ) )
                                                 , summaryTable_( 0 )
                                                 , centroid_marker_( std::make_shared< adplot::PeakMarker >() )
                                                 , timeAxis_( false ) {

            processedSpectrum_->enableAxis( QwtPlot::yRight ); // enable y-axis for profile overlay
            centroid_marker_->attach( processedSpectrum_ );
            centroid_marker_->setYAxis( QwtPlot::yRight );
        }

        adplot::SpectrumWidget * processedSpectrum_;
        adwidgets::MSCalibrateSummaryTable * summaryTable_;
        std::shared_ptr< adplot::PeakMarker > centroid_marker_;
        std::weak_ptr< adcontrols::MassSpectrum > calibCentroid_;
        std::weak_ptr< adcontrols::MSCalibrateResult > calibResult_;
        std::weak_ptr< adcontrols::MSPeakInfo > peakInfo_;
        adplot::plot_stderror plot_stderror_;

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
                double mass = segments[ fcn ].mass( idx );
                centroid_marker_->setValue( adplot::PeakMarker::idPeakCenter, mass, 0 );
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
            // connect( pSummary, &ST::on_apply_calibration_to_all, this, &MSCalibrationWnd::handle_apply_calibration_to_all );
            connect( pSummary, &ST::on_apply_calibration_to_default, this, &MSCalibrationWnd::handle_apply_calibration_to_default );
            connect( pSummary, &ST::on_add_selection_to_peak_table, this, &MSCalibrationWnd::handle_add_selection_to_peak_table );
            connect( pSummary, &ST::exportCalibration, this, &MSCalibrationWnd::handleExportCalibration );

            // Make a connection to zoomer in order to sync table in visible range
            connect( pImpl_->processedSpectrum_->zoomer(), &adplot::Zoomer::zoomed, pSummary, &ST::handle_zoomed );
            typedef adplot::SpectrumWidget SW;
            connect( pImpl_->processedSpectrum_, static_cast<void(SW::*)(const QRectF&)>(&SW::onSelected), pSummary, &ST::handle_selected );


            if ( auto p = qobject_cast< adplugin::LifeCycle * >( pSummary ) )
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

    portfolio::Folder folder = folium.parentFolder();

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
        if ( auto fcalibResult = portfolio::find_first_of( attachments
                                                           , []( portfolio::Folium& f ){
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
            pImpl_->processedSpectrum_->setData( centroid, 1, QwtPlot::yLeft );

        if ( result && centroid ) {
            QVector< QPointF > errors;

            //boost::uuids::uuid massSpectrometerClsid;
            if ( auto processor = SessionManager::instance()->getActiveDataprocessor() ) {
                if ( auto sp = processor->massSpectrometer() ) {
                    if ( auto scanLaw = adcontrols::MassSpectrometer::make_scanlaw( sp->massSpectrometerClsid(), centroid->getMSProperty() ) ) {

                        adcontrols::ComputeMass< adcontrols::ScanLaw > mass_calculator( *scanLaw, result->calibration() );

                        for ( auto a : result->assignedMasses() ) {
                            double mass = mass_calculator( a.time(), a.mode() );
                            errors.push_back( QPointF( a.exactMass(), (mass - a.exactMass()) * 1000 ) );
                        }
                    }
                }
            } else {
                for ( auto a : result->assignedMasses() ) {
                    double mass = adcontrols::detail::compute_mass< adcontrols::MSCalibration::TIMESQUARED >::compute( a.time(), result->calibration() );
                    errors.push_back( QPointF( a.exactMass(), (mass - a.exactMass()) * 1000 ) );
                }
            }
            pImpl_->plot_stderror_( errors, *pImpl_->processedSpectrum_ );
            pImpl_->processedSpectrum_->replot();
        }
    }
}

void
MSCalibrationWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    pImpl_->timeAxis_ = ( axis == adcontrols::hor_axis_time );
	pImpl_->processedSpectrum_->setAxis( static_cast< adplot::SpectrumWidget::HorizontalAxis >( axis ) );

    // replot profile
    boost::any& data = pImpl_->folium_;
    if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( data ) ) {
        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( data );
        pImpl_->processedSpectrum_->setData( ptr, idx_profile, QwtPlot::yRight ); // on yRight axis
    }

    if ( auto centroid = pImpl_->calibCentroid_.lock() )
        pImpl_->processedSpectrum_->setData( centroid, idx_centroid, QwtPlot::yLeft );
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
        pImpl_->processedSpectrum_->setData( centroid, 1, QwtPlot::yLeft );
    }
}

bool
MSCalibrationWnd::readCalibSummary( adcontrols::MSAssignedMasses& assigned )
{
    if ( auto p = qobject_cast< adplugin::LifeCycle * >( pImpl_->summaryTable_ ) ) {
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
    if ( ! ( pCalibMethod = pm.find< adcontrols::MSCalibrateMethod >() ) ) {
        return false;
    }

    // recalc polinomials
	mass_calibrator calibrator( calibResult.assignedMasses(), prop );

    unsigned int nterm = pCalibMethod->polynomialDegree() + 1;
    if ( calibrator.size() < nterm )
        nterm = static_cast<int>( calibrator.size() );

    boost::uuids::uuid massSpectrometerClsid{{0}};
    if ( auto processor = SessionManager::instance()->getActiveDataprocessor() ) {
        if ( auto sp = processor->massSpectrometer() )
            massSpectrometerClsid = sp->massSpectrometerClsid();
    }

    adcontrols::MSCalibration calib( massSpectrometerClsid );

    if ( calibrator.polfit( calib, nterm ) ) {

        calibResult.setCalibration( calib );

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
                pImpl_->processedSpectrum_->setData( centroid, 1, QwtPlot::yLeft );

            emit onSetData( *calibResult, *centroid );
        }

    }
}


void
MSCalibrationWnd::handle_reassign_mass_requested()
{
    std::shared_ptr< adcontrols::MSCalibrateResult > calibResult = pImpl_->calibResult_.lock();
    if ( ! calibResult ) {
        return;
    }

	std::shared_ptr< adcontrols::MassSpectrum > calibSpectrum = pImpl_->calibCentroid_.lock();
	if ( ! calibSpectrum ) {
		return;
    }

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
                            ms.setMass( i, calib.compute_mass( ms.time( i ) ) ); //getNormalizedTime( i ) ) );
                    }
                }
                pImpl_->processedSpectrum_->setData( profile, idx_profile, QwtPlot::yRight );

				// centroid (override profile that has better visibility)
                adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *centroid );
                for ( auto& ms: segments ) {
					ms.setCalibration( calib );
                    for ( size_t i = 0; i < ms.size(); ++i )
						ms.setMass( i, calib.compute_mass( ms.time( i ) ) ); //ms.getNormalizedTime( i ) ) );
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
                            item.set_mass( mass, centroid_left, centroid_right );
                            item.set_width_hh_lr( hhL, hhR, false );
                        }
                    }
				}

                // update annotation
                DataprocHandler::doAnnotateAssignedPeaks( *centroid, assigned );

                pImpl_->processedSpectrum_->setData( centroid, idx_centroid, QwtPlot::yLeft );

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

        ADDEBUG() << "### " << __FUNCTION__ << " TBD";
        assert( result->calibration().massSpectrometerClsid() != boost::uuids::uuid{{0}} );

        if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
            processor->applyCalibration( *result );
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
    std::filesystem::path path = QStandardPaths::locate( QStandardPaths::ConfigLocation, "QtPlatz", QStandardPaths::LocateDirectory ).toStdString();
    path /= "default.msclb";
	Dataprocessor::MSCalibrationSave( pImpl_->folium_, QString::fromStdString( path.string() ) );
}

void
MSCalibrationWnd::handleExportCalibration()
{
    if ( auto processor = SessionManager::instance()->getActiveDataprocessor() ) {
        handle_reassign_mass_requested();

        std::filesystem::path path( processor->filename() );
        QString dir( QString::fromStdString( path.parent_path().string() ) );

        QString file =
            QFileDialog::getSaveFileName( 0
                                          , tr( "Export Mass Calibration" )
                                          , dir
                                          , tr( "MS Calibration Files(*.msclb)" ) );
        if ( ! file.isEmpty() ) {
            Dataprocessor::MSCalibrationSave( pImpl_->folium_, file );
        }
    }
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
    //printer.setPaperSize( QPrinter::A4 );
    printer.setPageSize( QPageSize( QPageSize::A4 ) );
    printer.setFullPage( false );
	//printer.setOrientation( QPrinter::Landscape );
    printer.setPageOrientation( QPageLayout::Landscape );

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
        painter.setFont( qtwrapper::font()( QFont( painter.font() ), 0.8 ) );
        painter.drawText( drawRect, Qt::TextWordWrap, text );
    }
    // ---------- end calibration equestion -----

    if ( connect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                  , pImpl_->summaryTable_, SLOT(handlePrint(QPrinter&, QPainter&)) ) ) {
        emit onPrint( printer, painter );
        bool res = disconnect( this, SIGNAL( onPrint(QPrinter&, QPainter&) ), pImpl_->summaryTable_, SLOT(handlePrint(QPrinter&, QPainter&)) );
        assert( res );
        (void)res;
    }
}
