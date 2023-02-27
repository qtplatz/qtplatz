// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "quandocument.hpp"

using namespace quan;

QuanDocument::~QuanDocument()
{
}

QuanDocument::QuanDocument()
{
}

Core::IDocument::OpenResult
QuanDocument::open(QString *errorString, const Utils::FilePath &filePath,
                    const Utils::FilePath &realFilePath)
{


    return Core::IDocument::OpenResult::ReadError;
}

Core::IDocument::ReloadBehavior
QuanDocument::reloadBehavior(ChangeTrigger state, ChangeType type) const
{
    return IDocument::BehaviorSilent;
}

bool
QuanDocument::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

bool
QuanDocument::isModified() const
{
    return false;
}

bool
QuanDocument::isSaveAsAllowed() const
{
    return false;
}

QString
QuanDocument::defaultPath() const
{
    return {};
}

QString
QuanDocument::suggestedFileName() const
{
    return {};
}

bool
QuanDocument::isFileReadOnly() const
{
    return true;
}
