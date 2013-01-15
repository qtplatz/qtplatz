/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/smart_ptr.hpp>
#include <map>

class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QLineEdit;

namespace adcontrols { class ProcessMethod; }
namespace adportable { class Configuration; }
namespace adplugin   { class LifeCycle; }
namespace adextension { class iEditorFactory; }
namespace adsequence  { class sequence; }
namespace ControlMethod{ struct Method; }
namespace Core { class IMode; }

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

        bool getControlMethod( ControlMethod::Method& ) const;
        bool getProcessMethod( adcontrols::ProcessMethod& ) const;

        bool setControlMethod( const ControlMethod::Method& );
        bool setProcessMethod( const adcontrols::ProcessMethod& );
        void setControlMethodName( const QString& );
        void setProcessMethodName( const QString& );

		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );
		static MainWindow * instance();

    signals:
            
    public slots:

    private:
        static MainWindow * instance_;
		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
        QDockWidget * toolBarDockWidget_;
        QAction * actionConnect_;
        QLineEdit * ctrlMethodName_;
        QLineEdit * procMethodName_;
        boost::scoped_ptr< ControlMethod::Method > defaultControlMethod_;

        std::vector< adplugin::LifeCycle * > editors_;

		QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );
		QDockWidget * toolBarDockWidget() { return toolBarDockWidget_; }
        void setToolBarDockWidget( QDockWidget * dock );
        void createToolbar();
    };

}

#endif // MAINWINDOW_HPP
