//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessor.h"
#include "datafileimpl.h"
#include "constants.h"
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>
#include <coreplugin/uniqueidmanager.h>

using namespace dataproc;
using namespace dataproc::internal;

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor()
{
}

Dataprocessor::Dataprocessor( const Dataprocessor& t ) : datafileimpl_( t.datafileimpl_ )
{
}

// Core::IEditor
//bool
//Dataprocessor::createNew(const QString &contents )
//{
//    Q_UNUSED( contents );
//    return false;
//}

bool
Dataprocessor::open(const QString &fileName )
{
    adcontrols::datafile * file = adcontrols::datafile::open( qtwrapper::wstring::copy( fileName ), true );
    if ( file ) {
        datafileimpl_.reset( new internal::datafileimpl( file ) );
        file->accept( *datafileimpl_ );
        return true;
    }
    return false;
}

//QWidget *
//Dataprocessor::widget()
//{
//    return widget_;
//}

Core::IFile *
Dataprocessor::ifile()
{
    return static_cast<Core::IFile *>( datafileimpl_.get() );
}

/*
const char *
Dataprocessor::kind() const
{
  return Constants::C_DATAPROCESSOR;
}

QString
Dataprocessor::displayName() const
{
    return displayName_;
}

void
Dataprocessor::setDisplayName(const QString &title)
{
    displayName_ = title;
}

bool
Dataprocessor::duplicateSupported() const
{
    return true;
}

Core::IEditor *
Dataprocessor::duplicate(QWidget *parent)
{
  Q_UNUSED( parent );
  return 0;
}

QByteArray
Dataprocessor::saveState() const
{
  return QByteArray();
}

bool
Dataprocessor::restoreState(const QByteArray &state)
{
  Q_UNUSED( state );
  return false;
}

bool
Dataprocessor::isTemporary() const
{
  return false;
}

QWidget *
Dataprocessor::toolBar()
{
  return 0;
}
// end Core::IEditor

void
Dataprocessor::slotTitleChanged( const QString& title )
{
    setDisplayName( title );
}
*/