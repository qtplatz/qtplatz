/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "sequenceeditor.hpp"
#include "sequencefile.hpp"
#include "constants.hpp"
#include "sequencewidget.hpp"
#include "mainwindow.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <qtwrapper/qstring.hpp>

using namespace sequence;

SequenceEditor::~SequenceEditor()
{
    delete file_;
    delete widget_;
}

SequenceEditor::SequenceEditor(QObject *parent) : Core::IEditor(parent)
                                                , displayName_( "Sequence Editor" )
                                                , file_( new SequenceFile( *this ) )
                                                , widget_( new SequenceWidget( file_->adsequence().schema(), 0 ) )
                                                , currRow_( 0 )
                                                , currCol_( 0 )
{
    Core::UniqueIDManager* uidm = Core::UniqueIDManager::instance();
    if ( uidm )
        context_ << uidm->uniqueIdentifier( Constants::C_SEQUENCE_EDITOR );

    widget_->OnInitialUpdate( file_->adsequence().schema() );
    bool res;	
	res = connect( widget_, SIGNAL( lineAdded( size_t ) ), this, SLOT( onLineAdded( size_t ) ) );
	assert( res );
	res = connect( widget_, SIGNAL( lineDeleted( size_t ) ), this, SLOT( onLineDeleted( size_t ) ) );
	assert( res );
	res = connect( widget_, SIGNAL( currentChanged( size_t, size_t ) ), this, SLOT( onCurrentChanged( size_t, size_t ) ) );
	assert( res );
}

// Core::IEditor
bool
SequenceEditor::createNew(const QString &contents )
{
    Q_UNUSED( contents );
	widget_->OnInitialUpdate( file_->adsequence().schema() );
	widget_->setSequenceName( "default.sequ" );
	file_->fileName( "default.sequ" );
    
	return true;
}

const char *
SequenceEditor::uniqueModeName() const 
{
    return sequence::Constants::C_SEQUENCE_MODE;
}

bool
SequenceEditor::open( const QString &fileName )
{
    boost::filesystem::path path( qtwrapper::wstring::copy( fileName ) );
    
    if ( boost::filesystem::exists( path ) && file_->load( fileName ) ) {

        setSequence( file_->adsequence() );
        widget_->setSequenceName( fileName );

        Core::FileManager * filemgr = Core::ICore::instance()->fileManager();
        if ( filemgr && filemgr->addFile( file_ ) )
            filemgr->addToRecentFiles( fileName );

        return true;
    } else {
        widget_->setSequenceName( fileName );
        return true;
    }

    return false;
}

Core::IFile *
SequenceEditor::file()
{
    return static_cast<Core::IFile *>( file_ );
}

const char *
SequenceEditor::kind() const
{
    return Constants::C_SEQUENCE_EDITOR;
}

QString
SequenceEditor::displayName() const
{
    // displayName shows on "Open Documents" pane in Navigator
    if ( file_ )
        return file_->fileName();
    return "SequenceEditor";
}

void
SequenceEditor::setDisplayName(const QString &title)
{
    displayName_ = title;
}

bool
SequenceEditor::duplicateSupported() const
{
    return false;
}

Core::IEditor *
SequenceEditor::duplicate(QWidget *parent)
{
    Q_UNUSED( parent );
    return 0;
}

QByteArray
SequenceEditor::saveState() const
{
    return QByteArray();
}

bool
SequenceEditor::restoreState(const QByteArray &state)
{
	Q_UNUSED( state );
	return false;
}

int
SequenceEditor::currentLine() const
{
    return static_cast<int>(currRow_) + 1;
}

int
SequenceEditor::currentColumn() const
{
    return static_cast<int>(currCol_) + 1;
}

bool
SequenceEditor::isTemporary() const
{
	return false;
}

QWidget *
SequenceEditor::toolBar()
{
	return 0;
}
// end Core::IEditor

// implement IContext
QList<int>
SequenceEditor::context() const
{ 
    return context_;
}

QWidget *
SequenceEditor::widget()
{ 
    return widget_;
}

void
SequenceEditor::slotTitleChanged( const QString& title )
{
    setDisplayName( title );
}

////
void
SequenceEditor::setSequence( const adsequence::sequence& sequence )
{
    widget_->setSequence( sequence );
    currRow_ = 0;
    if ( sequence.size() > 0 )
        saveToWidget( currRow_ );
}

void
SequenceEditor::getSequence( adsequence::sequence& sequence ) const
{
    widget_->getSequence( sequence );
}

void
SequenceEditor::getDefault( adcontrols::ProcessMethod& m ) const
{
	MainWindow::instance()->getProcessMethod( m );
}

void
SequenceEditor::getDefault( adcontrols::ControlMethod& m ) const
{
	MainWindow::instance()->getControlMethod( m );
}

void
SequenceEditor::setModified( bool modified )
{
    file_->setModified( modified );
}

void
SequenceEditor::onLineAdded( size_t /* row */)
{
	file_->setModified( true );
}

void
SequenceEditor::onLineDeleted( size_t /* row */)
{
	file_->setModified( true );
}

void
SequenceEditor::onCurrentChanged( size_t row, size_t column )
{
    saveToObject( currRow_ ); // Widget --> IFile
    currRow_ = row;
    currCol_ = column;
    saveToWidget( row ); // IFile --> Widget
}

void
SequenceEditor::saveToObject( size_t row )
{
    std::wstring ctrlname = qtwrapper::wstring( widget_->getControlMethodName( row ) );
    if ( ! ctrlname.empty() ) {
        const adcontrols::ControlMethod * pCM = file_->getControlMethod( ctrlname );
        adcontrols::ControlMethod tmp;
        if ( pCM ) {
            // each device method editor only update own reagin, so existing method should be preserved
            // for all existing methods
            tmp = *pCM;
        }
        MainWindow::instance()->getControlMethod( tmp );

        file_->setControlMethod( ctrlname, tmp );
    }

    std::wstring procname = qtwrapper::wstring( widget_->getProcessMethodName( row ) );
    if ( ! procname.empty() ) {
        const adcontrols::ProcessMethod * pPM = file_->getProcessMethod( procname );
        adcontrols::ProcessMethod tmp;
        if ( pPM )
            tmp = *pPM;
        MainWindow::instance()->getProcessMethod( tmp );
        file_->setProcessMethod( procname, tmp );
    }
}

void
SequenceEditor::saveToWidget( size_t row )
{
    QString qctrlname = widget_->getControlMethodName( row );
    QString qprocname = widget_->getProcessMethodName( row );

    MainWindow::instance()->setControlMethodName( qctrlname );  // GUI updateGUI (middle bar on MainWindow) update
    MainWindow::instance()->setProcessMethodName( qprocname );  // GUI updateGUI (middle bar on MainWindow) update

    std::wstring ctrlname = qtwrapper::wstring( qctrlname );
    if ( ! ctrlname.empty() ) {
        const adcontrols::ControlMethod * p = file_->getControlMethod( ctrlname );
        if ( p ) {
            MainWindow::instance()->setControlMethod( *p );
        }
    }

    std::wstring procname = qtwrapper::wstring( qprocname );
    if ( ! procname.empty() ) {
        const adcontrols::ProcessMethod * p = file_->getProcessMethod( procname );
        if ( p )
            MainWindow::instance()->setProcessMethod( *p );
    }
}
