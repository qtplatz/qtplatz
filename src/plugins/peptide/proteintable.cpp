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

#include "proteintable.hpp"
#include <adpeptide/protfile.hpp>
#include <QStandardItemModel>
#include <QItemDelegate>

namespace peptide {
    namespace detail {

        class ProteinTableDelegate : public QItemDelegate {
        public:
            
        };

    }
}

using namespace peptide;

ProteinTable::ProteinTable(QWidget *parent) : QTreeView(parent)
                                            , model_( new QStandardItemModel )
                                            , delegate_( new detail::ProteinTableDelegate )
{
    setModel( model_ );
    setItemDelegate( delegate_ );
    init( *model_ );
}

ProteinTable::~ProteinTable()
{
    delete delegate_;
    delete model_;
}

void
ProteinTable::init( QStandardItemModel& model )
{
    model.setColumnCount( 3 );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr("name") );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr("sequence") );
}

void
ProteinTable::setData( const adpeptide::protfile& file ) 
{
    QStandardItemModel& model = *model_;

    model.setRowCount( static_cast<int>( file.size() ) );
    int row = 0;
    for ( auto& prot: file ) {
        model.setData( model.index( row, 0 ), QString::fromStdString( prot.name() ) );
        model.setData( model.index( row, 1 ), QString::fromStdString( prot.sequence() ) );
		++row;
    }
}

