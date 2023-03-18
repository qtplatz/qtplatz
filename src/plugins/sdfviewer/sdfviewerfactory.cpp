// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2023 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "sdfviewerfactory.hpp"
#include "sdfviewer.hpp"
#include "sdfview.hpp"
#include "constants.hpp"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QSpacerItem>
#include <QToolBar>
#include <QWidget>

#include <adportable/debug.hpp>

using namespace Core;
using namespace Utils;

using namespace sdfviewer;

SDFViewerFactory::SDFViewerFactory()
{
    setId(constants::SDFVIEWER_ID);

    setDisplayName(QObject::tr("SDF Viewer"));

    setEditorCreator([] {
        ADDEBUG() << "########### SDFViewerFactory::editorCreator ##############";
        return new SDFViewer;
    });

    for ( auto &format : { "chemical/x-mdl-sdfile"
                           // , "application/x-sqlite3"
                           , "application/db" } ) {
        addMimeType( QString::fromLatin1(format) );
    }

    ADDEBUG() << "## SDFViewerFactory ##";
}
