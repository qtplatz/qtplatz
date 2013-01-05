// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

using namespace dataproc;

namespace dataproc {

    class ElementalCompWndImpl {
    public:
        ~ElementalCompWndImpl() {}
        ElementalCompWndImpl() : ticPlot_(0)
                               , profileSpectrum_(0)
                               , processedSpectrum_(0)
                               , drawIdx_(0) {
        }
      
        adwplot::ChromatogramWidget * ticPlot_;
        adwplot::SpectrumWidget * profileSpectrum_;
        adwplot::SpectrumWidget * processedSpectrum_;
        size_t drawIdx_;
    };

    //---------------------------------------------------------
    template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
        Wnd& wnd_;
        selProcessed( Wnd& wnd ) : wnd_(wnd) {}

        template<typename T> void operator ()( T& ) const { }

        void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.draw2( ptr );
        }
    };

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
        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            splitter->addWidget( pImpl_->processedSpectrum_ );
            splitter->setOrientation( Qt::Vertical );
        }
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
    adcontrols::MassSpectrum& ms = *ptr;
    pImpl_->profileSpectrum_->setData( ms );
}

void
ElementalCompWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    pImpl_->processedSpectrum_->setData( ms, pImpl_->drawIdx_++ );
}

void
ElementalCompWnd::handleSessionAdded( Dataprocessor * )
{
}

void
ElementalCompWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    pImpl_->drawIdx_ = 0;

    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    // boost::apply_visitor( selChanged(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ElementalCompWnd>( *this ), contents );
    }
}

void
ElementalCompWnd::onApplyMethod( const adcontrols::ProcessMethod& m )
{
	using adcontrols::IsotopeMethod;
	using adcontrols::IsotopeCluster;
	using adcontrols::MassSpectrum;

	const IsotopeMethod * p = m.find< IsotopeMethod >();
	if ( p ) {
		for ( IsotopeMethod::vector_type::const_iterator it = p->begin(); it != p->end(); ++it ) {
			MassSpectrum ms;
			ms.setAcquisitionMassRange( 50, 500 );
			if ( IsotopeCluster::isotopeDistribution( ms, it->formula ) ) 
				pImpl_->processedSpectrum_->setData( ms, pImpl_->drawIdx_++ );
		}
	}
    
}
