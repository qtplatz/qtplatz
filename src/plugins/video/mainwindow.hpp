/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <utils/fancymainwindow.h>
#include <QWidget>
#include <memory>
#include <array>

namespace Core { class IMode; }
namespace Utils { class StyledBar; }
namespace adprot { class protein; class protfile; class protease; }
namespace adcontrols { class ChemicalFormula; }

class QToolButton;
class QAction;
class QLineEdit;
class QDockWidget;
class QStackedWidget;

namespace video {

    class iSequenceImpl; class iControllerImpl;

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow( QWidget *parent = 0 );
        ~MainWindow();

        void createActions();

        QWidget * createContents( Core::IMode * );
        void onInitialUpdate();
        void onFinalClose();

        void getExtensionFactories( iSequenceImpl& );
        void getExtensionFactories( iControllerImpl& );
        // bool loadFile(const QString &fileName);

        static QToolButton * toolButton( const char * );

    private:
        QStackedWidget * stack_;

        void createDockWidgets();
        QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& pagename = QString() );
        void setSimpleDockWidgetArrangement();
        Utils::StyledBar * createTopStyledToolbar();
        Utils::StyledBar * createMidStyledToolbar();

        static QToolButton * toolButton( QAction * );

        void commit();
        void handleIndexChanged( int index, int subIndex );
        void handleSequenceCompleted();
        void filePrintPdf();
        void hideDock( bool );

    signals:

    public slots :

    private slots :

    };

}

#endif // MAINWINDOW_HPP
