//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogramwnd.h"
#include "dataprocessor.h"
#include <adcontrols/description.h>
#include <adcontrols/datafile.h>
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/chromatogram.h>
#include <adutils/processeddata.h>
#include <portfolio/folium.h>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <qtwrapper/qstring.h>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/axis.h>
#include <QTableWidget>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {

    namespace internal {
        class ChromatogramWndImpl {
        public:
            ~ChromatogramWndImpl() {}
            ChromatogramWndImpl() : ticPlot_(0)
                , profileSpectrum_(0)
                , processedSpectrum_(0) {
            }

            void setData( const adcontrols::Chromatogram&, const QString& );
      
            adwidgets::ui::ChromatogramWidget * ticPlot_;
            adwidgets::ui::ChromatogramWidget * profileSpectrum_;
            adwidgets::ui::SpectrumWidget * processedSpectrum_;
      
        };

        //////////
    }
}


ChromatogramWnd::ChromatogramWnd(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void
ChromatogramWnd::init()
{
    pImpl_.reset( new ChromatogramWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( pImpl_->ticPlot_ = new adwidgets::ui::ChromatogramWidget ) {
            splitter->addWidget( pImpl_->ticPlot_ );
            //splitter->addWidget( pImpl_->profileSpectrum_ );
            //splitter->addWidget( pImpl_->processedSpectrum_ );
            QTableWidget * pTable = new QTableWidget;
            pTable->setRowCount(10);
            pTable->setColumnCount(10);
            splitter->addWidget( pTable );
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
ChromatogramWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    // pImpl_->profileSpectrum_->setData( ms );
}

void
ChromatogramWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    // pImpl_->processedSpectrum_->setData( ms );
}

void
ChromatogramWnd::draw( adutils::ChromatogramPtr& ptr )
{
    adcontrols::Chromatogram& c = *ptr;
    pImpl_->ticPlot_->setData( c );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            pImpl_->setData( c, filename );
        }
    }
}

namespace dataproc {
    namespace internal {
        namespace chromatogram {

            struct selChanged : public boost::static_visitor<void> {
                selChanged( ChromatogramWnd& wnd ) : wnd_(wnd) {}
                template<typename T> void operator ()( T& ) const { }
                template<> void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                    wnd_.draw1( ptr );
                }
                template<> void operator () ( adutils::ChromatogramPtr& ptr ) const {
                    wnd_.draw( ptr );
                }
                ChromatogramWnd& wnd_;
            };
            //----------------------------//
            struct selProcessed : public boost::static_visitor<void> {
                selProcessed( ChromatogramWnd& wnd ) : wnd_(wnd) {}
                template<typename T> void operator ()( T& ) const { }
                template<> void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                    wnd_.draw2( ptr );
                }
                template<> void operator () ( adutils::ChromatogramPtr& ptr ) const {
                    wnd_.draw( ptr );
                }
                ChromatogramWnd& wnd_;
            };
        }
    }
}

void
ChromatogramWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( chromatogram::selChanged(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( chromtogram::selProcessed( *this ), contents );
    }
}


///////////////////////////

void
ChromatogramWndImpl::setData( const adcontrols::Chromatogram& c, const QString& )
{
    ticPlot_->setData( c );
}