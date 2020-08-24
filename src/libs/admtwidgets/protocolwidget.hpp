/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
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

#pragma once

#include "admtwidgets_global.hpp"
#include <admtcontrols/fwd.hpp>
#include <adwidgets/lifecycle.hpp>
#include <QFrame>
#include <memory>

class QStandardItemModel;
class QModelIndex;

namespace adcontrols { namespace ControlMethod { class MethodItem; } }
namespace admtcontrols { class ScanLaw;  class OrbitProtocol; }

namespace admtwidgets {

    class ADMTWIDGETSSHARED_EXPORT protocolWidget : public QFrame
                                                     , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        ~protocolWidget();
        explicit protocolWidget(QWidget *parent = 0);

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        QSize sizeHint() const override;
        //
        QByteArray toJson( bool pritty = false ) const;
        void fromJson( const QByteArray& );
        void setDirty( bool );

    signals:
        void applyTriggered();
        void fetchTriggered();

    public slots :

    private slots:
        void showContextMenu( const QPoint& );

    private:
        class impl;
        impl * impl_;
    };

}
