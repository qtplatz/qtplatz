/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DUALSPINSLIDER_HPP
#define DUALSPINSLIDER_HPP

#include <QWidget>
#include "adwidgets_global.hpp"

class QString;
class QStringList;

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT DualSpinSlider : public QWidget
    {
        Q_OBJECT

        DualSpinSlider( const DualSpinSlider& ) = delete;
        DualSpinSlider& operator = ( const DualSpinSlider& ) = delete;

    public:
        ~DualSpinSlider();
        explicit DualSpinSlider(QWidget *parent = 0);
        DualSpinSlider( Qt::Orientation orientation, QWidget * parent = 0 );

        enum idPair { idFirst, idSecond };

        void setTitles( const QString&, const QStringList& );
        void setRange( idPair, const QPair<double, double>& );
        void setActualValues( const QPair<double, double>& );
        void setValue( idPair, double );
        void setValues( const QPair<double, double>& );
        double value( idPair ) const;
        QPair<double, double> values() const;

    private:
        class impl;
        impl * impl_;

    signals:
        void valueChanged( int, double );

    public slots:

    };

}

#endif // DUALSPINSLIDER_HPP
