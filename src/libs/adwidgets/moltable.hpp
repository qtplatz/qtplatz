/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MOLTABLE_HPP
#define MOLTABLE_HPP

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include "moltablecolumns.hpp"
#include <adcontrols/constants_fwd.hpp>
#include <adportable/index_of.hpp>

class QStandardItemModel;
class QMenu;

namespace adcontrols { class moltable; }

namespace adwidgets {

    using namespace moltable; // moltablecolumns

    class ADWIDGETSSHARED_EXPORT  MolTable;

    class MolTable : public TableView  {
        Q_OBJECT
    public:
        explicit MolTable(QWidget *parent = 0);
        ~MolTable();

        typedef std::tuple<
            col_formula
            , col_adducts
            , col_mass
            , col_nlaps
            , col_apparent_mass
            , col_tof
            , col_msref
            , col_abundance
            , col_synonym
            , col_svg
            , col_smiles
            , col_logp
            , col_memo
            > column_list;

        // for compatiblity to old code
        enum fields {
            c_formula         = adportable::index_of< col_formula,   column_list >::value
            , c_adducts       = adportable::index_of< col_adducts,   column_list >::value
            , c_mass          = adportable::index_of< col_mass,      column_list >::value
            , c_nlaps         = adportable::index_of< col_nlaps,     column_list >::value
            , c_apparent_mass = adportable::index_of< col_apparent_mass, column_list >::value
            , c_time          = adportable::index_of< col_tof,       column_list >::value
            , c_msref         = adportable::index_of< col_msref,     column_list >::value
            , c_abundance     = adportable::index_of< col_abundance, column_list >::value
            , c_synonym       = adportable::index_of< col_synonym,   column_list >::value
            , c_svg           = adportable::index_of< col_svg,       column_list >::value
            , c_smiles        = adportable::index_of< col_smiles,    column_list >::value
            , c_logp          = adportable::index_of< col_logp,      column_list >::value
            , c_description   = adportable::index_of< col_memo,      column_list >::value
        };

        static constexpr const size_t ncolumns = std::tuple_size< column_list >();

        void onInitialUpdate();

        void setContents( const adcontrols::moltable& );
        void getContents( adcontrols::moltable& );

        void setColumnEditable( int column, bool );
        bool isColumnEditable( int column ) const;

        void setColumHide( const std::vector< std::pair< fields, bool > >& hides );

        template< typename T > void setColumnHidden( T&&, bool hidden ) {
            TableView::setColumnHidden( adportable::index_of< T, column_list >::value, hidden );
        }

    private:
        class impl;
        impl * impl_;

        void handleDataChanged( const QModelIndex&, const QModelIndex& );
        void handleContextMenu( const QPoint& );
        void enable_all( bool );
        void setData( int row, const QString& formula, const QString& smiles, const QByteArray& svg );

        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;

    signals:
        void onContextMenu( QMenu&, const QPoint& );
        void onValueChanged();

    public slots:
        void handlePolarity( adcontrols::ion_polarity );

    private slots:
        void handleCopyToClipboard() override;
        void handlePaste() override;
        void handleSetAdducts();
    };

}

#endif // MOLTABLE_HPP
