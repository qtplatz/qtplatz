/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include <QWidget>
#include "multumwidgets_global.hpp"
#include <boost/property_tree/ptree_fwd.hpp>

class QJsonObject;

namespace multumcontrols { class ScanLaw; class OrbitProtocol; }

namespace multumwidgets {

    class MULTUMWIDGETSSHARED_EXPORT protocolForm;

    class protocolForm : public QWidget {

        Q_OBJECT

    public:
        explicit protocolForm( int protocol, QWidget *parent = 0 );
        ~protocolForm();

        // void setScanLaw( double va, double t0 );
        // void setScanLaw( const multumcontrols::ScanLaw& );
        // void setProtocol( int idx, const multumcontrols::OrbitProtocol& );
        void setDirty( bool );
        void setGateWindow( bool, double );
        void getGateWindow( bool&, double& ) const;
        //void setEditorBehavior( const infcontrols::MethodEditorBehavior& );
        //const infcontrols::MethodEditorBehavior& editorBehavior() const;
        // void setTrigRate( uint32_t );
        // uint32_t trigRate() const;

        QJsonObject json() const;
        void setJson( const QJsonObject& );
        void setJson( const boost::property_tree::ptree& );

        // const multumcontrols::ScanLaw * scanLaw() const;
        // const multumcontrols::OrbitProtocol& protocol() const;

    signals:
        void onDataChanged( int hint );
        void editorBehaviorChanged();

    private slots:
        void handleValueChanged( QWidget * );
        void handleCommit( QWidget * );
        void showContextMenu( const QPoint& );

    private:
        class impl;
        impl * impl_;
        uint32_t protocolId_;
    };

}
