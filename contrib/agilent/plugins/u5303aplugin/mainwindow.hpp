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
#include <array>
#include <atomic>
#include <memory>
#include <mutex>

class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;

namespace adcontrols { class MassSpectrum; class Trace; namespace ControlMethod { class Method; } }
namespace acqrscontrols { class method; }
namespace adextension { class iController; class iSequenceImpl; }

namespace Core { class IMode; class Context; }
namespace Utils { class StyledBar; }

namespace u5303a {

    namespace Internal { class u5303APlugin; }

	class MainWindow : public Utils::FancyMainWindow {
		Q_OBJECT
	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

		QWidget * createContents( Core::IMode * );
		
		void OnInitialUpdate();
		void OnFinalClose();
		void activateLayout();
		void setSimpleDockWidgetArrangement();
		QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& page = QString() );

        void createActions();

		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );

		static MainWindow * instance();

        void setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method> );
        
        std::shared_ptr< adcontrols::ControlMethod::Method > getControlMethod() const;

        void getEditorFactories( adextension::iSequenceImpl& );

        size_t findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& vec ) const;
        
        void editor_commit();

    private:
        std::vector< QWidget* > widgets_;
        QAction * createAction( const QString& iconname, const QString& msg, QObject * parent );

    signals:

	public slots:
        void actSnapshot();
        void iControllerConnected( adextension::iController * );
        void saveCurrentImage();
        void printCurrentView();
        void hideDock( bool );
                                                               
    private slots:
        void handle_reply( const QString&, const QString& );
        void handleInstState( int status );

	private:
        QAction * actionConnect_;
        static MainWindow * instance_;

        void setToolBarDockWidget( QDockWidget * dock );
        void createDockWidgets();
        Utils::StyledBar * createTopStyledToolbar();
        Utils::StyledBar * createMidStyledToolbar();
	};
	
}


