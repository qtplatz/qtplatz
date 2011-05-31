// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#ifndef ACQUIREUIMANAGER_H
#define ACQUIREUIMANAGER_H

#include <QObject>
#include <QWidget>
#if defined _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4805)
#endif
#include <adinterface/receiverC.h>
#if defined _MSC_VER
#pragma warning(default:4805)
#pragma warning(default:4996)
#endif

namespace Utils { class FancyMainWindow; }
class QDockWidget;
class QAction;
class QMainWindow;

namespace Acquire {

  namespace internal {

    struct AcquireManagerActions;
    struct AcquireUIManagerData;

    //------------
    //------------
    class AcquireUIManager : public QObject {
      Q_OBJECT
    public:
      ~AcquireUIManager();
      explicit AcquireUIManager(QObject *parent = 0);
      
      QMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();

      void OnInitialUpdate();
      void OnFinalClose();

      //
    signals:
        void signal_eventLog( QString );
        void signal_message( unsigned long msg, unsigned long value );
        void signal_debug_print( unsigned long priority, unsigned long category, QString text );

    public slots:
        void handle_message( unsigned long msg, unsigned long value );
        void handle_eventLog( const ::EventLog::LogMessage& );
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );

    private:
      AcquireUIManagerData * d_;

    public:
      //const AcquireManagerActions& acquireManagerActions() const;

    };

  }
}

#endif // ACQUIREUIMANAGER_H
