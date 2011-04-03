// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
    return tr("datafiles");
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
