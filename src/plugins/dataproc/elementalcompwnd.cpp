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
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/timeutil.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folium.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

using namespace dataproc;

namespace dataproc {

    class ElementalCompWndImpl {
    public:
        ~ElementalCompWndImpl() {}
        ElementalCompWndImpl() : ticPlot_(0)
                               , referenceSpectrum_(0)
                               , processedSpectrum_(0)
                               , drawIdx_(0) {
        }
      
        adplot::ChromatogramWidget * ticPlot_;
        adplot::SpectrumWidget * referenceSpectrum_;
        adplot::SpectrumWidget * processedSpectrum_;
        int drawIdx_;
    };

    // //---------------------------------------------------------
    // template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
    //     Wnd& wnd_;
    //     selProcessed( Wnd& wnd ) : wnd_(wnd) {}

    //     template<typename T> void operator ()( T& ) const { }

    //     void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
    //         wnd_.draw2( ptr );
    //     }
    // };

}

ElementalCompWnd::~ElementalCompWnd()
{
    delete pImpl_;
}

ElementalCompWnd::ElementalCompWnd(QWidget *parent) : QWidget(parent)
                                                    , pImpl_(0)
{
    init();
}

void
ElementalCompWnd::init()
{
    pImpl_ = new ElementalCompWndImpl;
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( ( pImpl_->processedSpectrum_ = new adplot::SpectrumWidget(this) ) )
            splitter->addWidget( pImpl_->processedSpectrum_ );
        if ( ( pImpl_->referenceSpectrum_ = new adplot::SpectrumWidget(this) ) )
            splitter->addWidget( pImpl_->referenceSpectrum_ );

        pImpl_->processedSpectrum_->link( pImpl_->referenceSpectrum_ );
        pImpl_->referenceSpectrum_->link( pImpl_->processedSpectrum_ );

        splitter->setOrientation( Qt::Vertical );
    }
  
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter );
    //toolBarAddingLayout->addWidget( toolBar2 );
}

void
ElementalCompWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    pImpl_->referenceSpectrum_->setData( ptr, 0 );
}

void
ElementalCompWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    pImpl_->processedSpectrum_->setData( ptr, pImpl_->drawIdx_++ );
}

void
ElementalCompWnd::handleAxisChanged( int axis )
{
    using adplot::SpectrumWidget;
    pImpl_->referenceSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
    pImpl_->processedSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
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
    pImpl_->drawIdx_ = 0;

    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

    if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            if ( ptr->size() > 0
                 && adportable::compare<double>::approximatelyEqual( ptr->getMass( ptr->size() - 1 ), ptr->getMass( 0 ) ) ) {
                // no mass assigned
                pImpl_->processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime, false );
            } else {
                pImpl_->processedSpectrum_->setAxis( adplot::SpectrumWidget::HorizontalAxisMass, false );
            }
            pImpl_->referenceSpectrum_->enableAxis( QwtPlot::yRight, true );            
            pImpl_->processedSpectrum_->enableAxis( QwtPlot::yRight, true );
            pImpl_->processedSpectrum_->setData( ptr, 1, true );
            pImpl_->processedSpectrum_->setAlpha( 1, 0x20 );
        }

        portfolio::Folio attachments = folium.attachments();
        if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                    return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
        
            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                if ( centroid->isCentroid() ) {
                    pImpl_->processedSpectrum_->setData( centroid, 0, false );
                }
            }
        }

    }
}

void
ElementalCompWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
	using adcontrols::IsotopeMethod;
	//using adcontrols::IsotopeCluster;
	using adcontrols::MassSpectrum;

//	const IsotopeMethod * p = m.find< IsotopeMethod >();
/*
	if ( p ) {
		for ( IsotopeMethod::vector_type::const_iterator it = p->begin(); it != p->end(); ++it ) {
			auto ms = std::make_shared< MassSpectrum >();
			ms->setAcquisitionMassRange( 50, 500 );
			if ( IsotopeCluster::isotopeDistribution( *ms, it->formula ) ) 
				pImpl_->processedSpectrum_->setData( ms, pImpl_->drawIdx_++ );
		}
	}
*/
    
}

void
ElementalCompWnd::simulate( const adcontrols::MSSimulatorMethod& m )
{
    auto ms = std::make_shared< adcontrols::MassSpectrum >();

    std::vector< std::tuple< std::string, std::string, double > > formulae;
    for ( auto& m : m.molecules().data() ) {
        if ( !m.adducts.empty() ) {
            std::vector< std::string > alist;
            auto v = adcontrols::ChemicalFormula::standardFormulae( m.formula, m.adducts, alist );
            for ( size_t i = 0; i < v.size(); ++i )
                formulae.push_back( std::make_tuple( v[ i ], m.formula + alist[ i ], m.abandance ) );
        } else
            formulae.push_back( std::make_tuple( m.formula, m.formula, m.abandance ) );
    }

    adcontrols::isotopeCluster isocalc;
    adcontrols::annotations annots;
    // annotation( const std::string&, double x = 0, double y = 0, int id = (-1), int priority = 0, DataFormat f = dataText );

    for ( auto& formula: formulae ) {
        adcontrols::mol::molecule mol;
        if ( adcontrols::ChemicalFormula::getComposition( mol.elements, std::get<0>( formula ) ) ) { // standard formula
            isocalc( mol );
            auto it = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                            , [](const adcontrols::mol::isotope& a, const adcontrols::mol::isotope& b){
                                                  return a.abundance < b.abundance; } );
            double pmax = it->abundance;
            auto last = std::remove_if( mol.cluster.begin(), mol.cluster.end()
                                        , [=]( const adcontrols::mol::isotope& i ){
                                            return i.abundance / pmax < 0.001;}); // delete less than 0.1% base peak

            for ( auto pi = mol.cluster.begin(); pi != last; ++pi )
                *( ms ) << std::make_pair( pi->mass, pi->abundance / pmax * 10000 * std::get<2>( formula ) );
        }
    }
    adportable::TimeSquaredScanLaw scanlaw( m.accelerator_voltage(), m.tDelay(), m.length() );
    for ( size_t i = 0; i < ms->size(); ++i )
        ms->setTime( i, scanlaw.getTime( ms->getMass( i ), 0 ) );

    for ( auto& f: formulae ) {
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( std::get<0>(f) );
        auto pos = ms->lower_bound( mass );
        if ( pos != adcontrols::MassSpectrum::npos ) {
            if ( pos != 0 ) {
                if ( std::abs( ms->getMass( pos ) - mass ) > std::abs( ms->getMass( pos - 1 ) - mass ) )
                    --pos;
            }
            annots << adcontrols::annotation( std::get<1>( f ) // formatted formula
                                              , mass, std::get<2>( f ) * 10000, int( pos ), 0, adcontrols::annotation::dataFormula );
        }
    }

    ms->setCentroid( adcontrols::CentroidNative );
    ms->set_annotations( annots );
	double lMass = ms->getMass( 0 );
	double hMass = ms->getMass( ms->size() - 1 );
    lMass = m.lower_limit() > 0 ? m.lower_limit() : double( int( lMass / 10 ) * 10 );
    hMass = m.upper_limit() > 0 ? m.upper_limit() : double( int( ( hMass + 10 ) / 10 ) * 10 );
    ms->setAcquisitionMassRange( lMass, hMass );
    draw1( ms );
}
