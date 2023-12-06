/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <QFrame>
#include "adwidgets_global.hpp"
#include "pugrest.hpp"
#include <adplugin/lifecycle.hpp>
#include <adwidgets/lifecycle.hpp>
#include <memory>

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT PUGRestForm : public QFrame {

        Q_OBJECT

    public:

        explicit PUGRestForm(QWidget *parent = 0);
        ~PUGRestForm();

        adcontrols::PUGREST data() const;
        void setData( const adcontrols::PUGREST& );

    public slots:

    signals:
        void dataChanged( const QString& );
        void apply( const QByteArray& );

    private:
        class impl;
        impl * impl_;
    };
}
