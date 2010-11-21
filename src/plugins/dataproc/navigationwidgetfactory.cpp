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

    QToolButton * filter = new QToolButton;
    filter->setIcon( QIcon(":/dataproc/images/filtericon.png"));
    filter->setToolTip( tr("Filter tree"));
    filter->setPopupMode( QToolButton:: InstantPopup );

    n.dockToolBarWidgets << filter; // << ptw->togggleSync();
    return n;
}
