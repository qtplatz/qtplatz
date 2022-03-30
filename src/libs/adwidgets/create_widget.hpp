/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

class QWidget;

namespace adwidgets {

    template<typename _Ty, typename... _Types >
    inline _Ty * create_widget( const char * ident, _Types&&... _Args )
    {
        auto w = new _Ty( std::forward<_Types>( _Args )... );
        if ( ident && *ident )
            w->setObjectName( ident );
        return w;
    }

    template<typename _Ty, typename _P, typename... _Types >
    inline _Ty * add_widget( _P * p, _Ty * w, _Types&&... _Args )
    {
        p->addWidget( w, std::forward<_Types>( _Args )... );
        return w;
    }

}
