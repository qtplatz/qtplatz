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
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
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
        impl() : model_( new QStandardItemModel ) {
        }

        ~impl() {
        }

        QStandardItemModel *model_;
    };
}

namespace {
    enum columns { c_id, c_formula, c_adducts, c_mass, c_masswindow, c_time, c_timewindow, c_algo, c_protocol, ncolumns };
}


XChromatogramsTable::XChromatogramsTable(QWidget *parent) : TableView(parent)
                                                          , impl_( std::make_unique< impl >() )
{
    impl_->model_->setColumnCount( ncolumns );
    impl_->model_->setRowCount( 8 );

    setModel( impl_->model_ );
	// setItemDelegate( new delegate( [this]( const QModelIndex& index ){
    //     if ( ! signalsBlocked() )
    //         handleValueChanged( index );
    // } ) );

    setHorizontalHeader( new HtmlHeaderView );
    setSortingEnabled( false );
    // setAcceptDrops( true );

    // connect( this, &TableView::rowsDeleted, [this]() {
    //     if ( model_->rowCount() == 0 )
    //         model_->setRowCount( 1 );
    // });

    // setContextMenuPolicy( Qt::CustomContextMenu );
    // connect( this, &QTableView::customContextMenuRequested, this, &XChromatogramsTable::handleContextMenu );

    // //setColumnHidden( c_smiles, true );
}

XChromatogramsTable::~XChromatogramsTable()
{
}

void
XChromatogramsTable::onInitialUpdate()
{
    auto model = impl_->model_;
    model->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
    model->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
    model->setHeaderData( c_adducts,    Qt::Horizontal, QObject::tr( "Adducts" ) );
    model->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
    model->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
    model->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(&mu;s)" ) );
    model->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method" ) );
    model->setHeaderData( c_protocol,   Qt::Horizontal, QObject::tr( "Protocol#" ) );
}

void
XChromatogramsTable::setValue( const adcontrols::TofChromatogramsMethod& xm )
{
    auto model = impl_->model_;
    size_t row(0);
    for ( const auto& m: xm ) {
        model->setData( model->index( row, c_formula ), QString::fromStdString( m.formula() ) );
        if ( auto item = model->item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model->setData( model->index( row, c_formula ), m.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        model->setData( model->index( row, c_mass ), m.mass() );
        model->setData( model->index( row, c_masswindow ), m.massWindow() );
        model->setData( model->index( row, c_time ), m.time() * std::micro::den );
        model->setData( model->index( row, c_timewindow ), m.timeWindow() * std::micro::den );
        model->setData( model->index( row, c_algo ), m.intensityAlgorithm() );
        model->setData( model->index( row, c_protocol ), m.protocol() );
        ++row;
    }
}
