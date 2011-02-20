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
#include "dataprochandler.h"
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/ifile.h>
#include <portfolio/portfolio.h>
#include <portfolio/folium.h>
#include <portfolio/folder.h>
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
#include <adcontrols/mscalibrateresult.h>
#include <adcontrols/centroidprocess.h>
#include <adcontrols/descriptions.h>
#include <adcontrols/description.h>
#include <stack>
#include <qdebug.h>

using namespace dataproc;

namespace dataproc {
    namespace internal {
        struct DataprocessorImpl {
            static adcontrols::MassSpectrumPtr findAttachedMassSpectrum( portfolio::Folium& folium );
            static bool applyMethod( portfolio::Folium&, const adcontrols::IsotopeMethod& );
            static bool applyMethod( portfolio::Folium&, const adcontrols::MSCalibrateMethod& );
            static bool applyMethod( portfolio::Folium&, const adcontrols::CentroidMethod&, const adcontrols::MassSpectrum& );
        };
    }
}

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor() : portfolio_( new portfolio::Portfolio() )
{
}

bool
Dataprocessor::create(const QString& token )
{
    adcontrols::datafile * file = adcontrols::datafile::open( qtwrapper::wstring::copy( token ), false );
    if ( file ) {
        datafileimpl_.reset( new datafileimpl( file ) );
        file->accept( *datafileimpl_ );
        file->accept( *this );
        return true;
    }
    return false;
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

        folium = file().fetch( folium.path(), folium.dataClass() );
 
        portfolio::Folio attachs = folium.attachments();
        for ( portfolio::Folio::iterator it = attachs.begin(); it != attachs.end(); ++it ) {
            if ( it->empty() )
                *it = file().fetch( it->path(), it->dataClass() );
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
                return internal::DataprocessorImpl::applyMethod( folium, m, *ptr_ );
            }

            template<> bool operator () ( const adcontrols::IsotopeMethod& m ) const {
                return internal::DataprocessorImpl::applyMethod( folium, m );
            }

            template<> bool operator () ( const adcontrols::MSCalibrateMethod& m ) const {
                return internal::DataprocessorImpl::applyMethod( folium, m );
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
Dataprocessor::applyProcess( const adcontrols::ProcessMethod& m, internal::ProcessType procType )
{
    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium ) {
        adcontrols::ProcessMethod method;
        //------------------ remove 'calibration' from method pipeline --------------
        for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it ) {
            if ( it->type() != typeid( adcontrols::MSCalibrateMethod ) )
                method.appendMethod( *it );

            // check and add Isotop method
            if ( it->type() == typeid( adcontrols::IsotopeMethod ) && procType == internal::IsotopeProcess )
                method.appendMethod( *it );
        }
        //---------------------------------------------------------------------------

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
     
        for ( adcontrols::ProcessMethod::vector_type::const_iterator it = method.begin(); it != method.end(); ++it )
            boost::apply_visitor( internal::processIt(*it, folium), data );

        SessionManager::instance()->selectionChanged( this, folium );
    }
}

void
Dataprocessor::applyCalibration( const adcontrols::ProcessMethod& m )
{
    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium ) {
        adcontrols::ProcessMethod method;
        //----------------------- take centroid and calibration method w/ modification ----------------------
        for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it ) {
            if ( it->type() == typeid( adcontrols::CentroidMethod ) ) {
                adcontrols::CentroidMethod centroidMethod( boost::get< adcontrols::CentroidMethod >(*it) );
                centroidMethod.centroidAreaIntensity( false );  // force hight for easy overlay
                method.appendMethod( centroidMethod );
            } else if ( it->type() == typeid( adcontrols::MSCalibrateMethod ) ) {
                method.appendMethod( boost::get< adcontrols::MSCalibrateMethod >( *it ) );
            }
        }
        //----------------------------------------------------------------------------------------------------

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
        if ( data.type() == typeid( adutils::MassSpectrumPtr ) )
            addCalibration( * boost::get< adutils::MassSpectrumPtr >( data ), method );
    }
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.addFolium( L"CalibrantSpectrum" );

    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    static_cast<boost::any&>( folium ) = ms;

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( internal::doSpectralProcess( ms, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
}

void
Dataprocessor::addSpectrum( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectra" );

    const adcontrols::Descriptions& descs = src.getDescriptions();
    std::wstring name;
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[i].text();

    portfolio::Folium folium = folder.addFolium( name );

    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    static_cast<boost::any&>( folium ) = ms;

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( internal::doSpectralProcess( ms, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
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

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

adcontrols::MassSpectrumPtr
internal::DataprocessorImpl::findAttachedMassSpectrum( portfolio::Folium& folium )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr ptr;

    Folium::vector_type& atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find_first_of< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() )
        Folium::get< adcontrols::MassSpectrumPtr >( ptr, *it);

    return ptr; // can be null
}

bool
internal::DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::IsotopeMethod& m )
{
    adcontrols::MassSpectrumPtr prev = findAttachedMassSpectrum( folium );
    // copy centroid result if exist, for meta data copy
    adcontrols::MassSpectrumPtr pResult( new adcontrols::MassSpectrum( *prev ) );
    portfolio::Folium att = folium.addAttachment( L"Isotope Cluster" );
    if ( DataprocHandler::doIsotope( *pResult, m ) )
        static_cast< boost::any& >( att ) = pResult;
    return true;
}

bool
internal::DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::MSCalibrateMethod& m )
{
    using namespace portfolio;
    Folium::vector_type& atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find_first_of< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() ) {
        adcontrols::MassSpectrumPtr pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>( *it ) );
        if ( pCentroid ) {
            adcontrols::MSCalibrateResultPtr pResult( new adcontrols::MSCalibrateResult );
            if ( DataprocHandler::doMSCalibration( *pResult, *pCentroid, m ) ) {
                portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                static_cast<boost::any&>( att ) = pResult;
            }
            return true;
        }
    }
    return false;
}

bool
internal::DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::CentroidMethod& m, const adcontrols::MassSpectrum& profile )
{
    portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
    adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );
    if ( DataprocHandler::doCentroid( *pCentroid, profile, m ) )
        static_cast<boost::any&>( att ) = pCentroid;
    return true;
}
