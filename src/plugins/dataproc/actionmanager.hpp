// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <QAction>
#include <memory>
#include <array>

class QAction;

namespace Core { class IContext; class Context; }

namespace dataproc {

    class Dataprocessor;
    class NavigationWidget;

    class ActionManager : public QObject {
        Q_OBJECT
    public:
        explicit ActionManager(QObject *parent = 0);

        enum idActions {
            idActOpen
            , idActSave
            , idActSaveAs
            , idActCloseCurrentEditor
            , idActCloseAllEditor
            , idActOtherEditor
            , idActImportFile
            , idActMethodOpen
            , idActMethodSave
            , idActMethodApply
            , idActPrintCurrentView
            , idActCalibFileApply
            , idActProcessCheckedSpectra
            , idActExplortCheckedSpectra
            , idActCheckAllSpectra
            , idActUncheckAllSpectra
            , idActApplyProcessToAllChecked
            , idActExportPeakListAllChecked
            , idActImportAllChecked  // a.k.a. merge into a file
            , idActCreateSpectrogram
            , idActClusterSpectrogram
            , idActExportAllChecked
            , idActExportRMSAllChecked
            , numOfActions
        };

        bool initialize_actions( const Core::Context& context );
        void connect_navigation_pointer( dataproc::NavigationWidget * navi );

    private:
        std::array< QAction *, numOfActions > actions_;

        bool install_edit_actions();
        bool install_file_actions();
        bool install_toolbar_actions();

    signals:

    private slots :
        void handleOpen();
        void handleSave();
        void handleSaveAs();

        void handleContextChanged( const QList<Core::IContext *>&, const Core::Context& );

        bool importFile();

        void actMethodSave();
        void actMethodOpen();

        void actPrintCurrentView();
        void actCalibFileApply();
        void actMethodApply();
    private:
        void handleCheckAllSpectra();   // handled by NavigationWidget
        void handleUncheckAllSpectra(); // handled by NavigationWidget

        static QAction * create( const QString& icon_name, const QString& baloon, QObject * parent );
    };
}
