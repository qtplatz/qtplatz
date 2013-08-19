// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <adcontrols/processmethod.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>
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
    QIcon iconMethodOpen, iconMethodSave;
    iconMethodOpen.addFile( Constants::ICON_METHOD_OPEN );
    actMethodOpen_.reset( new QAction( iconMethodOpen, tr("Process method open..."), this ) );
    connect( actMethodOpen_.get(), SIGNAL( triggered() ), this, SLOT( actMethodOpen() ) );

    iconMethodSave.addFile( Constants::ICON_METHOD_SAVE );
    actMethodSave_.reset( new QAction( iconMethodSave, tr("Process method save..."), this ) );
    connect( actMethodSave_.get(), SIGNAL( triggered() ), this, SLOT( actMethodSave() ) );

	Core::ActionManager *am = Core::ICore::instance()->actionManager();
    if ( am ) {
        Core::Command * cmd = 0;
        cmd = am->registerAction( actMethodOpen_.get(), Constants::METHOD_OPEN, context );
        cmd = am->registerAction( actMethodSave_.get(), Constants::METHOD_SAVE, context );
		(void)cmd;
    }
    return true;
}

/////////////////////////////////////////////////////////////////
/// copy from editormanager.cpp
bool
ActionManager::saveFileAs() // Core::IEditor *editor )
{
    return false;
}

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
        adfs::cpio< adcontrols::ProcessMethod >::copyin( m, adfile );
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
        adfs::cpio< adcontrols::ProcessMethod >::copyout( m, *it );
        MainWindow::instance()->processMethodLoaded( name, m );
    }
}

