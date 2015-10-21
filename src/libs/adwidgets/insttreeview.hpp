/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QWidget>
#include "adwidgets_global.hpp"
#include <QTreeView>

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT InstTreeView : public QTreeView {
        
        Q_OBJECT

    public:
        explicit InstTreeView(QWidget *parent = 0);
        ~InstTreeView();

        void addItem( const QString& key, const QString& displayValue, bool isChecked = true, bool isEnabled = true );
        
        void setChecked( const QString& key, bool checked );
        void setEnabled( const QString& key, bool checked );
        bool checked( const QString& key ) const;
        bool enabled( const QString& key ) const;

        void setInstState( const QString& key, const QString& state_name );

        size_t size() const;
        QString key( size_t idx ) const;
        QString displayValue( const QString& key ) const;

        void setObserverTree( const QString& key, const QString& json );

    signals:
        void stateChanged( const QString& key, bool isChecked );

    private:
        class delegate;
        class impl;
        impl * impl_;
    };

}

