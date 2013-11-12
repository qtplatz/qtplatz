// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include <QObject>
#include <QList>
#include <memory>
#include <QAction>

class QAction;

namespace Core { class IContext; }

namespace dataproc {
    
    class Dataprocessor;

    class ActionManager : public QObject {
        Q_OBJECT
    public:
        explicit ActionManager(QObject *parent = 0);

        bool initialize_actions( const QList<int>& context );
        bool saveDefaults();
        bool loadDefaults();

    private:
        std::unique_ptr< QAction > actSave_;
        std::unique_ptr< QAction > actSaveAs_;

        std::unique_ptr< QAction > closeCurrentEditorAction_;
        std::unique_ptr< QAction > closeAllEditorsAction_;
        std::unique_ptr< QAction > closeOtherEditorsAction_;
        std::unique_ptr< QAction > importFile_;
        std::unique_ptr< QAction > actMethodOpen_;
        std::unique_ptr< QAction > actMethodSave_;
        std::unique_ptr< QAction > actPrintCurrentView_;
        std::unique_ptr< QAction > actCalibFileApply_;

    signals:

    private slots:
        void handleSave();
        void handleSaveAs();

        void handleContextChanged( Core::IContext * );

        bool importFile();

        void actMethodSave();
        void actMethodOpen();

        void actPrintCurrentView();
        void actCalibFileApply();
    private:
        static QAction * create( const QString& icon_name, const QString& baloon, QObject * parent );
    };
}


