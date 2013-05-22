/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "sequenceeditorfactory.hpp"
#include "sequenceeditor.hpp"
#include "constants.hpp"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/ifilefactory.h>

using namespace sequence::internal;

SequenceEditorFactory::~SequenceEditorFactory()
{
}

SequenceEditorFactory::SequenceEditorFactory(QObject *parent) :  Core::IEditorFactory(parent)
                                                              , kind_( Constants::C_SEQUENCE )
{
    mimeTypes_ << Constants::C_SEQUENCE_MIMETYPE;
}

// implementation for IEditorFactory
Core::IEditor *
SequenceEditorFactory::createEditor( QWidget * )
{
    return new SequenceEditor();
}

// implementation for IFileFactory

QStringList 
SequenceEditorFactory::mimeTypes() const
{
    return mimeTypes_;
}

QString 
SequenceEditorFactory::kind() const
{
  return kind_;
}

Core::IFile * 
SequenceEditorFactory::open(const QString& filename )
{
  Core::EditorManager * em = Core::EditorManager::instance();
  Core::IEditor * iface = em->openEditor( filename, kind_ );
  return iface ? iface->file() : 0;
}

