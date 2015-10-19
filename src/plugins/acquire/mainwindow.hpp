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

namespace adcontrols {
    namespace ControlMethod { class Method; }
    class SampleRun;
}

namespace adportable { class Configuration; }
namespace adextension { class iMonitorFactory; class iController; }
namespace adwidgets { class ControlMethodWidget; class SampleRunWidget; }
class QDockWidget;
class QAction;
class QMainWindow;

namespace acquire {
        
    struct AcquireManagerActions;
    struct MainWindowData;
        
    //------------
    //------------
    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        ~MainWindow();
        explicit MainWindow(QWidget *parent = 0);

        static MainWindow * instance();
      
        void init( const adportable::Configuration& config );
        void setSimpleDockWidgetArrangement();

        void OnInitialUpdate();
        void OnFinalClose();
        // 
        void eventLog( const QString& );

        std::shared_ptr< adcontrols::ControlMethod::Method > getControlMethod();
        void getControlMethod( adcontrols::ControlMethod::Method& );
        void setControlMethod( const adcontrols::ControlMethod::Method& );

        bool getSampleRun( adcontrols::SampleRun& );
        void setSampleRun( const adcontrols::SampleRun& );

        size_t findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& ) const;
        //
    signals:
        void signal_eventLog( QString );
        void signal_message( unsigned long msg, unsigned long value );
        void signal_debug_print( unsigned long priority, unsigned long category, QString text );

    public slots:
        void handle_message( unsigned long msg, unsigned long value );
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );
        void handleControlMethod();

        // new interface for pure c++ instrument controller (aka iController)
        void iControllerConnected( adextension::iController * inst );
        void iControllerMessage( adextension::iController *, uint32_t msg, uint32_t value );

    private:
        QDockWidget * createDockWidget( QWidget * widget, const QString& title, const QString& objname );
        adwidgets::ControlMethodWidget * cmEditor_;
        adwidgets::SampleRunWidget * runEditor_;
        static MainWindow * instance_;
    };

}

#endif // ACQUIREUIMANAGER_H
