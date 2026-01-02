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

#include "sdfviewerdocument.hpp"
#include "constants.hpp"
#include "utils/result.h"
#include <adportable/debug.hpp>

#include <coreplugin/editormanager/documentmodel.h>
#include <coreplugin/editormanager/ieditor.h>

#include <utils/filepath.h>
#include <utils/mimeutils.h>
#include <utils/qtcassert.h>

#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QMovie>
#include <QPainter>
#include <QPixmap>

namespace sdfviewer {

    SDFViewerDocument::SDFViewerDocument()
    {
        setId(constants::SDFVIEWER_ID);
        connect(this, &SDFViewerDocument::mimeTypeChanged, this, &SDFViewerDocument::changed);
    }

    SDFViewerDocument::~SDFViewerDocument()
    {
        cleanUp();
    }
#if QTC_VERSION < 0x10'00'00
    Core::IDocument::OpenResult
    SDFViewerDocument::open(QString *errorString,
                        const Utils::FilePath &filePath,
                        const Utils::FilePath &realfilePath)
#else
   Utils::Result<>
    SDFViewerDocument::open( const Utils::FilePath &filePath
                             , const Utils::FilePath &realfilePath)
#endif
    {
        ADDEBUG() << "############ SDFViewerDocument::open( " << filePath.toUrlishString().toStdString() << " )";
        QTC_CHECK(filePath == realfilePath); // does not support auto save
        // OpenResult success = openImpl(errorString, filePath);
        // emit openFinished(success == OpenResult::Success);
        return Utils::ResultOk;
    }

    Core::IDocument::ReloadBehavior SDFViewerDocument::reloadBehavior(ChangeTrigger state, ChangeType type) const
    {
        if (type == TypeRemoved)
            return BehaviorSilent;
        if (type == TypeContents && state == TriggerInternal && !isModified())
            return BehaviorSilent;
        return BehaviorAsk;
    }

    Utils::Result<>
    SDFViewerDocument::reload( Core::IDocument::ReloadFlag flag
                               , Core::IDocument::ChangeType type)
    {
        Q_UNUSED(type)
            if (flag == FlagIgnore)
                return Utils::ResultOk;
        // emit aboutToReload();
        // bool success = (openImpl(errorString, filePath()) == OpenResult::Success);
        // emit reloadFinished(success);
        return Utils::ResultOk;
    }

    void
    SDFViewerDocument::updateVisibility()
    {
    }

    void
    SDFViewerDocument::cleanUp()
    {
    }

} // SDFViewer::Internal
