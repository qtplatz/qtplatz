/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef FINDSLOPEFORM_HPP
#define FINDSLOPEFORM_HPP

#include "adwidgets_global.hpp"
#include <QWidget>

namespace adcontrols { class threshold_method; }

namespace adwidgets {

    namespace Ui {
        class findSlopeForm;
    }

    class ADWIDGETSSHARED_EXPORT findSlopeForm : public QWidget   {

        Q_OBJECT

    public:
        explicit findSlopeForm(QWidget *parent = 0);
        ~findSlopeForm();

        void setTitle( int ch, const QString& );
        int channel() const;

        bool isChecked() const;
        void setChecked( bool );

        void set( const adcontrols::threshold_method& );
        void get( adcontrols::threshold_method& ) const;
        void setJson( const QByteArray& );
        QByteArray readJson() const;

        static QByteArray toJson( const adcontrols::threshold_method& );
        static bool fromJson( const QByteArray&, adcontrols::threshold_method& );

    signals:
        void valueChanged( int ch );

    private:
        Ui::findSlopeForm *ui;
        int channel_;
    };
}

#endif // FINDSLOPEFORM_HPP
