/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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


#include "infitofwidgets_global.hpp"
#include <QFrame>
#include <boost/property_tree/ptree_fwd.hpp>
#include <memory>


namespace infitofwidgets {

    class INFITOFWIDGETSSHARED_EXPORT hvTuneWidget : public QWidget {

        Q_OBJECT
        
    public:
        
        explicit hvTuneWidget( const QString& server, const QString& port = "http", QWidget *parent = 0 );
        ~hvTuneWidget();

        void setServer( const QString& addr );
        QString server() const;

        QString setpts() const;
        void setSetpts( const boost::property_tree::ptree& );
        
    private:
        void setUrl( const QString& url );
        void uiUpdateSectorVoltages();

    private slots:
        void handleReply( const QString&, const QByteArray& );
        void handleSwitchClicked( QObject *, bool  );
        void handleSwitchToggled( QObject *, bool  );
        void handleValueChanged( QObject *, double );
        void handleSectorValueChanged( QObject *, double );
        void handleModeChanged( int );

    signals:
        void dataChanged();

    private:
        class impl;
        std::unique_ptr<impl> impl_;
    };
}

