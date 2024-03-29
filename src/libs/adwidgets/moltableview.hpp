/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include "columnstate.hpp"
#include <functional>
#include <memory>

class QMenu;

namespace adcontrols { class moltable; }

// this class is deplicated
// so far it has been inherit from mschromatogramwidget, tofchromatogramswidget, adtracewidget, countingwidget and chemistry/moltablewnd

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MolTableView : public TableView  {

        Q_OBJECT

    public:
        explicit MolTableView( QWidget *parent = 0 );
        ~MolTableView();

        void onInitialUpdate();

        void setColumnField( int column, ColumnState::fields f, bool editable = true, bool checkable = false );
        const ColumnState& columnState( int column ) const;
        void setColumnEditable( int column, bool );
        bool isColumnEditable( int column ) const;
        bool isColumnCheckable( int column ) const;
        void setContextMenuHandler( std::function<void(const QPoint& )> );
        void setChoice( int column, const std::vector< std::pair< QString, QVariant > >& );
        void setPrecision( int column, int prec );

        std::string json() const;
        void setContents( const std::string& json );
        bool setContents( const adcontrols::moltable& );
        bool getContents( adcontrols::moltable& ) const;

        // static double getMonoIsotopicMass( const QString& formula, const QString& adducts = QString() );

    private:
        class delegate;
        class impl;
        std::unique_ptr< impl > impl_;

        void handleValueChanged( const QModelIndex& );
        void handleContextMenu( const QPoint& );

        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;

    signals:
        void onContextMenu( QMenu&, const QPoint& );
        void valueChanged( const QModelIndex&, double ); // Editor 'QDoubleSpinBox' value changed
        void stateChanged( const QModelIndex&, Qt::CheckState );

    public slots:
        void handleCopyToClipboard() override;
        void handlePaste() override;
    };

}
