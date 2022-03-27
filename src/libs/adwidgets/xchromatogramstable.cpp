/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "xchromatogramstable.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
// #include "tofchromatogramstablehelper.hpp"
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
// #include <adcontrols/tofchromatogramstable.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <QApplication>
#include <QByteArray>
#include <QByteArray>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace adwidgets;

namespace adwidgets {

    class XChromatogramsTable::impl  {
    public:
        impl() {
        }

        ~impl() {
        }
    };
}


XChromatogramsTable::XChromatogramsTable(QWidget *parent) : MolTable(parent)
                                                          , impl_( std::make_unique< impl >() )
{
    // setModel( model_ );
	// setItemDelegate( new delegate( [this]( const QModelIndex& index ){
    //     if ( ! signalsBlocked() )
    //         handleValueChanged( index );
    // } ) );

    // setHorizontalHeader( new HtmlHeaderView );
    // setSortingEnabled( true );
    // setAcceptDrops( true );

    // connect( this, &TableView::rowsDeleted, [this]() {
    //     if ( model_->rowCount() == 0 )
    //         model_->setRowCount( 1 );
    // });

    // setContextMenuPolicy( Qt::CustomContextMenu );
    // connect( this, &QTableView::customContextMenuRequested, this, &XChromatogramsTable::handleContextMenu );

    // model_->setColumnCount( nbrColums );
    // model_->setRowCount( 1 );
    // //setColumnHidden( c_smiles, true );
}

XChromatogramsTable::~XChromatogramsTable()
{
}
