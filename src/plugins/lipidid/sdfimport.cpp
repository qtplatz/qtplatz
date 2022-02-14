/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "document.hpp"
#include "sdfimport.hpp"
#include <adchem/sdfile.hpp>
#include <adchem/sdmol.hpp>
#include <adchem/sdmolsupplier.hpp>
#include <qtwrapper/settings.hpp>
#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/inchi.h>
#include <boost/filesystem.hpp>
#include <QFileDialog>

using lipidid::SDFileImport;

namespace lipidid {

    class SDFileImport::impl {
    public:
        std::shared_ptr< adchem::SDFile > sdfile_;
        std::vector< adchem::SDMol > sdmols_;
    };

}

SDFileImport::~SDFileImport()
{
}

SDFileImport::SDFileImport( QWidget * parent ) : QWidget( parent )
                                               , impl_( std::make_unique< impl >() )
{
}


bool
SDFileImport::import()
{
    QString fn = QFileDialog::getOpenFileName(
        this
        , tr("Open File...")
        , qtwrapper::settings( *document::settings() ).recentFile( "SDF", "Files" )
        , tr("SDF Files (*.sdf);;All Files (*)"));

    if ( !fn.isEmpty() ) {
        auto path = boost::filesystem::path( fn.toStdString() );
        boost::system::error_code ec;
        if ( boost::filesystem::exists( path, ec ) ) {
            qtwrapper::settings( *document::settings() ).addRecentFiles( "SDF", "Files", fn );
            auto sdfile = adchem::SDFile::create( path.string() ); // std::shared_ptr< SDFile >
            if ( *sdfile ) {
                impl_->sdfile_ = std::move( sdfile );
            }
        }
    }
    return false;
}

void
SDFileImport::populate()
{
    auto progress = [&](size_t count){ if ( count % 1000 == 0 ) std::cerr << "\r" << count; };

    if ( impl_->sdmols_.empty() && impl_->sdfile_->size() ) {
        impl_->sdmols_ = impl_->sdfile_->populate( progress );
        std::cerr << "\r" << impl_->sdmols_.size() << std::endl;
    }
}
