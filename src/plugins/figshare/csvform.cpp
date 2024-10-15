/**************************************************************************
** Copyright (C) 2014-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "csvform.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qstackedwidget.h>
#include <adportable/csv_reader.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/create_widget.hpp>

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QRadioButton>
#include <QSplitter>
#include <QSvgRenderer>
#include <QWidget>

#include <boost/algorithm/string/trim.hpp>
#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>

namespace figshare {

    class CSVForm::impl {
    public:
        impl() {
        }
    };

    class CSVMSForm : public QWidget {
        Q_OBJECT
    public:
        explicit CSVMSForm(QWidget *parent = 0) : QWidget( parent ) {
            using adwidgets::add_widget;
            using adwidgets::add_layout;
            using adwidgets::create_widget;

            if ( auto topLayout = new QHBoxLayout( this ) ) {
                topLayout->setContentsMargins( {} );
                topLayout->setSpacing( 0 );
                if ( auto cbx = add_widget( topLayout, create_widget< QCheckBox >( "cbx", "Peak list" ) ) ) {
                    cbx->setChecked( true );
                }
                if ( auto label = add_widget( topLayout, create_widget< QLabel >( "label_1", "<i>m/z</i>" ) ) ) {
                    label->setTextFormat(Qt::RichText);
                    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                }
                if ( auto combo = add_widget( topLayout, create_widget< QComboBox >( "combo_x" ) ) ) {
                    combo->addItems( QStringList{ "none", "1", "2", "3", "4" } );
                    combo->setCurrentIndex( 1 );
                }
                if ( auto label = add_widget( topLayout, create_widget< QLabel >( "label_2", "intensity" ) ) ) {
                    label->setTextFormat(Qt::RichText);
                    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                }
                if ( auto combo = add_widget( topLayout, create_widget< QComboBox >( "combo_y" ) ) ) {
                    combo->addItems( QStringList{ "none", "1", "2", "3", "4" } );
                    combo->setCurrentIndex( 2 );
                }
            }
        }
        ~CSVMSForm() {}
    };

    class CSVChroForm : public QWidget {
        Q_OBJECT
    public:
        explicit CSVChroForm(QWidget *parent = 0) : QWidget( parent ) {
            if ( auto topLayout = new QVBoxLayout( this ) ) {
                using adwidgets::add_widget;
                using adwidgets::add_layout;
                using adwidgets::create_widget;
                topLayout->setContentsMargins( {} );
                topLayout->setSpacing( 0 );

                if ( auto edit = add_widget( topLayout, create_widget< QLabel >( "xxxxx", "yyyyyyy" ) ) ) {
                    edit->setText( "ABCDEFG");
                }

                if ( auto gbx = add_widget( topLayout, create_widget< QGroupBox >( "grpBox_1", tr("Chromatogram") ) ) ) {
                    // std::tuple< size_t, size_t > xy{0,0};
                    // auto gLayout = add_layout( gbx, create_widget< QGridLayout >( "ms_gridLayout" ) );
                    // gLayout->setSpacing( 2 );
                    // gLayout->setContentsMargins(4, 0, 4, 0);
                    // add_widget( gLayout, create_widget< QRadioButton >( "radioBtn_1", "m/z peak list" ), std::get<0>(xy), std::get<1>(xy)++ );
                    // add_widget( gLayout, create_widget< QRadioButton >( "radioBtn_2", "profile" ), std::get<0>(xy), std::get<1>(xy)++ );
                }
            }
        }
        ~CSVChroForm() {}
    };

    class CSVXYForm : public QWidget {
        Q_OBJECT
    public:
        explicit CSVXYForm(QWidget *parent = 0) : QWidget( parent ) {
            if ( auto topLayout = new QVBoxLayout( this ) ) {
                using adwidgets::add_widget;
                using adwidgets::add_layout;
                using adwidgets::create_widget;
                topLayout->setContentsMargins( {} );
                topLayout->setSpacing( 0 );

                if ( auto edit = add_widget( topLayout, create_widget< QLabel >( "xxxxx", "yyyyyyy" ) ) ) {
                    edit->setText( "ABCDEFG");
                }

                if ( auto gbx = add_widget( topLayout, create_widget< QGroupBox >( "xy_grpBox_1", tr("X,Y") ) ) ) {
                    // std::tuple< size_t, size_t > xy{0,0};
                    // auto gLayout = add_layout( gbx, create_widget< QGridLayout >( "ms_gridLayout" ) );
                    // gLayout->setSpacing( 2 );
                    // gLayout->setContentsMargins(4, 0, 4, 0);
                    // add_widget( gLayout, create_widget< QRadioButton >( "radioBtn_1", "m/z peak list" ), std::get<0>(xy), std::get<1>(xy)++ );
                    // add_widget( gLayout, create_widget< QRadioButton >( "radioBtn_2", "profile" ), std::get<0>(xy), std::get<1>(xy)++ );
                }
            }
        }
        ~CSVXYForm() {}
    };

}

using namespace figshare;

CSVForm::~CSVForm()
{
    delete impl_;
}

CSVForm::CSVForm( QWidget * parent ) : QWidget( parent )
                                     , impl_( new impl{} )
{
    using adwidgets::add_widget;
    using adwidgets::create_widget;

    if ( auto topLayout = new QVBoxLayout( this ) ) {
        topLayout->setContentsMargins( {} );
        topLayout->setSpacing( 0 );

        if ( auto tab = add_widget( topLayout, create_widget< QTabWidget >( "tabWidget" ) ) ) {
            if ( auto wnd = create_widget< CSVMSForm >( "msform" ) ) {
                tab->addTab( wnd, "Mass spectrum" );
            }
            if ( auto wnd = create_widget< CSVChroForm >( "chroform" ) ) {
                tab->addTab( wnd, "Chromatogram" );
            }
            if ( auto wnd = create_widget< CSVXYForm >( "xyform" ) ) {
                tab->addTab( wnd, "X,Y" );
            }
        }
    }
}

#include "csvform.moc"
