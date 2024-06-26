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

#include <memory>

class QWidget;
class QString;

namespace adextension {

    template<class Factory, class T> class factory_type : public Factory {
        QString title_;
        QString objname_;
    public:
        factory_type( const QString& title, const QString& objname ) : title_( title ), objname_( objname ) {
        }
        QWidget * operator () ( QWidget * parent ) override {
            return new T( parent );
        }
        QString title() const override {
            return title_;
        }
        QString objname() const override {
            return objname_;
        }
    };

}

