/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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

#pragma once

#include "adwidgets_global.hpp"
#include <QtCore/qbytearray.h>
#include <adwidgets/lifecycle.hpp>
#include <QWidget>
#include <memory>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace adwidgets {

	class ADWIDGETSSHARED_EXPORT PeakdWidget : public QWidget
                                             , public adplugin::LifeCycle {
		Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

	public:
		explicit PeakdWidget(QWidget *parent = 0);
		~PeakdWidget();

        void hideApplyButton( bool );

        // adplugin::LifeCycle
		virtual void OnCreate( const adportable::Configuration& ) override;
		virtual void OnInitialUpdate() override;
		virtual void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        //<--
	public slots:
        // void getContents( adcontrols::ProcessMethod& ) const;
        void handleOnNotifyRelativeAbundance( const QByteArray& );

    signals:
		// void apply( adcontrols::ProcessMethod& );
        void valueChanged();
        void triggerProcess( const QString& );

    private slots:

    private:
        // Ui::PeakMethodForm *ui;
        class impl;
        std::unique_ptr< impl > impl_;

        // void setContents( const adcontrols::PeakMethod& );
        // void getContents( adcontrols::PeakMethod& ) const;
	};

}
