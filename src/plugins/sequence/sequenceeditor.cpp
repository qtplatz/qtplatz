//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
