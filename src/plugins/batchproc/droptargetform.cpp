/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "droptargetform.hpp"
#include "ui_droptargetform.h"
#include <adcontrols/datafile.hpp>
#include <QList>
#include <QStandardItemModel>
#include <boost/filesystem.hpp>

using namespace batchproc;

DropTargetForm::DropTargetForm(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::DropTargetForm)
                                                , model_( new QStandardItemModel )
{
    ui->setupUi(this);
    ui->treeView->setModel( model_.get() );
    connect( ui->dropWidget, SIGNAL( dropFiles( const QList<QUrl>& ) ), this, SLOT( handleDropFiles( const QList<QUrl>& ) ) );
}

DropTargetForm::~DropTargetForm()
{
    delete ui;
}

void
DropTargetForm::handleDropFiles( const QList< QUrl >& list )
{
    dropfiles_.clear();

    model_->setRowCount( list.size() );
    model_->setColumnCount( 2 );
    QStandardItemModel& model = *model_;

    int row = 0;
    for ( auto& item: list ) {

        model.setData( model.index( row, 0 ), row + 1 );
        model.setData( model.index( row, 1 ), item.toLocalFile() );

        QModelIndex parent = model.index( row, 0 );
        model.insertColumns( 0, 2, parent );

        boost::filesystem::path path( item.toLocalFile().toStdWString() );

        if ( boost::filesystem::is_regular_file( path ) ) {

            std::wstring dropfile;

            if ( path.parent_path().extension() == L".d" )
                dropfile = path.parent_path().wstring();
            else if ( adcontrols::datafile::access( path.wstring() ) )
                dropfile = path.wstring();

            // duplicate check
            if ( ! dropfile.empty() && std::find( dropfiles_.begin(), dropfiles_.end(), dropfile ) == dropfiles_.end() ) {
                dropfiles_.push_back( dropfile );
                model.insertRow( model.rowCount( parent ), parent );
                model.setData( model.index( 0, 0, parent ), QVariant::fromValue( dropfiles_.size() ) );
                model.setData( model.index( 0, 1, parent ), QString::fromStdWString( dropfiles_.back() ) );
            }

        } else if ( boost::filesystem::is_directory( path ) ) {

            if ( path.extension() == L".d" ) {

                dropfiles_.push_back( path.wstring() );

				model.insertRow( model.rowCount( parent ), parent );
                model.setData( model.index( 0, 0, parent ), QVariant::fromValue( dropfiles_.size() ) );
                model.setData( model.index( 0, 1, parent ), QString::fromStdWString( dropfiles_.back() ) );

            } else {
                int r = 0;
                boost::filesystem::directory_iterator end;
                for ( boost::filesystem::directory_iterator dir( path ); dir != end; ++dir ) {
                    if ( boost::filesystem::is_directory( dir->status() ) && dir->path().extension() == L".d" ) {

                        dropfiles_.push_back( dir->path().wstring() );

						model.insertRow( model.rowCount( parent ), parent );
                        model.setData( model.index( r, 0, parent ), QVariant::fromValue( dropfiles_.size() ) );
                        model.setData( model.index( r, 1, parent ), QString::fromStdWString( dropfiles_.back() ) );
                        ++r;
                    }
                }
            }

        }

		++row;
    }
    QList< QString > files;
    for ( auto& file: dropfiles_ )
        files.push_back( QString::fromStdWString( file ) );
    emit dropped( files );
}

const std::vector< std::wstring >&
DropTargetForm::dropped_files() const
{
    return dropfiles_;
}
