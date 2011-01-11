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
    SessionManager::instance()->selectionChanged( this, folium );
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

