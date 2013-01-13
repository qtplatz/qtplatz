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

#include "sequencewidget.hpp"
#include "ui_sequencewidget.h"
#include "sequencedelegate.hpp"
#include "sequenceeditor.hpp"
#include "sequencefile.hpp"
#include "mainwindow.hpp"
#include <adcontrols/processmethod.hpp>
#include <adinterface/controlmethodC.h>
#include <adportable/profile.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
#include <qtwrapper/qstring.hpp>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <assert.h>
#include <QMessageBox>
#include <QDebug>

using namespace sequence;

SequenceWidget::SequenceWidget( const adsequence::schema& schema
                                , SequenceEditor& editor
                                , QWidget *parent) : QWidget( parent )
                                                   , ui( new Ui::SequenceWidget )
                                                   , model_( new QStandardItemModel )
                                                   , delegate_( new SequenceDelegate )
                                                   , schema_( new adsequence::schema( schema ) )
                                                   , editor_( editor )
{
    ui->setupUi(this);
    ui->treeView->setModel( model_.get() );
    ui->treeView->setItemDelegate( delegate_.get() );
    ui->treeView->setContextMenuPolicy( Qt::CustomContextMenu );

    assert( connect( ui->treeView, SIGNAL( customContextMenuRequested( const QPoint& ) )
                     , this, SLOT( showContextMenu( const QPoint& ) ) ) );

    assert( connect( ui->treeView, SIGNAL( signalCurrentChanged( const QModelIndex&, const QModelIndex& ) )
                     , this, SLOT( handleCurrentChanged( const QModelIndex&, const QModelIndex& ) ) ) );
}

SequenceWidget::~SequenceWidget()
{
    delete ui;
}

void
SequenceWidget::OnInitialUpdate( const adsequence::schema& schema )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir<char>() );
    dir /= "data";

    ui->lineEditName->setText( ( dir / "sequence.sequ" ).string().c_str() );
    ui->lineEditDataDir->setText( dir.string().c_str() );

    QStandardItem * rootnode = model_->invisibleRootItem();
    size_t ncols = std::distance( schema.begin(), schema.end() );
    rootnode->setColumnCount( ncols );

    for ( adsequence::schema::vector_type::const_iterator it = schema.begin(); it != schema.end(); ++it )
        model_->setHeaderData( std::distance( schema.begin(), it ), Qt::Horizontal, it->display_name().c_str() );

    addLine();
}

void
SequenceWidget::OnFinalClose()
{
}

void
SequenceWidget::setSequenceName( const QString& name )
{
    ui->lineEditName->setText( name );
}

void
SequenceWidget::setDataSaveIn( const QString& dir )
{
    ui->lineEditDataDir->setText( dir );
}

void
SequenceWidget::handleCurrentChanged( const QModelIndex& curr, const QModelIndex& )
{
/*
    // save previous data
    do {
        ControlMethod::Method cmth;
        adcontrols::ProcessMethod pmth;
        int row = prev.row();
        std::wstring ctrlname = qtwrapper::wstring( model_->index( row, 5 ).data().toString() );
        MainWindow::instance()->getControlMethod( cmth );
        // ctrlmethods_[ ctrlname ] = cmth;

        std::wstring procname = qtwrapper::wstring( model_->index( row, 6 ).data().toString() );
        MainWindow::instance()->getProcessMethod( pmth );
        // procmethods_[ procname ] = pmth;

    } while( 0 );
*/
    // update for new line
    do {
        // display method names on center tool bar
        int row = curr.row();
        emit controlMethodSelected( model_->index( row, 5 ).data().toString() );
        emit processMethodSelected( model_->index( row, 6 ).data().toString() );
    } while( 0 );
}

void
SequenceWidget::showContextMenu( const QPoint& pt )
{
    QMenu menu;

    if ( ui->treeView->indexAt( pt ).isValid() ) {
        const adsequence::schema& schema = *schema_;
        QModelIndex index = ui->treeView->currentIndex();
        if ( size_t( index.column() ) < schema.size() ) {
			const adsequence::column& column = schema[ index.column() ];
            if ( column.name() == "name_control" || column.name() == "name_process" ) {
                menu.addAction( "Browse...", this, SLOT( browse() ) );
                menu.addAction( "Save As...", this, SLOT( saveAs() ) );
            }
        }
    }

    menu.addAction( "Add new line", this, SLOT( addLine() ) );
    menu.addAction( "Delete line", this, SLOT( delLine() ) );
	menu.exec( ui->treeView->mapToGlobal( pt ) );

}

void
SequenceWidget::addLine()
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = *schema_;

    size_t row = model.rowCount();
    model.insertRow( row );
    size_t col = 0;
    for ( adsequence::schema::vector_type::const_iterator it = schema.begin(); it != schema.end(); ++it ) {
        if ( it->type() == adsequence::COLUMN_SAMPLE_TYPE ) // samp_type
            model.setData( model.index( row, col++ ), "UNKNOWN" );
        else if ( it->name() == "vial_num" )
            model.setData( model.index( row, col++ ), (boost::format( "%d" ) % (row + 1)).str().c_str() );
        else if ( it->name() == "samp_id" ) 
            model.setData( model.index( row, col++ ), (boost::format( "RUN_%03d" ) % row).str().c_str() );
        else if ( it->name() == "injvol" ) 
            model.setData( model.index( row, col++ ), double( 1.0 ) ); // should be text
        else if ( it->name() == "run_length" ) 
            model.setData( model.index( row, col++ ), double( 1.0 ) ); // should be text
        else if ( it->name() == "name_control" ) 
            model.setData( model.index( row, col++ ), "default.ctrl" ); // should be text
        else if ( it->name() == "name_process" ) 
            model.setData( model.index( row, col++ ), "default.proc" ); // should be text
    }
    editor_.setModified( true );
}

void
SequenceWidget::delLine()
{
    QModelIndex index = ui->treeView->currentIndex();
    model_->removeRows( index.row(), 1 );
    editor_.setModified( true );
}

void
SequenceWidget::browse()
{
	QModelIndex index = ui->treeView->currentIndex();
	QMessageBox::warning( 0, "SequenceWidget::browse", index.data().toString() );
}

void
SequenceWidget::saveAs()
{
	QModelIndex index = ui->treeView->currentIndex();
	QMessageBox::warning( 0, "SequenceWidget::saveAs", index.data().toString() );
}

namespace sequence {
    
    struct cdata_visitor_let : public boost::static_visitor<void> {
        QVariant v_;
        cdata_visitor_let( const QVariant& v ) : v_( v ) {}
        template<typename T> void operator()(T& t) {
            t = v_;
        }
    };
}

void
SequenceWidget::getSequence( adsequence::sequence& seq ) const
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = seq.schema();

    seq.clear();

    for ( int row = 0; row < model.rowCount(); ++row ) {

        adsequence::line_t line;

        for ( int col = 0; col < model.columnCount(); ++col ) {

            QVariant v = model.index( row, col ).data( Qt::EditRole );

            switch ( schema[ col ].type() ) {
            case adsequence::COLUMN_INT:
                line.push_back( v.toInt() );
                break;
            case adsequence::COLUMN_DOUBLE:
                line.push_back( v.toDouble() );
                break;
            case adsequence::COLUMN_VARCHAR:
                line.push_back( qtwrapper::wstring::copy( v.toString() ) );
                break;
            case adsequence::COLUMN_BLOB:
                line.push_back( adsequence::blob() );
                break;
            case adsequence::COLUMN_SAMPLE_TYPE:
                line.push_back( v.toInt() );
                break;
            }

        }
        seq << line;
    }

}

void
SequenceWidget::setSequence( const adsequence::sequence& seq )
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = seq.schema();

    model.removeRows( 0, model.rowCount() );

    for ( size_t row = 0; row < seq.size(); ++row ) {

        model.insertRow( row );

        const adsequence::line_t& line = seq[ row ];
        
        for ( size_t col = 0; col < schema.size(); ++col ) {

            switch ( schema[ col ].type() ) {
            case adsequence::COLUMN_INT:
                model.setData( model.index( row, col++ ), boost::get<int>( line[ col ] ) );
                break;
            case adsequence::COLUMN_DOUBLE:
                model.setData( model.index( row, col++ ), boost::get<double>( line[ col ] ) );
                break;
            case adsequence::COLUMN_VARCHAR:
                model.setData( model.index( row, col++ ), qtwrapper::qstring::copy( boost::get<std::wstring>( line[ col ] ) ) );
                break;
            case adsequence::COLUMN_BLOB:
                break;
            case adsequence::COLUMN_SAMPLE_TYPE:
                model.setData( model.index( row, col++ ), boost::get<int>( line[ col ] ) );
                break;
            } // switch
            
        }
    }
}

