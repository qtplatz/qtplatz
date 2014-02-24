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
#include "constants.hpp"
#include "mainwindow.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/utf.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/profile.hpp>
#include <portfolio/portfolio.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace dataproc;

ActionManager::ActionManager(QObject *parent) : QObject(parent)
{
}

bool
ActionManager::initialize_actions( const QList<int>& context )
{
    do {
        actMethodOpen_.reset( create( Constants::ICON_METHOD_OPEN, tr("Process method open..."), this ) );
        connect( actMethodOpen_.get(), SIGNAL( triggered() ), this, SLOT( actMethodOpen() ) );
    } while(0);

    do {
        actMethodSave_.reset( create( Constants::ICON_METHOD_SAVE, tr("Process method save..."), this ) );
        connect( actMethodSave_.get(), SIGNAL( triggered() ), this, SLOT( actMethodSave() ) );
    } while(0);

    do {
        actPrintCurrentView_.reset( create( Constants::ICON_PDF, tr("Print current view..."), this ) );
        connect( actPrintCurrentView_.get(), SIGNAL( triggered() ), this, SLOT( actPrintCurrentView() ) );
    } while(0);

    do {
        actCalibFileApply_.reset( create( Constants::ICON_CALIBFILE, tr("Apply mass calibration to this..."), this ) );
        connect( actCalibFileApply_.get(), SIGNAL( triggered() ), this, SLOT( actCalibFileApply() ) );
    } while(0);
    
	if ( Core::ActionManager *am = Core::ICore::instance()->actionManager() ) {
        Core::Command * cmdOpen = 0;
        Core::Command * cmdSave = 0;
        Core::Command * cmdPrint = 0;
        Core::Command * cmdCalib = 0;
        if ( am ) {
            cmdOpen = am->registerAction( actMethodOpen_.get(), Constants::METHOD_OPEN, context );
            cmdSave = am->registerAction( actMethodSave_.get(), Constants::METHOD_SAVE, context );
            cmdPrint = am->registerAction( actPrintCurrentView_.get(), Constants::PRINT_CURRENT_VIEW, context );
            cmdCalib = am->registerAction( actCalibFileApply_.get(), Constants::CALIBFILE_APPLY, context );
        }
        
        if ( Core::ActionContainer * menu = am->createMenu( "dataproc.menu" ) ) {
            menu->menu()->setTitle( "Processing" );
            menu->addAction( cmdPrint );
            menu->addAction( cmdOpen );
            menu->addAction( cmdCalib );
            am->actionContainer( Core::Constants::M_FILE )->addMenu( menu );
        }

        do {
            actSave_.reset( create( Constants::ICON_SAVE, tr("Save"), this ) );
            am->registerAction( actSave_.get(), Core::Constants::SAVE, context );
            connect( actSave_.get(), SIGNAL( triggered() ), this, SLOT( handleSave() ) );
        } while(0);

        do {
            actSaveAs_.reset( create( Constants::ICON_SAVE, tr("Save As..."), this ) );
            am->registerAction( actSaveAs_.get(), Core::Constants::SAVEAS, context );
            connect( actSaveAs_.get(), SIGNAL( triggered() ), this, SLOT( handleSaveAs() ) );
        } while(0);

        connect( Core::ICore::instance(), SIGNAL( contextChanged( Core::IContext * ) )
                 , this, SLOT( handleContextChanged( Core::IContext * ) ) );

    }

    return true;
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
                                                 , tr("Save process method"), "."
                                                 , tr("Process method files(*.pmth)" ) );
    if ( ! name.isEmpty() ) {
        boost::filesystem::path path( name.toStdString() );
        path.replace_extension( ".pmth" );
        adfs::filesystem file;
        try {
            if ( !file.create( path.wstring().c_str() ) )
                return;
        } catch ( adfs::exception& ex ) {
            QMessageBox::warning( 0, "Process method", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
            return;
        }

        adfs::folder folder = file.addFolder( L"/ProcessMethod" );
        adfs::file adfile = folder.addFile( path.wstring() ); // internal filename := os filename
        adcontrols::ProcessMethod m;
        MainWindow::instance()->getProcessMethod( m );
        try {
            adfs::cpio< adcontrols::ProcessMethod >::save( m, adfile );
        } catch ( std::exception& e ) {
            QMessageBox::warning( 0, "Save process method", 
                                  (boost::format("%1% @ %2% #%3%") % e.what() % __FILE__ % __LINE__ ).str().c_str() );
            return;
        }
        adfile.dataClass( adcontrols::ProcessMethod::dataClass() );
        adfile.commit();
        
        MainWindow::instance()->processMethodSaved( name );
    }
}

void
ActionManager::actMethodOpen()
{
    QString name = QFileDialog::getOpenFileName( MainWindow::instance()
                                                 , tr("Open process method"), "."
                                                 , tr("Process method files(*.pmth)" ) );
    if ( ! name.isEmpty() ) {
		boost::filesystem::path path( name.toStdString() );
        adfs::filesystem file;
        try {
            if ( ! file.mount( path.wstring().c_str() ) )
                return;
        } catch ( adfs::exception& ex ) {
            QMessageBox::warning( 0, "SequenceFile", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
            return;
        }

        adfs::folder folder = file.findFolder( L"/ProcessMethod" );
        std::vector< adfs::file > files = folder.files();
        if ( files.empty() )
            return;
        auto it = files.begin();
        adcontrols::ProcessMethod m;
        try {
            adfs::cpio< adcontrols::ProcessMethod >::load( m, *it );
        } catch ( std::exception& ex ) {
            QMessageBox::warning( 0, "Open process method"
                                  , (boost::format("%1% @ %2% #%3%") % ex.what() % __FILE__ % __LINE__ ).str().c_str() );
            return;
        } 
        MainWindow::instance()->processMethodLoaded( name, m );
    }
}

bool
ActionManager::saveDefaults()
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
    dir /= "data";
    if ( ! boost::filesystem::exists( dir ) ) 
        if ( ! boost::filesystem::create_directories( dir ) )
            return false;
    boost::filesystem::path fname = dir / "default.pmth";

    adfs::filesystem file;
    try {
        if ( !file.create( fname.wstring().c_str() ) )
            return false;
    } catch ( adfs::exception& ex ) {
        QMessageBox::warning( 0, "Process method", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
        return false;
    }
    adfs::folder folder = file.addFolder( L"/ProcessMethod" );
    adfs::file adfile = folder.addFile( fname.wstring() ); // internal filename := os filename
    adcontrols::ProcessMethod m;
    MainWindow::instance()->getProcessMethod( m );
    try {
        adfs::cpio< adcontrols::ProcessMethod >::save( m, adfile );
    } catch ( std::exception& e ) {
        QMessageBox::warning( 0, "Save default process method", 
                              (boost::format("%1% @ %2% #%3%") % e.what() % __FILE__ % __LINE__ ).str().c_str() );        
    }
    adfile.dataClass( adcontrols::ProcessMethod::dataClass() );
    adfile.commit();

    return true;
}

bool
ActionManager::loadDefaults()
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
    dir /= "data";

    boost::filesystem::path path = dir / "default.pmth";
    adfs::filesystem file;
    try {
        if ( ! file.mount( path.wstring().c_str() ) )
            return false;
    } catch ( adfs::exception& ex ) {
        QMessageBox::warning( 0, "SequenceFile", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
        return false;
    }

    adfs::folder folder = file.findFolder( L"/ProcessMethod" );
    std::vector< adfs::file > files = folder.files();
    if ( files.empty() )
        return false;
    auto it = files.begin();
    adcontrols::ProcessMethod m;
    try {
        adfs::cpio< adcontrols::ProcessMethod >::load( m, *it );
    } catch ( std::exception& ex ) {
        QMessageBox::warning( 0, "Open default process method"
                              , (boost::format("%1% @ %2% #%3%") % ex.what() % __FILE__ % __LINE__ ).str().c_str() );
        return false;
    } 

    MainWindow::instance()->processMethodLoaded( path.string().c_str(), m );
    return true;
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
			QMessageBox::warning( 0, "apply calibration", "Calibration file load failed" );
		}
	}
}

void
ActionManager::handleSave()
{
	Core::EditorManager::instance()->saveFile();
}

void
ActionManager::handleSaveAs()
{
	Core::EditorManager::instance()->saveFileAs();
}

void
ActionManager::handleContextChanged( Core::IContext * context )
{
	Core::IEditor *editor = context ? qobject_cast< Core::IEditor *>( context ) : 0;
	if ( editor && editor->file() ) {
        boost::filesystem::path path = editor->file()->fileName().toStdWString();
        QString text = QString::fromStdWString( ( boost::wformat( L"Save \"%1%\" As..." ) % path.stem().wstring() ).str() );
        actSaveAs_->setText( text );
	}
}
