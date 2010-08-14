//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "factory.h"
#include <adplugin/adplugin.h>
#include <QtCore/qplugin.h>
#include "logwidget.h"



using namespace qtwidgets;

factory::factory(QObject *parent) :
    adplugin::IFactory(parent)
{
}

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    if ( std::wstring(iid) == adplugin::iid_iLog ) {
       return new LogWidget( parent );
    } 
    return 0;
}

QObject *
factory::create_object( const wchar_t * iid, QObject * parent )
{
    return 0;
}

Q_EXPORT_PLUGIN( factory )
