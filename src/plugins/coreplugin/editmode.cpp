/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "coreconstants.h"
#include "editmode.h"
#include "icore.h"
#include "modemanager.h"
#include "minisplitter.h"
#include "navigationwidget.h"
#include "outputpane.h"
#include "rightpane.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QLatin1String>
#include <QHBoxLayout>
#include <QWidget>
#include <QIcon>

using namespace Core;
using namespace Core::Internal;

EditMode::EditMode() :
    m_splitter(new MiniSplitter),
    m_rightSplitWidgetLayout(new QVBoxLayout)
{
    setObjectName(QLatin1String("EditMode"));
    setDisplayName(tr("Edit"));
    setIcon(QIcon(QLatin1String(":/fancyactionbar/images/mode_Edit.png")));
    setPriority(Constants::P_MODE_EDIT);
    setId(Constants::MODE_EDIT);

    m_rightSplitWidgetLayout->setSpacing(0);
    m_rightSplitWidgetLayout->setMargin(0);
    QWidget *rightSplitWidget = new QWidget;
    rightSplitWidget->setLayout(m_rightSplitWidgetLayout);
    auto editorPlaceHolder = new EditorManagerPlaceHolder(this);
    m_rightSplitWidgetLayout->insertWidget(0, editorPlaceHolder);

    MiniSplitter *rightPaneSplitter = new MiniSplitter;
    rightPaneSplitter->insertWidget(0, rightSplitWidget);
    rightPaneSplitter->insertWidget(1, new RightPanePlaceHolder(this));
    rightPaneSplitter->setStretchFactor(0, 1);
    rightPaneSplitter->setStretchFactor(1, 0);

    MiniSplitter *splitter = new MiniSplitter;
    splitter->setOrientation(Qt::Vertical);
    splitter->insertWidget(0, rightPaneSplitter);
    QWidget *outputPane = new Core::OutputPanePlaceHolder(this, splitter);
    outputPane->setObjectName(QLatin1String("EditModeOutputPanePlaceHolder"));
    splitter->insertWidget(1, outputPane);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 0);

    m_splitter->insertWidget(0, new NavigationWidgetPlaceHolder(this));
    m_splitter->insertWidget(1, splitter);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    connect(ModeManager::instance(), SIGNAL(currentModeChanged(Core::IMode*)),
            this, SLOT(grabEditorManager(Core::IMode*)));
    m_splitter->setFocusProxy(editorPlaceHolder);

    IContext *modeContextObject = new IContext(this);
    modeContextObject->setContext(Context(Constants::C_EDITORMANAGER));
    modeContextObject->setWidget(m_splitter);
    ICore::addContextObject(modeContextObject);

    setWidget(m_splitter);
    setContext(Context(Constants::C_EDIT_MODE,
                       Constants::C_NAVIGATION_PANE));
}

EditMode::~EditMode()
{
    delete m_splitter;
}

void EditMode::grabEditorManager(Core::IMode *mode)
{
    if (mode != this)
        return;

    if (EditorManager::currentEditor())
        EditorManager::currentEditor()->widget()->setFocus();
}
