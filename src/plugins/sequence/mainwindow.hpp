/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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
#include <map>
#include <memory>

class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QLineEdit;

namespace adcontrols { class ProcessMethod; class ControlMethod; }
namespace adportable { class Configuration; }
namespace adplugin   { class LifeCycle; }
namespace adextension { class iEditorFactory; }
namespace adsequence  { class sequence; }
namespace ControlMethod{ struct Method; }
namespace Core { class IMode; }
namespace Utils { class StyledBar; }

namespace sequence {
    
    class SequenceWidget;

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

		QWidget * createContents( Core::IMode * );
		void createActions();

		void OnInitialUpdate();
        void OnFinalClose();
		void activateLayout();
		void setSimpleDockWidgetArrangement();

        bool getControlMethod( adcontrols::ControlMethod& ) const;
        bool getProcessMethod( adcontrols::ProcessMethod& ) const;

        bool setControlMethod( const adcontrols::ControlMethod& );
        bool setProcessMethod( const adcontrols::ProcessMethod& );
        void setControlMethodName( const QString& );
        void setProcessMethodName( const QString& );

		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );
		static MainWindow * instance();

    signals:
            
    public slots:
        void handleOpenSequence();

    private:
        static MainWindow * instance_;
        QAction * actionConnect_;
        QLineEdit * ctrlMethodName_;
        QLineEdit * procMethodName_;
        std::unique_ptr< adcontrols::ControlMethod > defaultControlMethod_;

        std::vector< adplugin::LifeCycle * > editors_;

		QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& objname = QString() );
        void createToolbar();
        Utils::StyledBar * createTopStyledBar();
        Utils::StyledBar * createMidStyledBar();
        
        // Utils::StyledBar * createMidStyledToolbar();
    };

}

#endif // MAINWINDOW_HPP
