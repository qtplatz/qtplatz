// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "elementalcompwnd.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/timeutil.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/polfit.hpp>
#include <adutils/processeddata.hpp>
#include <adportfolio/folium.hpp>
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
#include <QMenu>
#include <QPrinter>

using namespace dataproc;

ElementalCompWnd::~ElementalCompWnd()
{
}

ElementalCompWnd::ElementalCompWnd(QWidget *parent) : QWidget(parent)
                                                    , referenceSpectrum_( 0 )
                                                    , processedSpectrum_( 0 )
                                                    , drawIdx_( 0 )
{
    init();
}

void
ElementalCompWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;

    if ( splitter ) {
        if ( ( processedSpectrum_ = new adplot::SpectrumWidget(this) ) ) {
            splitter->addWidget( processedSpectrum_ );
            connect( processedSpectrum_, &adplot::SpectrumWidget::onSelected, this, &ElementalCompWnd::selectedOnProcessed );
        }

        if ( ( referenceSpectrum_ = new adplot::SpectrumWidget(this) ) )
            splitter->addWidget( referenceSpectrum_ );

        processedSpectrum_->setMinimumHeight( 80 );
        referenceSpectrum_->setMinimumHeight( 80 );        
		processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
		referenceSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
		processedSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
		referenceSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );

        // referenceSpectrum_->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );

        processedSpectrum_->link( referenceSpectrum_ );
        //referenceSpectrum_->link( processedSpectrum_ );

        splitter->setOrientation( Qt::Vertical );
    }
  
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
ElementalCompWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    referenceSpectrum_->setData( ptr, 0 );
}

void
ElementalCompWnd::estimateScanLaw( const QString& model_name, adutils::MassSpectrumPtr& ptr )
{
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
        double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( sformula );
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
    referenceSpectrum_->setAxis( axis == hor_axis_mass ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
    processedSpectrum_->setAxis( axis == hor_axis_mass ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
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
ElementalCompWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    drawIdx_ = 0;

    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

    if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            if ( ptr->size() > 0
                 && adportable::compare<double>::approximatelyEqual( ptr->getMass( ptr->size() - 1 ), ptr->getMass( 0 ) ) ) {
                // no mass assigned
                processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime, false );
            } else {
                processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisMass, false );
            }
            referenceSpectrum_->enableAxis( QwtPlot::yRight, true );            
            processedSpectrum_->enableAxis( QwtPlot::yRight, true );
            processedSpectrum_->setData( ptr, 1, true );
            processedSpectrum_->setAlpha( 1, 0x20 );
        }

        portfolio::Folio attachments = folium.attachments();
        if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                    return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
        
            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                if ( centroid->isCentroid() ) {
                    processedSpectrum_->setData( centroid, 0, false );
                    centroid_ = centroid;
                }
            }
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
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() )
        fullpath = processor->qfilename();
    
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

    renderer.render( referenceSpectrum_, &painter, rc );
    rc.moveTo( rc.left(), rc.bottom() );
    renderer.render( processedSpectrum_, &painter, rc );    
}

void
ElementalCompWnd::simulate( const adcontrols::MSSimulatorMethod& m )
{
    std::vector< std::string > display_formulae; // will expand from molecule + adduct combination
    std::vector< std::pair< std::string, double > > formula_abundances;

    for ( auto& mol : m.molecules().data() ) {
        if ( mol.enable() ) {
            if ( !std::string( mol.adducts() ).empty() ) {
                std::vector< std::string > alist;
                auto v = adcontrols::ChemicalFormula::standardFormulae( mol.formula(), mol.adducts(), alist );
                for ( size_t i = 0; i < v.size(); ++i ) {
                    formula_abundances.push_back( std::make_pair( v[ i ], mol.abundance() ) );
                    display_formulae.push_back( mol.formula() + alist[ i ] );
                }
            } else {
                formula_abundances.push_back( std::make_pair( mol.formula(), mol.abundance() ) );
                display_formulae.push_back( mol.formula() );
            }
        }
    }
    for ( const auto& f: formula_abundances ) {
        auto list = adcontrols::isotopeCluster::formulae( f.first );
        for ( auto& x: list )
            ADDEBUG() << x;
    }

    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->setCentroid( adcontrols::CentroidNative );

    adcontrols::isotopeCluster()( *ms, formula_abundances, m.resolvingPower() );
    if ( m.isTof() ) {
        adportable::TimeSquaredScanLaw scanLaw( m.acceleratorVoltage(), m.tDelay(), m.length() );
        for ( size_t i = 0; i < ms->size(); ++i )
            ms->setTime( i, scanLaw.getTime( ms->getMass( i ), 0 ) );
    }

    // annotation
    for ( size_t i = 0; i < formula_abundances.size(); ++i ) {
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula_abundances[ i ].first );
        auto pos = ms->find( mass, mass / m.resolvingPower() );
        if ( pos != adcontrols::MassSpectrum::npos ) {
            auto& annots = ms->get_annotations();
            annots << adcontrols::annotation( display_formulae[ i ], mass, ms->getIntensity( pos ), int( pos ), 0, adcontrols::annotation::dataFormula );
        }
    }

	double lMass = ms->getMass( 0 );
	double hMass = ms->getMass( ms->size() - 1 );
    lMass = m.lMassLimit() > 0 ? m.lMassLimit() : double( int( lMass / 10 ) * 10 );
    hMass = m.uMassLimit() > 0 ? m.uMassLimit() : double( int( ( hMass + 10 ) / 10 ) * 10 );
    ms->setAcquisitionMassRange( lMass, hMass );
    draw1( ms );
}

//////////////////////////////////////////
void
ElementalCompWnd::selectedOnProcessed( const QRectF& rc )
{
    QMenu menu;

    std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
    
    std::vector< QAction * > actions; // additional 
    if ( !models.empty() ) {
        for ( auto model : models )
            actions.push_back( menu.addAction( QString( "Estimate scan law based on %1" ).arg( QString::fromStdWString( model ) ) ) );
        if ( !centroid_.lock() )
            std::for_each( actions.begin(), actions.end(), []( QAction * a ){ a->setEnabled( false );  } );
    }
    
    auto selected = menu.exec( QCursor::pos() );
    if ( selected ) {
        auto it = std::find( actions.begin(), actions.end(), selected );
        if ( it != actions.end() ) {
            if ( auto centroid = centroid_.lock() )
                estimateScanLaw( QString::fromStdWString( models[ std::distance( actions.begin(), it ) ] ), centroid );
        }
    }
}

