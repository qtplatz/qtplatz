//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "navigationwidgetfactory.h"
#include "navigationwidget.h"
#include <QKeySequence>
#include <QTreeView>
#include <QToolButton>

using namespace dataproc;

NavigationWidgetFactory::NavigationWidgetFactory()
{
}

NavigationWidgetFactory::~NavigationWidgetFactory()
{
}

QString
NavigationWidgetFactory::displayName()
{
    return tr("Open datafiles");
}

QKeySequence
NavigationWidgetFactory::activationSequence()
{
    return QKeySequence( Qt::ALT + Qt::Key_X );
}

Core::NavigationView
NavigationWidgetFactory::createWidget()
{
    Core::NavigationView n;
    NavigationWidget *ptw = new NavigationWidget;
    n.widget = ptw;

    QToolButton * toggleSync = new QToolButton;
    toggleSync->setIcon( QIcon(":/core/images/linkicon.png"));
    toggleSync->setCheckable( true );
    toggleSync->setChecked( ptw->autoSyncronization() );
    toggleSync->setToolTip( tr("Synchronize with Editor" ) );
    // filter->setPopupMode( QToolButton:: InstantPopup );

    connect( toggleSync, SIGNAL(clicked(bool)), ptw, SLOT( toggleAutoSynchronization() ) );

    n.dockToolBarWidgets << toggleSync;
    return n;
}
