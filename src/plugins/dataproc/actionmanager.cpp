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

#include "actionmanager.hpp"
#include "aboutdlg.hpp"
#include "constants.hpp"
#include "dataproc_document.hpp"
#include "mainwindow.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include "navigationwidget.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/utf.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/profile.hpp>
#include <adportfolio/portfolio.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mainwindow.h>
#include <utils/hostosinfo.h>
#include <extensionsystem/pluginmanager.h>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace dataproc;

ActionManager::ActionManager(QObject *parent) : QObject(parent)
{
    std::fill( actions_.begin(), actions_.end(), static_cast<QAction*>(0) );
}

bool
ActionManager::install_file_actions()
{
	if ( Core::ActionManager *am = Core::ActionManager::instance() ) {

        // File->Processing
        if ( Core::ActionContainer * menu = am->createMenu( "dataproc.menu" ) ) {

            menu->menu()->setTitle( tr("Processing") );

            menu->addAction( am->command( Constants::METHOD_OPEN ) );
            menu->addAction( am->command( Constants::METHOD_SAVE ) );
            menu->addAction( am->command( Constants::PRINT_CURRENT_VIEW ) );
            menu->addAction( am->command( Constants::CALIBFILE_APPLY ) );
            menu->addAction( am->command( Constants::PROCESS_ALL_CHECKED ) );
            menu->addAction( am->command( Constants::LISTPEAKS_ON_CHECKED ) );
            menu->addAction( am->command( Constants::IMPORT_ALL_CHECKED ) );

            menu->addAction( am->command( Constants::CREATE_SPECTROGRAM ) );
            menu->addAction( am->command( Constants::CLUSTER_SPECTROGRAM ) );

            menu->addAction( am->command( Constants::HIDE_DOCK ) );

            am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
        }
    }
    return true;
}

bool
ActionManager::install_toolbar_actions()
{
    return true;
}

bool
ActionManager::initialize_actions( const Core::Context& context )
{
    // About QtPlatz
    do {
        auto icon = QIcon::fromTheme( QLatin1String( "help-about") );
        auto action = new QAction( icon, Utils::HostOsInfo::isMacHost() ? tr( "About &QtPlatz" ) : tr("About &QtPlatz..."), this );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ABOUT_QTPLATZ, Core::Context( Core::Constants::C_GLOBAL ) );

        if ( Utils::HostOsInfo::isMacHost())
            cmd->action()->setMenuRole( QAction::ApplicationSpecificRole );
        auto mhelp = Core::ActionManager::actionContainer( Core::Constants::M_HELP );
        auto about = Core::ActionManager::command( Core::Constants::ABOUT_QTCREATOR );
        mhelp->menu()->insertAction( about->action(), cmd->action() );
        action->setEnabled( true );
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::aboutQtPlatz );
    } while(0);
        

	if ( auto * am = Core::ActionManager::instance() ) {

        if ( auto cmd = am->command( Core::Constants::OPEN ) )
            cmd->action()->setText( tr( "Open data files..." ) );  // override text

        if ( auto p = actions_[ idActSave ] = create( Constants::ICON_SAVE, tr("Save"), this ) ) {
            am->registerAction( p, Core::Constants::SAVE, context );
            connect( p, &QAction::triggered, this, &ActionManager::handleSave );
        }
        
        if ( auto p = actions_[ idActSaveAs ] = create( Constants::ICON_SAVE, tr("Save As"), this ) ) {
            am->registerAction( p, Core::Constants::SAVEAS, context );
            connect( p, &QAction::triggered, this, &ActionManager::handleSaveAs );
        }

        if ( auto p = actions_[ idActMethodOpen ] = create( Constants::ICON_METHOD_OPEN, tr("Process method open..."), this ) ) {
            am->registerAction( p, Constants::METHOD_OPEN, context );
            connect( p, &QAction::triggered, this, &ActionManager::actMethodOpen );
        }

        if ( auto p = actions_[ idActMethodSave ] = create( Constants::ICON_METHOD_SAVE, tr("Process method save..."), this ) ) {
            am->registerAction( p, Constants::METHOD_SAVE, context );
            connect( p, &QAction::triggered, this, &ActionManager::actMethodSave );
        }

        if ( auto p = actions_[ idActMethodApply ] = create( Constants::ICON_METHOD_APPLY, tr("Apply process method"), this ) ) {
            am->registerAction( p, Constants::METHOD_APPLY, context );
            connect( p, &QAction::triggered, this, &ActionManager::actMethodApply );
        }

        if ( auto p = actions_[ idActPrintCurrentView ] = create( Constants::ICON_PDF, tr("Print current view..."), this ) ) {
            am->registerAction( p, Constants::PRINT_CURRENT_VIEW, context );
            connect( p, &QAction::triggered, this, &ActionManager::actPrintCurrentView );
        }

        if ( auto p = actions_[ idActCalibFileApply ]
             = create( Constants::ICON_CALIBFILE, tr( "Apply mass calibration to this..." ), this ) ) {
            am->registerAction( p, Constants::CALIBFILE_APPLY, context );
            connect( p, &QAction::triggered, this, &ActionManager::actCalibFileApply );
        }

        if ( auto p = actions_[ idActApplyProcessToAllChecked ] = new QAction( tr( "Apply process to all checked spectra" ), this ) ) {
            am->registerAction( p, Constants::PROCESS_ALL_CHECKED, context );
            connect( p, &QAction::triggered, MainWindow::instance(), &MainWindow::handleProcessChecked );
        }

        if ( auto p = actions_[ idActExportPeakListAllChecked ] = new QAction( tr( "Export peak list on all checked spectra..." ), this ) ) {
            am->registerAction( p, Constants::LISTPEAKS_ON_CHECKED, context );
            connect( p, &QAction::triggered, MainWindow::instance(), &MainWindow::handleExportPeakList );
        }

        if ( auto p = actions_[ idActImportAllChecked ] = new QAction( tr( "Import and merge all checked spectra..." ), this ) ) {
            am->registerAction( p, Constants::IMPORT_ALL_CHECKED, context );
            connect( p, &QAction::triggered, MainWindow::instance(), &MainWindow::handleImportChecked );
        }

		if ( auto p = actions_[ idActCreateSpectrogram ] = new QAction( tr("Create Spectrogram"), this ) ) {
            am->registerAction( p, Constants::CREATE_SPECTROGRAM, context );
            connect( p, &QAction::triggered, MainWindow::instance(), &MainWindow::actCreateSpectrogram );
        }

		if ( auto p = actions_[ idActClusterSpectrogram ] = new QAction( tr("Cluster Spectrogram"), this ) ) {
            am->registerAction( p, Constants::CLUSTER_SPECTROGRAM, context );
            connect( p, &QAction::triggered, MainWindow::instance(), &MainWindow::actClusterSpectrogram );
        }

        // edit menu
        if ( auto p = actions_[ idActCheckAllSpectra ] = new QAction( tr( "Check all spectra" ), this ) ) 
            am->registerAction( p, Constants::CHECK_ALL_SPECTRA, context );

        if ( auto p = actions_[ idActUncheckAllSpectra ] = new QAction( tr( "Uncheck all spectra" ), this ) )
            am->registerAction( p, Constants::UNCHECK_ALL_SPECTRA, context );

        do {
            QIcon icon;
            icon.addPixmap( QPixmap( Constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
            icon.addPixmap( QPixmap( Constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );
            auto * action = new QAction( icon, tr( "Hide dock" ), this );
            action->setCheckable( true );
            am->registerAction( action, Constants::HIDE_DOCK, context );
            connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::hideDock );
        } while ( 0 );
    }

    connect( Core::ICore::instance(), &Core::ICore::contextChanged, this, &ActionManager::handleContextChanged );

    return  install_toolbar_actions() && install_file_actions();
}

void
ActionManager::connect_navigation_pointer( NavigationWidget * navi )
{
    connect( actions_[ idActCheckAllSpectra ], &QAction::triggered, navi, &NavigationWidget::handleCheckAllSpectra );
    connect( actions_[ idActUncheckAllSpectra ], &QAction::triggered, navi, &NavigationWidget::handleUncheckAllSpectra );

	if ( Core::ActionManager *am = Core::ActionManager::instance() ) {
        auto edit = am->actionContainer( Core::Constants::M_EDIT );
        edit->addAction( am->command( Constants::CHECK_ALL_SPECTRA ) );
        edit->addAction( am->command( Constants::UNCHECK_ALL_SPECTRA ) );
    }
}

QAction *
ActionManager::create( const QString& icon_name, const QString& msg, QObject * parent )
{
    QIcon icon;

    icon.addFile( icon_name );
    QAction * action = new QAction( icon, msg, parent );
    
    return action;
}

/////////////////////////////////////////////////////////////////
/// copy from editormanager.cpp

bool
ActionManager::importFile()
{
    return true;
}

void
ActionManager::actMethodSave()
{
    QString name = QFileDialog::getSaveFileName( MainWindow::instance()
                                                 , tr("Save process method")
                                                 , dataproc_document::instance()->recentFile( Constants::GRP_METHOD_FILES, true )
                                                 , tr("Process method files(*.pmth)" ) );

    adcontrols::ProcessMethod pm;
    MainWindow::instance()->getProcessMethod( pm );
    if ( dataproc_document::save( name, pm ) ) {
        dataproc_document::instance()->setProcessMethod( pm, name );
    }
}

void
ActionManager::actMethodOpen()
{
    QString name = QFileDialog::getOpenFileName( MainWindow::instance()
                                                 , tr("Open process method")
                                                 , dataproc_document::instance()->recentFile( Constants::GRP_METHOD_FILES, true )
                                                 , tr("Process method files(*.pmth);;XML Files(*.pmth.xml)" ) );

    adcontrols::ProcessMethod pm;
    if ( dataproc_document::load( name, pm ) ) {
        dataproc_document::instance()->setProcessMethod( pm, name );
    }
}

void
ActionManager::actMethodApply()
{
    MainWindow::instance()->actionApply();
}


void
ActionManager::actPrintCurrentView()
{
	std::string title;
	MainWindow::instance()->currentProcessView( title );

	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

		boost::filesystem::path path = dp->getPortfolio().fullpath();
		path = path.parent_path() / path.stem();

		boost::filesystem::path pdfname = path;
		pdfname.replace_extension( ".pdf" );
		int nnn = 0;
		while ( boost::filesystem::exists( pdfname ) )  {
			pdfname = path.wstring() + ( boost::wformat(L"(%d)") % nnn++ ).str();
			pdfname.replace_extension( ".pdf" );
		}
		std::string caption = ( boost::format( "Save %1% current view to file" ) % title ).str();
		QString qpdfname( qtwrapper::qstring::copy( pdfname.wstring() ) );
        QString fname = QFileDialog::getSaveFileName( MainWindow::instance() // parent
                                                      , caption.c_str()    // caption
                                                      , qpdfname                 // dir
                                                      , tr("PDF (*.pdf *.svg)") );  // filter
        MainWindow::instance()->printCurrentView( fname );
	} else {
        QMessageBox::warning( MainWindow::instance(), tr("Print current view"), tr("No current data exist") );
    }
}

void
ActionManager::actCalibFileApply()
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() );
    dir /= L"data";

	QFileDialog dlg( 0, "Open MS Calibration file", QString::fromStdWString( dir.wstring() ) );
	dlg.setNameFilter( tr("MSCalibrations(*.msclb)" ) );

	dlg.setFileMode( QFileDialog::ExistingFile );
    if ( dlg.exec() == QDialog::Accepted ) {
		auto result = dlg.selectedFiles();

        adcontrols::MSCalibrateResult calibResult;
        adcontrols::MassSpectrum ms;

        if ( Dataprocessor::loadMSCalibration( result[0].toStdWString(), calibResult, ms ) ) {

			std::wstring dataInterpreterClsid = adportable::utf::to_wstring( ms.getMSProperty().dataInterpreterClsid() );

            if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
				processor->applyCalibration( dataInterpreterClsid, calibResult );

        } else {
            QMessageBox::warning( 0, tr( "apply calibration" ), tr( "Calibration file load failed" ) );
		}
	}
}

void
ActionManager::handleOpen()
{
    // Core::EditorManager()
}

void
ActionManager::handleSave()
{
	Core::EditorManager::instance()->saveDocument();
}

void
ActionManager::handleSaveAs()
{
	Core::EditorManager::instance()->saveDocumentAs();
}

void
ActionManager::handleContextChanged( const QList<Core::IContext *>& t1, const Core::Context& )
{
    for ( auto& context : t1 ) {
        if ( Core::IEditor * editor = qobject_cast<Core::IEditor *>(context) ) {
            QString text = QString( tr( "Save '%1' As..." ) ).arg( editor->document()->filePath() );
            actions_[ idActSaveAs ]->setText( text );
        }
    }
}

