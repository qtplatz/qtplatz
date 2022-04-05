/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adwidgets_global.hpp"
#include "moltablecolumns.hpp"
#include <adportable/optional.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <QAbstractItemModel>
#include <QMetaType>
#include <QString>
#include <vector>
#include <tuple>

class QUrl;
class QClipboard;
class QByteArray;

namespace adcontrols { class moltable; }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MolTableHelper;

    class MolTableHelper {
    public:
        struct SmilesToSVG {
            adportable::optional< std::tuple< QString, QByteArray > > // formula, svg
            operator()( const QString& smiles ) const;
        };

        struct SDMolSupplier {
            typedef std::tuple< QString, QString, QByteArray > value_type; // formula,smiles,svg

            std::vector< value_type > operator()( const QUrl& ) const;           // drag&drop
            std::vector< value_type > operator()( const QClipboard* ) const;     // paste
        };
        static double monoIsotopicMass( const QString& formula, const QString& adducts = {} );

        static adportable::optional< std::pair<double, double> > logP( const QString& smiles );
        static adportable::optional< adcontrols::moltable > paste();
    };

    namespace moltable {

        std::tuple< double, QString > computeMass( const QString& formula, const QString& adducts );

        ///// set headerData helper
        template< class Tuple, std::size_t... Is> void
        setHeaderDataImpl( QAbstractItemModel * model
                           , const Tuple& t, std::index_sequence<Is...>) {
            ( (model->setHeaderData( Is, Qt::Horizontal, std::get<Is>(t).header )), ... );
        }

        template< typename... Args > void
        setHeaderData( QAbstractItemModel * model
                       , const std::tuple< Args ...>&& args) {
            setHeaderDataImpl( model, args, std::index_sequence_for<Args...>{} );
        }

        //------------------------------------------------------
        //--------------- clipboard copy helper ----------------
        //------------------------------------------------------
        // equal_range will be supported on c++20
        template< typename Iterator, typename T, typename Compare > std::pair< Iterator, Iterator >
        equal_range( Iterator first, Iterator last, const T& value, Compare comp ) {
            return std::make_pair( std::lower_bound( first, last, value, comp )
                                   , std::upper_bound( first, last, value, comp ) );
        }

        //------------------------------------------------------
        //------------------------------------------------------

        template< typename T > void __assign( T& t, const QModelIndex& index, adcontrols::moltable::value_type& value ) {
        };

        template<> void __assign( moltable::col_formula& t,       const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_adducts& t,       const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_mass& t,          const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_retentionTime& t, const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_msref& t,         const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_protocol& t,      const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_synonym& t,       const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_memo& t,          const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_smiles& t,        const QModelIndex& index, adcontrols::moltable::value_type& value );
        // template<> void __assign( moltable::col_nlaps& t,         const QModelIndex& index, adcontrols::moltable::value_type& value );
        template<> void __assign( moltable::col_abundance& t,     const QModelIndex& index, adcontrols::moltable::value_type& value );
        // template<> void __assign( moltable::col_logp& t,          const QModelIndex& index, adcontrols::moltable::value_type& value );
        // template<> void __assign( moltable::col_apparent_mass& t, const QModelIndex& index, adcontrols::moltable::value_type& value );
        // template<> void __assign( moltable::col_tof& t,           const QModelIndex& index, adcontrols::moltable::value_type& value );

        //------------------------------------------------------
        namespace copy_detail {
            typedef std::pair< const QModelIndexList::const_iterator, const QModelIndexList::const_iterator > QModelIndexRange;

            template< size_t Is >
            bool if_contains( const QModelIndexRange& range ) {
                return std::find_if( range.first, range.second, [](const auto& a){ return a.column() == Is; }) != range.second;
            }

            template< size_t Is, typename T >
            void assign( T& t, const QModelIndexRange& range, adcontrols::moltable::value_type& value ) {
                auto it = std::find_if( range.first, range.second, [](const auto& a){ return a.column() == Is; });
                if ( it != range.second ) {
                    __assign( t, *it, value );
                }
            }

            template< typename Tuple, std::size_t... Is >
            adcontrols::moltable::value_type value_from( const QModelIndexRange& range
                                                         , Tuple&& tag
                                                         , std::index_sequence<Is...> ) {
                adcontrols::moltable::value_type value;
                (( assign<Is>( std::get<Is>( tag ), range, value ) ), ... );
                return value;
            }
        }

        // argument indices must be sorted in advance!!
        template< typename... Args >
        adcontrols::moltable copy( const QModelIndexList& indices, std::tuple< Args... >&& ) {
            using namespace copy_detail;
            std::pair< QModelIndexList::const_iterator, QModelIndexList::const_iterator > range{ indices.begin(), {} };
            adcontrols::moltable mols;
            while ( range.first != indices.end() ) {
                range = equal_range( indices.begin(), indices.end(), *range.first, [](const auto& a, const auto& b){ return a.row() < b.row(); });
                if ( range.first != indices.end() ) {
                    mols << value_from( range, std::tuple<Args...>{}, std::index_sequence_for< Args... >{} );
                }
                range.first = range.second;
            }
            return mols;
        }

        namespace value_from_detail {

            template< size_t Is, typename T >
            void assign( T&& t, const QModelIndex& index, adcontrols::moltable::value_type& value ) {
                __assign( t, index, value );
            }

            template< typename Tuple, std::size_t... Is >
            adcontrols::moltable::value_type value_from_impl( const QAbstractItemModel * model
                                                              , int row
                                                              , Tuple&& tag
                                                              , std::index_sequence<Is...> ) {
                adcontrols::moltable::value_type value;
                (( assign<Is>( std::get<Is>( tag ), model->index( row, Is ), value ) ), ... );
                return value;
            }
        }

        template< typename... Args >
        adcontrols::moltable::value_type value_from( const QAbstractItemModel * model, int row, std::tuple< Args... >&& ) {
            using namespace value_from_detail;
            return value_from_impl( model, row, std::tuple<Args...>{}, std::index_sequence_for< Args... >{} );
        }
    }

}
