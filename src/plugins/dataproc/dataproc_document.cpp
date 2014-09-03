/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "dataproc_document.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/msqpeaks.hpp>
#include <adcontrols/chromatogram.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/format.hpp>
#include <app/app_version.h>
#include <QSettings>

using namespace dataproc;

dataproc_document * dataproc_document::instance_ = 0;

dataproc_document::dataproc_document(QObject *parent) : QObject(parent)
                                    , quant_( std::make_shared< adcontrols::MSQPeaks >() )
                                    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                                , QLatin1String( "QtPlatz" ) ) )
{
}

dataproc_document * 
dataproc_document::instance()
{
    if ( instance_ == 0 )
        instance_ = new dataproc_document;
    return instance_;
}

adcontrols::MSQPeaks *
dataproc_document::msQuanTable()
{
    return quant_.get();
}

const adcontrols::MSQPeaks *
dataproc_document::msQuanTable() const
{
    return quant_.get();
}

void
dataproc_document::setMSQuanTable( const adcontrols::MSQPeaks& v )
{
    quant_ = std::make_shared< adcontrols::MSQPeaks >( v );
}

// static
size_t
dataproc_document::findCheckedTICs( Dataprocessor * dp, std::set< int >& vfcn )
{
    vfcn.clear();
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        for ( auto& folium: cfolder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                const std::wstring& name = folium.name();
                auto found = name.find( std::wstring( L"TIC/TIC.") );
                if ( found != std::wstring::npos ) {
                    auto dot = name.find_last_of( L'.' );
                    if ( dot != std::wstring::npos ) {
                        int fcn = std::stoi( name.substr( dot + 1 ) );
                        vfcn.insert( fcn - 1 );
                    }
                }
            }
        }
    }
    return vfcn.size();
}

//static
const std::shared_ptr< adcontrols::Chromatogram >
dataproc_document::findTIC( Dataprocessor * dp, int fcn )
{
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        std::wstring name = ( boost::wformat( L"TIC/TIC.%d" ) % ( fcn + 1 ) ).str();
        if ( auto folium = cfolder.findFoliumByName( name ) ) {
            auto cptr = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium );
            return cptr;
        }
    }
    return 0;
}

