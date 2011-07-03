/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef ISOTOPEMETHODMODEL_HPP
#define ISOTOPEMETHODMODEL_HPP

#include <adcontrols/isotopemethod.hpp>
#include <QAbstractListModel>

namespace qtwidgets {

    class IsotopeMethodModel : public QAbstractListModel  {
        Q_OBJECT
        Q_PROPERTY( bool polarityPositive READ polarityPositive WRITE polarityPositive NOTIFY valueChanged )
        Q_PROPERTY( bool useElectronMass READ useElectronMass WRITE useElectronMass NOTIFY valueChanged )
        Q_PROPERTY( double resolution READ resolution WRITE resolution NOTIFY valueChanged )
        Q_PROPERTY( double threshold READ threshold WRITE threshold NOTIFY valueChanged )
    public:

        enum Roles {
            FormulaRole = Qt::UserRole + 1 // std::wstring formula;
            , AdductRole // std::wstring adduct;
            , ChargeRole // size_t chargeState;
            , AmountsRole // double relativeAmounts;
        };

        explicit IsotopeMethodModel(QObject *parent = 0);
        
        bool polarityPositive() const;
        void polarityPositive( bool );
        
        bool useElectronMass() const;
        void useElectronMass( bool );
        
        double threshold() const;
        void threshold( double );
        
        double resolution() const;
        void resolution( double );

        // QAbstractListModel
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
        Qt::ItemFlags flags( const QModelIndex& index ) const;
        //---------------------
        const adcontrols::IsotopeMethod& method() const { return method_; }

        //void insertRow( const QModelIndex& index = QModelIndex() );
        Q_INVOKABLE void appendRow();

        void appendFormula( const adcontrols::IsotopeMethod::Formula& );

    signals:
        void valueChanged();
                           
    public slots:


    private:
        adcontrols::IsotopeMethod method_;
    };

}

#endif // ISOTOPEMETHODMODEL_HPP
