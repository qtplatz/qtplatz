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

#pragma once

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/idocument.h>
#include <memory>

namespace sdfviewer {

    class SDFViewerDocument;

    class SDFViewer : public Core::IEditor {
        Q_OBJECT

    public:
        SDFViewer();
        ~SDFViewer() override;

        Core::IDocument *document() const override;
        QWidget *toolBar() override;

    private:
        SDFViewer(std::unique_ptr< SDFViewerDocument > && );
        void ctor();
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
