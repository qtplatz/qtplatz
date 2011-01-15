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

#include "dataprocessor.h"
#include "datafileimpl.h"
#include "constants.h"
#include "sessionmanager.h"
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/ifile.h>
#include <portfolio/portfolio.h>
#include <portfolio/folium.h>
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/processeddataset.h>
#include <adcontrols/processmethod.h>
#include <adutils/processeddata.h>

#include <adcontrols/centroidmethod.h>
#include <adcontrols/isotopemethod.h>
#include <adcontrols/elementalcompositionmethod.h>
#include <adcontrols/mscalibratemethod.h>
#include <adcontrols/targetingmethod.h>

#include <adcontrols/massspectrum.h>
#include <adcontrols/centroidprocess.h>

#include <qdebug.h>

using namespace dataproc;

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor() : portfolio_( new portfolio::Portfolio() )
{
}

bool
Dataprocessor::open(const QString &fileName )
{
    adcontrols::datafile * file = adcontrols::datafile::open( qtwrapper::wstring::copy( fileName ), true );
    if ( file ) {
        datafileimpl_.reset( new datafileimpl( file ) );
        file->accept( *datafileimpl_ );
        file->accept( *this );
        return true;
    }
    return false;
}

Core::IFile *
Dataprocessor::ifile()
{
    return static_cast<Core::IFile *>( datafileimpl_.get() );
}

adcontrols::datafile&
Dataprocessor::file()
{
    return datafileimpl_->file();
}

adcontrols::LCMSDataset *
Dataprocessor::getLCMSDataset()
{
    return datafileimpl_->getLCMSDataset();
}

portfolio::Portfolio
Dataprocessor::getPortfolio()
{
    return * portfolio_;
}

void
Dataprocessor::setCurrentSelection( portfolio::Folium& folium )
{
    if ( folium.empty() ) {

        folium = file().fetch( folium.path(), folium.dataType() );
 
        portfolio::Folio attachs = folium.attachments();
        for ( portfolio::Folio::iterator it = attachs.begin(); it != attachs.end(); ++it ) {
            if ( it->empty() )
                *it = file().fetch( it->path(), it->dataType() );
        }
    }
    idActiveFolium_ = folium.id();
    SessionManager::instance()->selectionChanged( this, folium );
}

namespace dataproc {
    namespace internal {

        // dispatch method
        struct doSpectralProcess : public boost::static_visitor<bool> {
            const adutils::MassSpectrumPtr& ptr_;
            portfolio::Folium& folium;

            doSpectralProcess( const adutils::MassSpectrumPtr& p, portfolio::Folium& f ) : ptr_(p), folium(f) {
            }

            template<typename T> bool operator () ( T& ) const {
                return false;
            }
            template<> bool operator () ( const adcontrols::CentroidMethod& m ) const {
                portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );

                adcontrols::CentroidProcess peak_detector( m );
                peak_detector( *ptr_ );
                adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );
                peak_detector.getCentroidSpectrum( *pCentroid );

                boost::any& any = static_cast<boost::any&>( att );
                any = pCentroid;

                return true;
            }
        };

        // dispatch data type
        struct processIt : public boost::static_visitor<bool> {
            const adcontrols::ProcessMethod::value_type& m_;
            portfolio::Folium& folium_;

            processIt( const adcontrols::ProcessMethod::value_type& m, portfolio::Folium& f ) : m_(m), folium_(f) {
            }

            template<typename T> bool operator ()( T& ) const {
                return false;
            }

            template<> bool operator () ( adutils::MassSpectrumPtr& ptr ) const {
                return boost::apply_visitor( doSpectralProcess(ptr, folium_), m_ );
            }

            template<> bool operator () ( adutils::ChromatogramPtr& ) const {
                // todo:  add doChromatographicProcess
                return false;
            }
        };
        //-----
    }
}

void
Dataprocessor::applyProcess( const adcontrols::ProcessMethod& m )
{
    const adcontrols::CentroidMethod * pCentroidMethod = m.find< adcontrols::CentroidMethod >();
    (void)pCentroidMethod;

    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium ) {
        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
     
        for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
            boost::apply_visitor( internal::processIt(*it, folium), data );

        SessionManager::instance()->selectionChanged( this, folium );
    }
}

///////////////////////////
void
Dataprocessor::subscribe( adcontrols::LCMSDataset& data )
{
   (void)data;
/* 
   size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        adcontrols::Chromatogram c;
        if ( data.getTIC( i, c ) )
            ; // ticVec_.push_back( c );
    }
*/
}

void
Dataprocessor::subscribe( adcontrols::ProcessedDataset& processed )
{
    std::wstring xml = processed.xml();
    portfolio_.reset( new portfolio::Portfolio( xml ) );
}

