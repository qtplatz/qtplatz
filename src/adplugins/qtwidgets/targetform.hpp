/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#ifndef TARGETFORM_HPP
#define TARGETFORM_HPP

#include <adplugin/lifecycle.hpp>
#include <QWidget>
#include <memory>

class QStandardItemModel;
class QTreeView;

namespace Ui {
    class TargetForm;
}

namespace adcontrols { class TargetingMethod; class ProcessMethod; }

namespace qtwidgets {

    class AdductsDelegate;
    class FormulaeDelegate;

    class TargetForm : public QWidget
		             , adplugin::LifeCycle {
        Q_OBJECT
    public:
        explicit TargetForm(QWidget *parent = 0);
        ~TargetForm();

		 // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
    public slots:
        void getLifeCycle( adplugin::LifeCycle *& p );
        void getContents( adcontrols::ProcessMethod& );
		virtual void update();
	
	private slots:
		void showContextMenu1( const QPoint& );
		void showContextMenu2( const QPoint& );
		void addAdductOrLoss();
		void delAdductOrLoss();
		void addFormula();
		void delFormula();
	signals:
         void apply( adcontrols::ProcessMethod& );

    private:
        Ui::TargetForm *ui;
        std::unique_ptr< QStandardItemModel > adductsModel_;
        std::unique_ptr< QStandardItemModel > formulaeModel_;
        std::unique_ptr< AdductsDelegate > adductsDelegate_;
        std::unique_ptr< FormulaeDelegate > formulaeDelegate_;
        std::unique_ptr< adcontrols::TargetingMethod > method_;

        void get_method( adcontrols::TargetingMethod& ) const;
        void set_method( const adcontrols::TargetingMethod& );

		static void update_adducts( QTreeView&
                                    , QStandardItemModel&
                                    , const QModelIndex&
                                    , const adcontrols::TargetingMethod&, bool positiveMode );

		static void update_formulae( QTreeView&, QStandardItemModel&, const adcontrols::TargetingMethod& );

		static void get_adducts( QTreeView&
                                 , QStandardItemModel&
                                 , const QModelIndex&
                                 , adcontrols::TargetingMethod&, bool positiveMode );
		static void get_formulae( QTreeView&, QStandardItemModel&, adcontrols::TargetingMethod& );

        static void init_adducts( QTreeView&, QStandardItemModel& );
        static void init_formulae( QTreeView&, QStandardItemModel& );
        static void enable_checkbox( QStandardItemModel&, QModelIndex&, bool isChecked );
    };

}

#endif // TARGETFORM_HPP
