/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QString>
#include <QVariant>

namespace adwidgets {

    struct ColumnState {

        enum fields {
            f_any
            , f_formula
            , f_adducts
            , f_mass
            , f_msref
            , f_abundance
            , f_synonym
            , f_description
            , f_svg
            , f_smiles
            , f_time
        };
            
        fields field;
        bool isEditable;
        bool isCheckable;
        int precision;
        std::vector< std::pair< QString, QVariant > > choice;
        
        inline bool isChoice() const { return !choice.empty(); }
        
        ColumnState( fields f = f_any
                     , bool editable = true
                     , bool checkable = false ) : field( f ), isEditable( editable ), isCheckable( checkable )
                                                , precision( 0 ) {
        }

        ColumnState( const ColumnState& t ) : field( t.field )
                                            , isEditable( t.isEditable )
                                            , isCheckable( t.isCheckable )
                                            , precision( t.precision )
                                            , choice( t.choice) {
        }
    };

}

