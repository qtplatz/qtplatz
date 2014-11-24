/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <QObject>
#include "adextension_global.hpp"

namespace adextension {

    class iWidgetFactory;

    class ADEXTENSIONSHARED_EXPORT iDataproc : public QObject {
        Q_OBJECT
    public:
        class factory_iterator {
            size_t pos_;
            iDataproc& t_;
        public:
            factory_iterator( iDataproc& t, size_t pos ) : pos_( pos ), t_( t ) {}
            const factory_iterator& operator ++ () { ++pos_; return *this; }
            const bool operator != ( const factory_iterator& rhs ) const { return pos_ != rhs.pos_; }
            operator iWidgetFactory* () const { return &t_[pos_]; }
        };

        typedef factory_iterator iterator;
        typedef iWidgetFactory& reference;

        iDataproc( QObject * parent = 0 );

        virtual size_t size() const = 0;
        virtual reference operator []( size_t idx ) = 0;

        inline iterator begin() { return factory_iterator( *this, 0 ); }
        inline iterator end() { return factory_iterator( *this, size() ); }
    };

}
