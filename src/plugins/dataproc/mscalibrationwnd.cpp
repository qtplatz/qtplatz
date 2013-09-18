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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adutils/processeddata.hpp>
#include <qwt_plot_renderer.h>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QPrinter>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/xml_serializer.hpp> // for quick print
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <stack>
#include <tuple>

using namespace dataproc;

namespace dataproc {

    enum { idx_profile, idx_centroid };

    class MSCalibrationWndImpl {
    public:
        ~MSCalibrationWndImpl() {}
        MSCalibrationWndImpl() : processedSpectrum_(0)
                               , calibSummaryWidget_(0) {
        }
        adwplot::SpectrumWidget * processedSpectrum_;
        QWidget * calibSummaryWidget_;
        std::weak_ptr< adcontrols::MassSpectrum > calibSpectrum_;
        std::weak_ptr< adcontrols::MSCalibrateResult > calibResult_;
        portfolio::Folium folium_;
        std::stack< std::tuple< size_t, size_t, int > > stack_;
        void restore_state( adcontrols::MassSpectrum& centroid ) {
            if ( !stack_.empty() ) {
                adcontrols::segment_wrapper<> segments( centroid );
                const std::tuple< size_t, size_t, int >& state = stack_.top();
                segments[ std::get<0>( state ) ].setColor( std::get<1>( state ), std::get<2>( state ) );
                stack_.pop();
            }
        }
        void store_state( adcontrols::MassSpectrum& centroid, size_t fcn, size_t idx, int color ) {
            adcontrols::segment_wrapper<> segments( centroid );
            stack_.push( std::make_tuple<>( fcn, idx, segments[ fcn ].getColor( idx ) ) );
            segments[ fcn ].setColor( idx, color ); // green
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
    pImpl_.reset( new MSCalibrationWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        // spectrum on top
        pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this);
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
                     , SIGNAL( fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) )
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

    if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibSpectrum_.lock() )
        pImpl_->restore_state( *centroid );

    if ( folder && folder.name() == L"MSCalibration" ) {
        
        pImpl_->folium_ = folium;
        
        boost::any& data = folium;
        // profile spectrum
        if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( data ) ) { 
            adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( data );
            pImpl_->processedSpectrum_->setData( *ptr, idx_profile );
        }

        portfolio::Folio attachments = folium.attachments();
        // centroid spectrum
        Folio::iterator it = 
            Folium::find_first_of<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
        if ( it != attachments.end() ) {
            pImpl_->calibSpectrum_ = boost::any_cast< adutils::MassSpectrumPtr >( *it ); // weak_ptr
            pImpl_->processedSpectrum_->setData( *pImpl_->calibSpectrum_.lock(), idx_centroid );
            while ( !pImpl_->stack_.empty() )
                pImpl_->stack_.pop();

            if ( const adcontrols::ProcessMethodPtr method = Dataprocessor::findProcessMethod( *it ) )
                MainWindow::instance()->setProcessMethod( *method );
        }
        
        // calib result
        it = Folium::find_first_of<adcontrols::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
        if ( it != attachments.end() ) {
            pImpl_->calibResult_ = boost::any_cast< adutils::MSCalibrateResultPtr >( *it ); // weak_ptr

            if ( const adcontrols::ProcessMethodPtr method = Dataprocessor::findProcessMethod( *it ) )
                MainWindow::instance()->setProcessMethod( *method );
        }

        auto result = pImpl_->calibResult_.lock();
        auto spectrum = pImpl_->calibSpectrum_.lock();
        if ( result && spectrum )
            emit fireSetData( *result, *spectrum );
    }
}

void
MSCalibrationWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSCalibrationWnd::handleSelSummary( size_t idx, size_t fcn )
{
    if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibSpectrum_.lock() ) {
        pImpl_->restore_state( *centroid );
        pImpl_->store_state( *centroid, fcn, idx, 2 );
        pImpl_->processedSpectrum_->setData( *centroid, 1 );
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
MSCalibrationWnd::calibPolynomialFit( adcontrols::MSCalibrateResult& calibResult )
{
    adcontrols::ProcessMethod pm;
    const adcontrols::MSCalibrateMethod * pCalibMethod = 0;
    MainWindow::instance()->getProcessMethod( pm );
    if ( ! ( pCalibMethod = pm.find< adcontrols::MSCalibrateMethod >() ) )
        return false;

    // recalc polinomials
	mass_calibrator calibrator( calibResult.assignedMasses() );
        
    size_t nterm = pCalibMethod->polynomialDegree() + 1;
    if ( calibrator.size() < nterm )
        nterm = calibrator.size();
    
    adcontrols::MSCalibration calib;
    if ( calibrator.compute( calib, nterm ) ) {

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

    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {

        calibResult->assignedMasses( assigned );

        calibPolynomialFit( *calibResult );

        if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibSpectrum_.lock() ) {
            
            if ( DataprocHandler::doAnnotateAssignedPeaks( *centroid, assigned ) )
                pImpl_->processedSpectrum_->setData( *centroid, 1 ); 
            
            emit fireSetData( *calibResult, *centroid );
        }

    }
}


void
MSCalibrationWnd::handle_reassign_mass_requested()
{
    std::shared_ptr< adcontrols::MSCalibrateResult > calibResult = pImpl_->calibResult_.lock();
    if ( ! calibResult )
        return;

    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {

        calibResult->assignedMasses( assigned );

        if ( calibPolynomialFit( *calibResult ) ) {
            const adcontrols::MSCalibration& calib = calibResult->calibration();

            // update m/z for MSAssignedMasses
            for ( auto& it : calibResult->assignedMasses() )
                it.mass( mass_calibrator::compute_mass( it.time(), calib ) );

            if ( std::shared_ptr< adcontrols::MassSpectrum > centroid = pImpl_->calibSpectrum_.lock() ) {

                // profile
                adcontrols::MassSpectrumPtr profile = boost::any_cast< adcontrols::MassSpectrumPtr >( pImpl_->folium_.data() );
                if ( profile ) {
                    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *profile );
                    for ( auto& ms: segments ) {
						ms.setCalibration( calib );
                        for ( size_t i = 0; i < ms.size(); ++i )
                            ms.setMass( i, calib.compute_mass( ms.getTime( i ) ) );
                    }
                }
                pImpl_->processedSpectrum_->setData( *profile, idx_profile ); 

				// centroid (override profile that has better visibility)
                DataprocHandler::doAnnotateAssignedPeaks( *centroid, assigned );
                adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *centroid );
                for ( auto& ms: segments ) {
					ms.setCalibration( calib );
                    for ( size_t i = 0; i < ms.size(); ++i )
                        ms.setMass( i, calib.compute_mass( ms.getTime( i ) ) );
                }
                pImpl_->processedSpectrum_->setData( *centroid, idx_centroid ); 

                emit fireSetData( *calibResult, *centroid );
            }
        }
    }
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
    assert(0);
}

void
MSCalibrationWnd::handle_apply_calibration_to_default()
{
    handle_reassign_mass_requested();
    MainWindow::instance()->saveDefaultMSCalibrateResult( pImpl_->folium_ );
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
        portfolio::Folio::iterator it
            = portfolio::Folium::find_first_of<adcontrols::MSCalibrateResultPtr>( attachments.begin(), attachments.end() );
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
