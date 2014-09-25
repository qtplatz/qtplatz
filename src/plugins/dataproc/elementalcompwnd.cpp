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
#include <adcontrols/timeutil.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
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

    portfolio::Folio attachments = folium.attachments();
    if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
        
        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
            if ( centroid->isCentroid() ) {
                draw2( centroid );
                // pProcessedSpectrum_ = centroid;
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
