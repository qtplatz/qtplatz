/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "sequenceeditor.hpp"
#include "sequence.hpp"
#include "constants.hpp"
#include <coreplugin/uniqueidmanager.h>

using namespace sequence;
using namespace sequence::internal;

SequenceEditor::SequenceEditor(QObject *parent) :
    Core::IEditor(parent)
    , widget_(0)
{
  Core::UniqueIDManager* uidm = Core::UniqueIDManager::instance();
  if ( uidm ) {
    context_ << uidm->uniqueIdentifier( Constants::C_SEQUENCE );
  }
}

// Core::IEditor
bool
SequenceEditor::createNew(const QString &contents )
{
    Q_UNUSED( contents );
    return false;
}

bool
SequenceEditor::open(const QString &fileName )
{
    Q_UNUSED( fileName );
    // todo
    return false;
}

Core::IFile *
SequenceEditor::file()
{
  return static_cast<Core::IFile *>( sequence_.get() );
}

const char *
SequenceEditor::kind() const
{
  return Constants::C_SEQUENCE;
}

QString
SequenceEditor::displayName() const
{
  return displayName_;
}

void
SequenceEditor::setDisplayName(const QString &title)
{
  displayName_ = title;
}

bool
SequenceEditor::duplicateSupported() const
{
  return false;
}

Core::IEditor *
SequenceEditor::duplicate(QWidget *parent)
{
  Q_UNUSED( parent );
  return 0;
}

QByteArray
SequenceEditor::saveState() const
{
  return QByteArray();
}

bool
SequenceEditor::restoreState(const QByteArray &state)
{
  Q_UNUSED( state );
  return false;
}

bool
SequenceEditor::isTemporary() const
{
  return false;
}

QWidget *
SequenceEditor::toolBar()
{
  return 0;
}
// end Core::IEditor

void
SequenceEditor::slotTitleChanged( const QString& title )
{
    setDisplayName( title );
}
