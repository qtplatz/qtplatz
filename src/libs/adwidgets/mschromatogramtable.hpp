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

#pragma once

#include "tableview.hpp"
#include <adcontrols/constants_fwd.hpp>
#include <adportable/debug.hpp>
#include <tuple>

class QStandardItemModel;
class QMenu;

namespace adcontrols {
    class MSChromatogramMethod;
    class moltable;
}

namespace adwidgets {

    template <typename X, typename Tuple> class index_of;

    template <typename X, typename... T>
    class index_of< X, std::tuple<T...> > {
        template <std::size_t... idx>
        static constexpr ssize_t find_idx(std::index_sequence<idx...>) {
            // return ((std::is_same<X, T>::value ? idx : 0) + ...); // return 0 if not found
            return std::max({static_cast<ssize_t>(std::is_same_v<X, T> ? idx : -1)...}); // return -1, if not found
        }
    public:
        static constexpr ssize_t value = find_idx(std::index_sequence_for<T...>{});
    };

    struct col_formula       { typedef std::string value_type; const QString header = "Formula";       };
    struct col_adducts       { typedef std::string value_type; const QString header = "Adduct";        };
    struct col_mass          { typedef double value_type;      const QString header = "Mass";          };
    struct col_retentionTime { typedef double value_type;      const QString header = "t<sub>R</sub>"; };
    struct col_lockmass      { typedef bool value_type;        const QString header = "lock mass";     };
    struct col_protocol      { typedef int value_type;         const QString header = "protocol";      };
    struct col_synonym       { typedef std::string value_type; const QString header = "Synonym";       };
    struct col_memo          { typedef std::string value_type; const QString header = "Memo";          };
    struct col_svg           { typedef std::string value_type; const QString header = "Structure";     };
    struct col_smiles        { typedef std::string value_type; const QString header = "SMILES";        };

    typedef std::tuple< col_formula
                        , col_adducts
                        , col_mass
                        , col_retentionTime
                        , col_lockmass
                        , col_protocol
                        , col_synonym
                        , col_memo
                        , col_svg
                        , col_smiles > column_list;

    class MSChromatogramTable : public TableView {
        Q_OBJECT
    public:
        explicit MSChromatogramTable(QWidget *parent = 0);
        ~MSChromatogramTable();

        void onInitialUpdate();

        template< typename T > void setColumnHidden( T&&, bool hidden ) {
            TableView::setColumnHidden( index_of< T, column_list >::value, hidden );
        }
        void setValue( const adcontrols::moltable& );

        // void setValue( const adcontrols::MSChromatogramMethod& );
        // void getContents( adcontrols::MSChromatogramMethod& );

    private:

    signals:
        void valueChanged();
        // void editorValueChanged( const QModelIndex&, double );

    public slots:
        void handlePolarity( adcontrols::ion_polarity );
        void handleDataChanged( const QModelIndex&, const QModelIndex& );

    private slots:
        // void handleContextMenu( const QPoint& pt );
        // void handleSetAdducts();

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
