/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef U5303AFORM_HPP
#define U5303AFORM_HPP

#include "constants.hpp"
#include <QWidget>

namespace acqrscontrols { namespace u5303a { class method; } }

namespace acqrswidgets {

    namespace Ui {
        class u5303AForm;
    }

    class u5303AForm : public QWidget {
        Q_OBJECT

    public:
        explicit u5303AForm( QWidget *parent = 0 );
        ~u5303AForm();

        void onInitialUpdate();

        void setContents( const acqrscontrols::u5303a::method& );

        void getContents( acqrscontrols::u5303a::method& );

        void onHandleValue( idCategory, int, const QVariant& );

    signals:
        void valueChanged( idCategory, int channel, const QVariant& );
        
    private:
        Ui::u5303AForm *ui;
        double sampRate_;  // Hz
    };

}

#endif // U5303AFORM_HPP
