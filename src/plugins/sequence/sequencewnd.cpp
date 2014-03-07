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

#include "sequencewnd.hpp"
#include "ui_sequencewidget.h"
#include "sequencedelegate.hpp"
#include "sequenceeditor.hpp"
#include "sequencefile.hpp"
#include "mainwindow.hpp"
#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
#include <qtwrapper/qstring.hpp>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <QMessageBox>
#include <QDebug>

using namespace sequence;

SequenceWnd::SequenceWnd( const adsequence::schema& schema
                                , QWidget *parent) : QWidget( parent )
                                                   , ui( new Ui::SequenceWidget )
                                                   , model_( new QStandardItemModel )
                                                   , delegate_( new SequenceDelegate )
                                                   , schema_( new adsequence::schema( schema ) )
{
    ui->setupUi(this);
    ui->treeView->setModel( model_.get() );
    ui->treeView->setItemDelegate( delegate_.get() );
    ui->treeView->setContextMenuPolicy( Qt::CustomContextMenu );

    bool res;
    res = connect( ui->treeView, SIGNAL( customContextMenuRequested( const QPoint& ) )
                   , this, SLOT( showContextMenu( const QPoint& ) ) );
    assert( res );

    res = connect( ui->treeView, SIGNAL( signalCurrentChanged( const QModelIndex&, const QModelIndex& ) )
                   , this, SLOT( handleCurrentChanged( const QModelIndex&, const QModelIndex& ) ) );
    assert( res );
}

SequenceWnd::~SequenceWnd()
{
    delete ui;
}

void
SequenceWnd::OnInitialUpdate( const adsequence::schema& schema )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir<char>() );
    dir /= "data";
    dir /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );

    ui->lineEditName->setText( ( dir / "sequence.sequ" ).string().c_str() );
    ui->lineEditDataDir->setText( dir.string().c_str() );

    QStandardItem * rootnode = model_->invisibleRootItem();
    size_t ncols = std::distance( schema.begin(), schema.end() );
    rootnode->setColumnCount( static_cast<int>(ncols) );

    for ( adsequence::schema::vector_type::const_iterator it = schema.begin(); it != schema.end(); ++it )
        model_->setHeaderData( std::distance( schema.begin(), it ), Qt::Horizontal, it->display_name().c_str() );
    
    for ( int i = 0; i < 3; ++i )
        addLine();
}

void
SequenceWnd::OnFinalClose()
{
}

void
SequenceWnd::setSequenceName( const QString& name )
{
    ui->lineEditName->setText( name );
}

void
SequenceWnd::setDataSaveIn( const QString& dir )
{
    ui->lineEditDataDir->setText( dir );
}

void
SequenceWnd::handleCurrentChanged( const QModelIndex& curr, const QModelIndex& )
{
    emit currentChanged( curr.row(), curr.column() );
}

void
SequenceWnd::showContextMenu( const QPoint& pt )
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
SequenceWnd::addLine()
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = *schema_;

    int row = model.rowCount();
    model.insertRow( row );
    int col = 0;
    for ( adsequence::schema::vector_type::const_iterator it = schema.begin(); it != schema.end(); ++it ) {
        if ( it->type() == adsequence::COLUMN_SAMPLE_TYPE ) // samp_type
            model.setData( model.index( row, col++ ), int( adsequence::SAMPLE_TYPE_UNKNOWN ) );
        else if ( it->name() == "vial_num" )
            model.setData( model.index( row, col++ ), (boost::format( "CStk0:0-%03d" ) % (row + 1)).str().c_str() );
        else if ( it->name() == "samp_id" ) 
            model.setData( model.index( row, col++ ), (boost::format( "RUN_%03d" ) % row).str().c_str() );
        else if ( it->name() == "injvol" ) 
            model.setData( model.index( row, col++ ), double( 1.0 ) );
        else if ( it->name() == "run_length" ) 
            model.setData( model.index( row, col++ ), double( 1.0 ) );
        else if ( it->name() == "name_control" ) 
            model.setData( model.index( row, col++ ), (boost::format( "default-%02d.cmth" ) % (row + 1) ).str().c_str() );
        else if ( it->name() == "name_process" ) 
            model.setData( model.index( row, col++ ), (boost::format( "default-%02d.pmth" ) % (row + 1) ).str().c_str() );
    }
    emit lineAdded( row );
}

void
SequenceWnd::delLine()
{
    QModelIndex index = ui->treeView->currentIndex();
    model_->removeRows( index.row(), 1 );
    emit lineDeleted( index.row() );
}

void
SequenceWnd::browse()
{
    QModelIndex index = ui->treeView->currentIndex();
    QMessageBox::warning( 0, "SequenceWnd::browse", index.data().toString() );
}

void
SequenceWnd::saveAs()
{
    QModelIndex index = ui->treeView->currentIndex();
    QMessageBox::warning( 0, "SequenceWnd::saveAs", index.data().toString() );
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
SequenceWnd::getSequence( adsequence::sequence& seq ) const
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
				line.push_back( v.toString().toStdString() );
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
SequenceWnd::setSequence( const adsequence::sequence& seq )
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = seq.schema();

    model.removeRows( 0, model.rowCount() );

    for ( int row = 0; row < static_cast<int>(seq.size()); ++row ) {

        model.insertRow( row );

        const adsequence::line_t& line = seq[ row ];
        
        for ( int col = 0; col < static_cast<int>(schema.size()); ++col ) {
            switch ( schema[ col ].type() ) {
            case adsequence::COLUMN_INT:
                model.setData( model.index( row, col ), boost::get<int>( line[ col ] ) );
                break;
            case adsequence::COLUMN_DOUBLE:
                model.setData( model.index( row, col ), boost::get<double>( line[ col ] ) );
                break;
            case adsequence::COLUMN_VARCHAR:
            {
                std::string varchar = boost::get< std::string >( line[ col ] );
				model.setData( model.index( row, col ), QString::fromStdString( varchar ) );
            }
            break;
            case adsequence::COLUMN_SAMPLE_TYPE:
                model.setData( model.index( row, col ), boost::get<int>( line[ col ] ) );
                break;
            } // switch
            
        }
    }
}

QString
SequenceWnd::getControlMethodName( size_t row ) const
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = *schema_;

    adsequence::schema::vector_type::const_iterator it
        = std::find_if( schema.begin(), schema.end(), boost::bind(&adsequence::column::name, _1 ) == "name_control" );
    if ( it != schema.end() ) {
        size_t col = std::distance( schema.begin(), it );
        return model.index( static_cast<int>(row), static_cast<int>(col) ).data( Qt::EditRole ).toString();
    }
    return QString();
}

QString
SequenceWnd::getProcessMethodName( size_t row ) const
{
    QStandardItemModel& model = *model_;
    const adsequence::schema& schema = *schema_;

    adsequence::schema::vector_type::const_iterator it
        = std::find_if( schema.begin(), schema.end(), boost::bind(&adsequence::column::name, _1 ) == "name_process" );
    if ( it != schema.end() ) {
        size_t col = std::distance( schema.begin(), it );
        return model.index( static_cast<int>(row), static_cast<int>(col) ).data( Qt::EditRole ).toString();
    }
    return QString();
}
