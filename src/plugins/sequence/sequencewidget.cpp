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
#include <adportable/profile.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
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

SequenceWidget::SequenceWidget(QWidget *parent) : QWidget( parent )
                                                , ui( new Ui::SequenceWidget )
                                                , model_( new QStandardItemModel )
                                                , delegate_( new SequenceDelegate )
                                                , sequence_( new adsequence::sequence )
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
SequenceWidget::OnInitialUpdate()
{
    const adsequence::schema& schema = sequence_->schema();

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
SequenceWidget::setSequence( const adsequence::sequence& s )
{
    *sequence_ = s;
    // todo:
    // if schema is not same with previous, then GUI layout should be changed
    // if control & process method configurations were changed, also change dock widget
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
SequenceWidget::handleCurrentChanged( const QModelIndex& curr, const QModelIndex& /* prev */ )
{
    int row = curr.row();
    emit controlMethodSelected( model_->index( row, 5 ).data().toString() );
    emit processMethodSelected( model_->index( row, 6 ).data().toString() );
}

void
SequenceWidget::showContextMenu( const QPoint& pt )
{
    QMenu menu;

    if ( ui->treeView->indexAt( pt ).isValid() ) {
        const adsequence::schema& schema = sequence_->schema();
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
    const adsequence::schema& schema = sequence_->schema();

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
}

void
SequenceWidget::delLine()
{
    QModelIndex index = ui->treeView->currentIndex();
    model_->removeRows( index.row(), 1 );
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
