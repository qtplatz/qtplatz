/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
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

#ifndef ELEMENTALCOMPMODEL_HPP
#define ELEMENTALCOMPMODEL_HPP

#include <QObject>
#include <QAbstractListModel>
#include <adcontrols/elementalcompositionmethod.hpp>

namespace qtwidgets {

    class ElementalCompModel : public QAbstractListModel {
        Q_OBJECT
        Q_ENUMS( ElecronMode )
        Q_PROPERTY( ElectronMode electronMode READ electronMode WRITE electronMode NOTIFY valueChanged )
        Q_PROPERTY( bool toleranceInPpm READ toleranceInPpm WRITE toleranceInPpm NOTIFY valueChanged )
        Q_PROPERTY( double tolerance_ppm READ tolerance_ppm WRITE tolerance_ppm NOTIFY valueChanged )
        Q_PROPERTY( double tolerance_mDa READ tolerance_mDa WRITE tolerance_mDa NOTIFY valueChanged )
        Q_PROPERTY( double dbeMinimum READ dbeMinimum WRITE dbeMinimum NOTIFY valueChanged )
        Q_PROPERTY( double dbeMaximum READ dbeMaximum WRITE dbeMaximum NOTIFY valueChanged )
        Q_PROPERTY( size_t numResults READ numResults WRITE numResults NOTIFY valueChanged )
    public:
        explicit ElementalCompModel(QObject *parent = 0);

        enum ElectronMode {
            Even = adcontrols::ElementalCompositionMethod::Even
            , Odd = adcontrols::ElementalCompositionMethod::Odd
            , OddEven = adcontrols::ElementalCompositionMethod::OddEven
        };

        enum Roles {
            AtomRole
            , MinimumRole
            , MaximumRole
        };

        ElectronMode electronMode() const;
        void electronMode( ElectronMode );
        bool toleranceInPpm() const;
        void toleranceInPpm( bool );
        double tolerance_ppm() const;
        void tolerance_ppm( double );
        double tolerance_mDa() const;
        void tolerance_mDa( double );
        double dbeMinimum() const;
        void dbeMinimum( double );
        double dbeMaximum() const;
        void dbeMaximum( double );
        size_t numResults() const;
        void numResults( size_t );

        // QAbstractListModel
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
        Qt::ItemFlags flags( const QModelIndex& index ) const;
        //---------------------
        const adcontrols::ElementalCompositionMethod& method() const { return method_; }

        //void insertRow( const QModelIndex& index = QModelIndex() );
        Q_INVOKABLE void appendRow( int currentRow );
        Q_INVOKABLE void removeRow( int currentRow );
        Q_INVOKABLE void setProperty( int rowIndex, const QString& role, const QVariant& value );
        
    signals:
        void valueChanged();

    public slots:

    private:
        adcontrols::ElementalCompositionMethod method_;
    };
    
}

#endif // ELEMENTALCOMPMODEL_HPP
