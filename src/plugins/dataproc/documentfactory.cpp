// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "documentfactory.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include "dataproceditor.hpp"
#include "constants.hpp"

#include "msprocessingwnd.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <QStringList>

#include <adportable/debug.hpp>
#include <adcontrols/datafile.hpp>
#include <qtwrapper/debug.hpp>

using namespace dataproc;

///////////////////////////
DocumentFactory::~DocumentFactory()
{
}

DocumentFactory::DocumentFactory()
{
    setMimeTypes( {
            "application/adfs"
            // , "application/vnd.sqlite3" // disable
            , "application/txt"
            , "application/csv"
        } );

    setOpener( [&](const Utils::FilePath& filePath){ return this->open(filePath); });
}

Core::IDocument *
DocumentFactory::open(const Utils::FilePath &filePath)
{
    qDebug() << "## DocumentFactory::open(" << filePath << ") ##";
    auto processor = Dataprocessor::make_dataprocessor();
    QString errorString;
    if ( processor->open( /*&errorString,*/ filePath, filePath ) == Utils::ResultOk ) //Core::IDocument::OpenResult::Success )
        return processor.get();
    return 0;
}
