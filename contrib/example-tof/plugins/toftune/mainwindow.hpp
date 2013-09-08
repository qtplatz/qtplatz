/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <utils/fancymainwindow.h>
#include <memory>

class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;

namespace adcontrols { class MassSpectrum; class Trace; }
namespace ControlMethod{ struct Method; }
namespace tof { class ControlMethod; }
namespace tofinterface { struct tofDATA; }

namespace Core { class IMode; }

namespace toftune {

    namespace Internal { class tofTunePlugin; }

    class tofSignalMonitorView;
    class dataMediator;
    class iSequenceImpl;

	class MainWindow : public Utils::FancyMainWindow {
		Q_OBJECT
	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

		QWidget * createContents( Core::IMode * );
		void createActions( Internal::tofTunePlugin * );
		
		void OnInitialUpdate();
		void activateLayout();
		void setSimpleDockWidgetArrangement();
		QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );

		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );
		static MainWindow * instance();

        void setData( const adcontrols::MassSpectrum& );
        void setData( const adcontrols::Trace&, const std::wstring& traceId );
        void setMethod( const ControlMethod::Method& );
        bool editor_factories( iSequenceImpl& );

    signals:

	public slots:
        void onDataChanged( const dataMediator * );

	private:
		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
        QDockWidget * toolBarDockWidget_;
        QAction * actionConnect_;

        tofSignalMonitorView * monitorView_;
        std::unique_ptr< tof::ControlMethod > method_;

		QDockWidget * toolBarDockWidget() { return toolBarDockWidget_; }
        void setToolBarDockWidget( QDockWidget * dock );
        void createDockWidgets();
        void createToolbar();
	};
	
}


