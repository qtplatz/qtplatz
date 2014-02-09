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

#include "digestedpeptidetable.hpp"
#include <adprot/protein.hpp>
#include <adprot/protease.hpp>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QItemDelegate>
#include <QPainter>

namespace peptide {
    namespace detail {

        class DigestedPeptideDelegate : public QItemDelegate {
        public:
        private:
        };

    }
}


using namespace peptide;

DigestedPeptideTable::DigestedPeptideTable(QWidget *parent) :  QTableView(parent)
                                            , model_( new QStandardItemModel )
                                            , delegate_( new detail::DigestedPeptideDelegate )
{
    setModel( model_ );
    setItemDelegate( delegate_ );
    init( *model_ );
}

DigestedPeptideTable::~DigestedPeptideTable()
{
    delete delegate_;
    delete model_;
}

void
DigestedPeptideTable::init( QStandardItemModel& model )
{
    model.setColumnCount( 3 );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr("sequence") );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr("mass") );
	setColumnWidth( 0, 200 );
	setWordWrap( true );
}

void
DigestedPeptideTable::setData( const adprot::protein& prot )
{
}

void
DigestedPeptideTable::setData( const std::shared_ptr< adprot::protease >& enzyme )
{
    protease_ = enzyme;
}

