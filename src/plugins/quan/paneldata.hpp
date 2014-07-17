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

#ifndef PANELDATA_HPP
#define PANELDATA_HPP

#include <memory>
#include <QString>
#include <QIcon>

class QWidget;

namespace quan {

    // may be inherit from iPanelData with extensionsystem
    class PanelData : public std::enable_shared_from_this< PanelData > {
    public:
        PanelData();
        QIcon icon() const { return icon_; }
        QWidget * widget() const { return widget_; }
        const QString& displayName() const { return displayName_; }

        void setDisplayName( const QString& );
        void setIcon( const QIcon& );
        void setWidget( QWidget * );
    private:
        QString displayName_;
        QWidget * widget_;
        QIcon icon_;
    };

}

#endif // PANELDATA_HPP
