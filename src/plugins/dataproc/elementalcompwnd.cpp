//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompwnd.h"
#include "dataprocessor.h"
#include <adcontrols/timeutil.h>
#include <adutils/processeddata.h>
#include <portfolio/folium.h>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/axis.h>
#include <boost/variant.hpp>
#include <boost/any.hpp>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class ElementalCompWndImpl {
        public:
            ~ElementalCompWndImpl() {}
            ElementalCompWndImpl() : ticPlot_(0)
                , profileSpectrum_(0)
                , processedSpectrum_(0) {
            }
      
            adwidgets::ui::ChromatogramWidget * ticPlot_;
            adwidgets::ui::SpectrumWidget * profileSpectrum_;
            adwidgets::ui::SpectrumWidget * processedSpectrum_;
      
        };

        //---------------------------------------------------------
        template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
            Wnd& wnd_;
            selProcessed( Wnd& wnd ) : wnd_(wnd) {}

            template<typename T> void operator ()( T& ) const { }

            template<> void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                wnd_.draw2( ptr );
            }
        };
        //-----
    }
}


ElementalCompWnd::ElementalCompWnd(QWidget *parent) :
    QWidget(parent)
{
  init();
}

void
ElementalCompWnd::init()
{
  pImpl_.reset( new ElementalCompWndImpl );
  Core::MiniSplitter * splitter = new Core::MiniSplitter;
  if ( splitter ) {
      if ( pImpl_->processedSpectrum_ = new adwidgets::ui::SpectrumWidget ) {
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
    pImpl_->processedSpectrum_->setData( ms );
}

void
ElementalCompWnd::handleSessionAdded( Dataprocessor * )
{
}

void
ElementalCompWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    // boost::apply_visitor( selChanged(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ElementalCompWnd>( *this ), contents );
    }
}

