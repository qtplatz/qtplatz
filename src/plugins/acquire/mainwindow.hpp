// This is a -*- C++ -*- header.
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

#ifndef ACQUIREUIMANAGER_H
#define ACQUIREUIMANAGER_H

#include <QObject>
#include <QWidget>
#include <adinterface/receiverC.h>
#include <utils/fancymainwindow.h>

namespace adportable { class Configuration; }
namespace adextension { class iMonitorFactory; }
class QDockWidget;
class QAction;
class QMainWindow;

namespace acquire {  namespace internal {
        
        struct AcquireManagerActions;
        struct MainWindowData;
        
        //------------
        //------------
        class MainWindow : public Utils::FancyMainWindow {
            Q_OBJECT
        public:
            ~MainWindow();
            explicit MainWindow(QWidget *parent = 0);
      
            // QMainWindow * mainWindow() const;
            void init( const adportable::Configuration& config );
            void setSimpleDockWidgetArrangement();

            void OnInitialUpdate();
            void OnFinalClose();
            // 
            void eventLog( const QString& );

            //
        signals:
            void signal_eventLog( QString );
            void signal_message( unsigned long msg, unsigned long value );
            void signal_debug_print( unsigned long priority, unsigned long category, QString text );

        public slots:
            void handle_message( unsigned long msg, unsigned long value );
            // void handle_eventLog( const ::EventLog::LogMessage& );
            void handle_shutdown();
            void handle_debug_print( unsigned long priority, unsigned long category, QString text );

        private:
            QDockWidget * createDockWidget( QWidget * widget, const QString& title, const QString& objname );

        public:
            //const AcquireManagerActions& acquireManagerActions() const;

        };

    }
}

#endif // ACQUIREUIMANAGER_H
