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

#include "mscalibrateform.hpp"
#include "ui_mscalibrateform.h"
#include "spin_t.hpp"
#include "msreferencedialog.hpp"
#include <adcontrols/mscalibratemethod.hpp>
#include <adportable/debug.hpp>
#include <boost/variant.hpp>


namespace adwidgets {
    namespace detail {
        namespace mscalibrateform {

            enum idItem {
                ePolynomialDegreeLabel
                , eMassToleranceLabel
                , eMinimumRALabel
                , eLowMassLabel
                , eHighMassLabel
                , ePolynomialDegree
                , eMassTolerance
                , eMinimumRA
                , eLowMass
                , eHighMass
                , numItems
            };

            typedef boost::variant< QLabel *, QSpinBox *, QDoubleSpinBox * > control_variant;

            struct ui_locator {
                Ui_MSCalibrateForm * ui_;
                control_variant ref_;
                ui_locator( Ui_MSCalibrateForm * ui ) : ui_( ui ) {
                }

                control_variant& operator()( idItem id ) {
                    switch( id ) {
                    case ePolynomialDegreeLabel: ref_ = ui_->label; return ref_;
                    case eMassToleranceLabel: ref_ = ui_->label_2; return ref_;
                    case eMinimumRALabel: ref_ = ui_->label_3; return ref_;
                    case eLowMassLabel: ref_ = ui_->label_4; return ref_;
                    case eHighMassLabel: ref_ = ui_->label_5; return ref_;
                    case ePolynomialDegree: ref_ = ui_->spinBox; return ref_;
                    case eMassTolerance: ref_ = ui_->doubleSpinBox; return ref_;
                    case eMinimumRA: ref_ = ui_->doubleSpinBox_2; return ref_;
                    case eLowMass: ref_ = ui_->doubleSpinBox_3; return ref_;
                    case eHighMass: ref_ = ui_->doubleSpinBox_4; return ref_;
                    case numItems:
                        break;
                    }
                    class error : public boost::exception, public std::exception {};
                    BOOST_THROW_EXCEPTION( error() );
                }
            };
        }
    }
}

using namespace adwidgets;
using namespace adwidgets::detail::mscalibrateform;

MSCalibrateForm::MSCalibrateForm(QWidget *parent) :  QWidget(parent)
                                                  , ui(new Ui::MSCalibrateForm)
                                                  , dlg_( 0 )
{
    ui->setupUi(this);

    connect( ui->pushButton, &QPushButton::clicked, this, &MSCalibrateForm::handleReferenceDlg );
    connect( ui->buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess(); } );

    // font_property()(ui->groupBox);

    ui_locator accessor( ui );
    using namespace spin_initializer;

    //spin_t< QSpinBox, int >::init( boost::get<QSpinBox *>(accessor(ePolynomialDegree)), 1, 25, 1);
    //spin_t< QDoubleSpinBox, double >::init( boost::get<QDoubleSpinBox *>(accessor(eMassTolerance)), 2.0, 50.0, 1.0);
    //spin_t< QDoubleSpinBox, double >::init( boost::get<QDoubleSpinBox *>(accessor(eMinimumRA)), 0.0, 100.0, 1.0); // %
    //spin_t< QDoubleSpinBox, double >::init( boost::get<QDoubleSpinBox *>(accessor(eLowMass)), 1.0, 10000, 1);
    //spin_t< QDoubleSpinBox, double >::init( boost::get<QDoubleSpinBox *>(accessor(eHighMass)), 1000.0, 10000, 1);
    spin_init( boost::get<QSpinBox *>(accessor(ePolynomialDegree))
               , std::make_tuple( Minimum<>{1}, Maximum<>{25}, Value<>{1},Alignment{Qt::AlignRight} ));
    spin_init( boost::get<QDoubleSpinBox *>(accessor(eMassTolerance))
               , std::make_tuple( Minimum<>{2.0}, Maximum<>{50.0}, Value<>{1.0}, Alignment{Qt::AlignRight}) );
    spin_init( boost::get<QDoubleSpinBox *>(accessor(eMinimumRA))
               , std::make_tuple( Minimum<>{0.0}, Maximum<>{100.0}, Value<>{1.0}, Alignment{Qt::AlignRight} )); // %
    spin_init( boost::get<QDoubleSpinBox *>(accessor(eLowMass))
               , std::make_tuple( Minimum<>{1.0}, Maximum<>{10000.}, Value<>{1.}, Alignment{Qt::AlignRight} ));
    spin_init( boost::get<QDoubleSpinBox *>(accessor(eHighMass))
               , std::make_tuple( Minimum<>{1000.0}, Maximum<>{10000.}, Value<>{1.}, Alignment{Qt::AlignRight} ));

    if ( auto spin = boost::get< QDoubleSpinBox * >( accessor( eHighMass ) ) ) {
        connect( spin, qOverload<double>(&QDoubleSpinBox::valueChanged)
                 , [&]( double value ){ if ( dlg_ ) dlg_->setHMass( value ); } );
    }
}

MSCalibrateForm::~MSCalibrateForm()
{
    //ADDEBUG() << "MSCalibrateForm::DTOR";
    if ( dlg_ )
        dlg_->close();
    delete dlg_;
    delete ui;
}

void
MSCalibrateForm::finalClose()
{
    //ADDEBUG() << "MSCalibrateForm::finalClose";
    if ( dlg_ )
        dlg_->close();
}

void
MSCalibrateForm::getContents( adcontrols::MSCalibrateMethod& m )
{
    ui_locator accessor( ui );

    m.polynomialDegree( boost::get< QSpinBox *>( accessor( ePolynomialDegree ) )->value() );
    m.massToleranceDa( boost::get< QDoubleSpinBox *>( accessor( eMassTolerance ) )->value() );
    m.minimumRAPercent( boost::get< QDoubleSpinBox *>( accessor( eMinimumRA ) )->value() );
    m.lowMass( boost::get< QDoubleSpinBox *>( accessor( eLowMass ) )->value() );
    m.highMass( boost::get< QDoubleSpinBox *>( accessor( eHighMass ) )->value() );
}

void
MSCalibrateForm::setContents( const adcontrols::MSCalibrateMethod& m )
{
    ui_locator accessor( ui );

    boost::get< QSpinBox *>( accessor( ePolynomialDegree ) )->setValue( m.polynomialDegree() );
    boost::get< QDoubleSpinBox *>( accessor( eMassTolerance ) )->setValue(m.massToleranceDa() );
    boost::get< QDoubleSpinBox *>( accessor( eMinimumRA ) )->setValue( m.minimumRAPercent() );
    boost::get< QDoubleSpinBox *>( accessor( eLowMass ) )->setValue( m.lowMass() );
    boost::get< QDoubleSpinBox *>( accessor( eHighMass ) )->setValue( m.highMass() );
}

void
MSCalibrateForm::handleReferenceDlg()
{
    if ( dlg_ == 0 ) {
        dlg_ = new MSReferenceDialog(this);
        dlg_->register_handler( [&] ( const adcontrols::MSReference& ref ){ emit addReference( ref ); } );
        // dlg_->setWindowFlags( Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );
        dlg_->setModal( false );
        dlg_->show();
        dlg_->raise();
        dlg_->activateWindow();

        ui_locator accessor( ui );
        if ( auto spin = boost::get< QDoubleSpinBox * >( accessor( eHighMass ) ) ) {
            dlg_->setHMass( spin->value() );
        }
    }
    dlg_->show();
    dlg_->raise();
}
