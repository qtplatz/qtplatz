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

#ifndef PANELSWIDGET_HPP
#define PANELSWIDGET_HPP

#include <QScrollArea>
#include <QGridLayout>
#include <vector>
#include <memory>

namespace quan {

    class PanelData;

    class PanelsWidget : public QScrollArea  {
        Q_OBJECT
    public:
        PanelsWidget( QWidget * parent );
        ~PanelsWidget();
        void addPanel( PanelData * );
        void addPanel( std::shared_ptr< PanelData >& );
        void commit();
    private:
        void addPanelWidget( PanelData *, int row );
        std::vector< std::shared_ptr< PanelData > > panels_;
        QGridLayout * layout_;
        QWidget * root_;
    signals:
        void onCommit();
    };

}

#endif // PANELSWIDGET_HPP
