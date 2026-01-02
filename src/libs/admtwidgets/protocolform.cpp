/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "protocolform.hpp"
#include <admtcontrols/scanlaw.hpp>
#include <admtcontrols/orbitprotocol.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextEdit>
#include <adwidgets/tableview.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstring>
#include <ratio>

namespace metric = adcontrols::metric;

namespace admtwidgets {

    constexpr int time_resolution = 10; // DG has 10ns precision

    inline double delay_time_round_off( double s, metric::prefix prefix =  metric::micro ) {
        int ns = int( metric::scale_to_nano( s ) + 0.5 );
        return metric::scale_to<double>( prefix, double( ns - ( ns % time_resolution ) ), metric::nano ); //
    }

    inline double delay_time_round_up( double s, metric::prefix prefix = metric::micro ) {
        int ns = int( metric::scale_to_nano( s ) + 0.5 );
        return metric::scale_to<double>( prefix, double( ( ns + time_resolution ) - ( ns % time_resolution ) ), metric::nano );
    }

    template< typename T > void set_value( QObject * _this, const QString& objectname, double value ) {
        if ( auto obj = _this->findChild< T >( objectname ) ) {
            if ( obj->isEnabled() ) {
                QSignalBlocker block( obj );
                obj->setValue( value );
            }
        }
    }

    template< typename T > double value( QObject * _this, const QString& objectname ) {
        if ( auto obj = _this->findChild< T >( objectname ) )
            return obj->value();
        return 0;
    }

    template< typename T > void set_text( QObject * _this, const QString& objectname, QString text ) {
        if ( auto obj = _this->findChild< T >( objectname ) ) {
            QSignalBlocker block( obj );
            obj->setText( text );
        }
    }

    // template<class _Ty,  class... _Types>
    // inline QWidget * create_widget(const char * ident, _Types&&... _Args)
    // {
    //     auto w = new _Ty( std::forward<_Types>(_Args)...);
    //     if ( ident && *ident )
    //         w->setObjectName( ident );
    //     return w;
    // }

    class protocolForm::impl {
    public:
        impl() : rowId_( 0 ) {
        }

        ~impl() {}
        int rowId_;

        void clear_dirty( protocolForm * form ) {
            for ( auto& w : form->findChildren<QSpinBox *>() )
                w->setStyleSheet( "QSpinBox { color: black; }" );

            for ( auto& w : form->findChildren<QDoubleSpinBox *>() )
                w->setStyleSheet( "QDoubleSpinBox { color: black; }" );
        }
    };

    static constexpr const char * const itemlist [] = { "push", "inject", "exit", "exit_1", "gate", "gate_1", "adc", "p_lift", "gate7" };
}

using namespace admtwidgets;

protocolForm::protocolForm( int protocol, QWidget *parent ) : QWidget(parent)
                                                            , impl_( new impl() )
                                                            , protocolId_( protocol )
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );

    if ( auto vLayout = new QVBoxLayout( this ) ) {

        auto gBox = qtwrapper::make_widget< QGroupBox >( "topGroupBox", QString( "P.%1" ).arg( protocol ) );
        gBox->setCheckable( true );
        vLayout->addWidget( gBox );

        if ( auto gridLayout = new QGridLayout() ) {
            gBox->setLayout( gridLayout );
            //vLayout->addLayout( gridLayout );
            gridLayout->setSpacing( 1 );

            int row = 0;
            int col = 0;
            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", "delay(&mu;s)" ), row, 1, Qt::AlignHCenter );
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", "width(&mu;s)" ), row, 2, Qt::AlignHCenter );

            // workaround
            for ( auto obj: { "edit_formula", "is_mass_reference", "laps", "lower_mass", "upper_mass" } )
                if ( auto w = findChild< QWidget * >( obj ) )
                    w->setEnabled( false );
            //

            //-------------- ScanLaw (va) -----
#if 0
            col = 3;
            double vacc   = impl_->scanlaw_->kAcceleratorVoltage();
            double tdelay = impl_->scanlaw_->tDelay();
            double L = impl_->scanlaw_->fLength( 0 );
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "<em>Vacc</em>" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawVacc", QString( "%1 (V)").arg( vacc ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QPushButton >( "commit", tr("Commit") ), row, col++, Qt::AlignRight );
#endif

            // Protocol#
            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "PUSH" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "push_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "push_width" ), row, col++ );
            //---------------------------------

            //-------------- ScanLaw (t0) -----
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "<em>T<sub>0</sub></em>" ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawTdelay", QString( "%1 (&mu;s)" ).arg( metric::scale_to_micro( tdelay ) ) ), row, col++, Qt::AlignRight );
            //---------------------------------

            ++row; col = 0;

            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "INJECT" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "inject_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "inject_width" ), row, col++ );
            // gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "inject_mass", tr( "" ) ), row, col++, Qt::AlignLeft );
            //-------------- ScanLaw (L) -----
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "<em>L</em>" ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawL", QString( "%1 (m)" ).arg( L ) ), row, col++, Qt::AlignRight );
            //---------------------------------

            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "EXIT" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "exit_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "exit_width" ), row, col++ );

            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "exit_1", tr( "EXIT2" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "exit_1_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "exit_1_width" ), row, col++ );

            // ++row; col = 0;
            //---------------------------------

            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "gate", tr( "GATE1" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate_width" ), row, col++ );
            //gridLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "gate_lock", tr("Lock") ), row, col++, Qt::AlignLeft );

            //++col;
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawGateLap", QString( "--" ) ), row, col++, Qt::AlignRight );

            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "gate_1", tr( "GATE2" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate_1_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate_1_width" ), row, col++ );

            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "Gate off <em>m/z</em>" ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawGateLimit_0", QString( "--" ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawGateLimit_1", QString( "--" ) ), row, col++, Qt::AlignRight );

            //-------------- ScanLaw (T) -----
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawFormula", tr( "<em>n/a</em>" ) ), row, col++, Qt::AlignRight );
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawGate", QString( "%1 (&mu;s)" ).arg( L ) ), row, col++, Qt::AlignRight );
            //if ( auto label = findChild< QLabel * >( "scanlawGate" ) ) {
            //    label->setToolTip( tr("Lap time.") );
            // }
            //---
            //gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "scanlawTime", QString( "%1 (&mu;s)" ).arg( L ) ), row, col++, Qt::AlignRight );
            //if ( auto label = findChild< QLabel * >( "scanlawTime" ) ) {
            //    label->setToolTip( tr("Expected time-of-flight.") );
            //}
            //-------------------------------------------

            ++row; col = 0;
#if 0
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "ADC Delay" ) ), row, col++, Qt::AlignRight ); // DG external delay
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "ext_adc_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "ext_adc_width" ), row, col++ );
#endif
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "ADC" ) ), row, col++, Qt::AlignRight ); // digitizer delay
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "adc_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "adc_width" ), row, col++ );
            //-------------------------------------------
#if 0
            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "Trig. Replicates" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QSpinBox >( "replicates" ), row, col++ );

            ++col; // skip
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "Trig. Rate(us)" ) ), row, col++, Qt::AlignRight );
            gridLayout->addWidget( qtwrapper::make_widget< QSpinBox >( "trig_rate" ), row, col++ );
#endif
            //-------------------------------------------
            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "P.Lift(&mu;s)" ) ), row, col++, Qt::AlignRight ); // DG external delay
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "p_lift_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "p_lift_width" ), row, col++ );

            ++row; col = 0;
            gridLayout->addWidget( qtwrapper::make_widget< QLabel >( "", tr( "GATE7(&mu;s)" ) ), row, col++, Qt::AlignRight ); // DG external delay
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate7_delay" ), row, col++ );
            gridLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "gate7_width" ), row, col++ );

            // add spacer
            ++row; col = 1;
            vLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

            for ( auto& label : findChildren<QLabel *>() ) {
                label->setTextFormat( Qt::RichText );
            }

            // Keyboard tracking disabled
            for ( auto spin: findChildren< QSpinBox *>() )
                spin->setKeyboardTracking( false );

            for ( auto spin: findChildren< QDoubleSpinBox *>() )
                spin->setKeyboardTracking( false );

            if ( auto spin = findChild< QSpinBox * >( "nbrAverage" ) ) {
                spin->setMinimum( -1 );
                spin->setMaximum( 65535 );
            }

            for ( auto& name: itemlist ) {
                if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_delay" ).arg( name ) ) ) {
                    spin->setMinimum( ( std::strcmp( name, "inject" ) == 0 ) ? -100 : 0 );
                    spin->setMaximum( 10000.0 );
                    spin->setDecimals( 3 );
                    spin->setSingleStep( 0.010 );
                }
                if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_width" ).arg( name ) ) ) {
                    spin->setMinimum( 0.000 );  // zero := disable
                    spin->setMaximum( 1000.0 );
                    spin->setDecimals( 3 );
                    spin->setSingleStep( 0.010 );
                }
            }
        }

        for ( auto& w : findChildren< QPushButton * >() ) {
            connect( w, &QPushButton::clicked, [this,w](){ handleCommit(w); } );
        }

        for ( auto& w : findChildren< QCheckBox *>() ) {
            //w->setTextFormat( Qt::RichText );
            connect( w, &QCheckBox::checkStateChanged
                     , [this,w](Qt::CheckState){
                           handleValueChanged(w);
                       });
        }

        for ( auto& w : findChildren<QSpinBox *>() ) {
            connect( w, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
                     , [this,w](int){
                           handleValueChanged(w);
                           w->setStyleSheet( "QSpinBox { color: #ff6347; }" );
                       });
        }

        for ( auto& w : findChildren<QDoubleSpinBox *>() ) {
            connect( w, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
                     , [this,w](double){
                           handleValueChanged(w);
                           w->setStyleSheet( "QDoubleSpinBox { color: #ff6347; }" );
                       });
        }

        for ( auto& w : findChildren<QComboBox *>() ) {
            connect( w, static_cast< void(QComboBox::*)(int) >(&QComboBox::currentIndexChanged)
                     , [this,w](int){
                           handleValueChanged(w);
                       });
        }

        if ( auto cbx = findChild< QCheckBox * >( "gate" ) ) {
            std::pair< QWidget *, QWidget * > w{ findChild< QDoubleSpinBox * >("gate_delay"), findChild< QDoubleSpinBox * >("gate_width") };
            connect( cbx, &QCheckBox::checkStateChanged
                     , [=]( int state ){
                           bool enable = ( state == Qt::Checked );// && impl_->editorBehavior_->gateEditable( 0 );
                           w.first->setEnabled( enable );
                           w.second->setEnabled( enable );
                       });
        }

        if ( auto cbx = findChild< QCheckBox * >( "gate_1" ) ) {
            std::pair< QWidget *, QWidget * > w{ findChild< QDoubleSpinBox * >("gate_1_delay"), findChild< QDoubleSpinBox * >("gate_1_width") };
            connect( cbx, &QCheckBox::checkStateChanged
                     , [=]( int state ){
                           bool enable = ( state == Qt::Checked ); // && impl_->editorBehavior_->gateEditable( 1 );
                           w.first->setEnabled( enable );
                           w.second->setEnabled( enable );
                       });
        }
    }
}

protocolForm::~protocolForm()
{
    delete impl_;
}

void
protocolForm::setDirty( bool dirty )
{
    if ( !dirty )
        impl_->clear_dirty( this );
}

void
protocolForm::handleCommit( QWidget * w )
{
}

void
protocolForm::handleValueChanged( QWidget * w )
{
    emit onDataChanged( impl_->rowId_ );
}

void
protocolForm::showContextMenu( const QPoint& pos )
{
    QMenu menu;

    std::vector< QAction * > actions;

    actions.emplace_back( menu.addAction( tr( "Lock Gate && INJECT Timings" ) ) );
    actions.back()->setCheckable( true );
    //actions.back()->setChecked( !impl_->editorBehavior_->gateEditable( 0 ) );

    actions.emplace_back( menu.addAction( tr( "Save Gate && INJECT Timing as default" ) ) );
    actions.emplace_back( menu.addAction( tr( "Restore default Gate && INJECT Timings" ) ) );

    if ( auto action = menu.exec( mapToGlobal( pos ) ) ) {

        if ( action == actions[ 0 ] ) {
            bool checked = action->isChecked();
            if ( auto cbx = findChild< QCheckBox * >( "gate_lock" ) ) {
                cbx->setChecked( checked );
            }
        }
    }
}

QJsonObject
protocolForm::json() const
{
    QJsonArray pulses;
    for ( auto& name: itemlist ) {
        std::pair< double, double > delay_pulse({0,0});
        bool enable( false );
        if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_delay" ).arg( name ) ) ) {
            delay_pulse.first = spin->value() / std::micro::den;
        }
        if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_width" ).arg( name ) ) ) {
            delay_pulse.second = spin->value() / std::micro::den;
        }
        if ( auto cbx = findChild< QCheckBox * >( name ) ) {
            enable = cbx->isChecked();
        }
        pulses.push_back( QJsonObject{ { "delay", delay_pulse.first }, { "width", delay_pulse.second }, { "enable", enable }, { "name", name } } );
    }

    QJsonObject jobj;
    jobj[ "pulses" ] = pulses;
    jobj[ "index" ] = int( protocolId_ );

    if ( auto gBox = findChild< QGroupBox * >( "topGroupBox" ) ) {
        jobj[ "enable" ] = gBox->isChecked();
    }

    return jobj;
}

void
protocolForm::setJson( const QJsonObject& jobj )
{
    // ADDEBUG() << QJsonDocument( jobj ).toJson().toStdString();
    if ( jobj.contains( "pulses" ) ) {

        auto pulses = jobj[ "pulses" ].toArray();

        for ( const auto& p: pulses ) {
            auto pulse = p.toObject();
            if ( pulse.contains( "name" ) ) {
                auto name = pulse[ "name" ].toString();
                if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_delay" ).arg( name ) ) )
                    spin->setValue( pulse[ "delay" ].toDouble() * std::micro::den );
                if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_width" ).arg( name ) ) )
                    spin->setValue( pulse[ "width" ].toDouble() * std::micro::den );
                if ( auto cbx = findChild< QCheckBox * >( name ) )
                    cbx->setChecked( pulse[ "enable" ].toBool() );
            } else {
                ADDEBUG() << "name does not exists -- assume fetched from dgctl";
            }
        }
    }

    if ( jobj.contains( "enable" ) ) {
        if ( auto gBox = findChild< QGroupBox * >( "topGroupBox" ) )
            gBox->setChecked( jobj[ "enable" ].toBool() );
    }
}

void
protocolForm::setJson( const boost::json::value& pt )
{
    if ( pt.is_object() ) {
        const auto& obj = pt.as_object();

        int replicates = boost::json::value_to< int >( obj.at( "replicates" ) );
        if ( auto gBox = findChild< QGroupBox * >( "topGroupBox" ) )
            gBox->setChecked( replicates != 0 );
    }

    size_t idx(0);
    auto pulses = adportable::json_helper::find( pt, "pulses" );
    if ( pulses.is_array() ) {
        for ( const auto& pulse: pulses.as_array() ) {
            double delay(0), width(0);
            adportable::json::extract( pulse.as_object(), delay, "delay" );
            adportable::json::extract( pulse.as_object(), width, "width" );
            auto name = itemlist[ idx ];
            if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_delay" ).arg( name ) ) )
                spin->setValue( delay );
            if ( auto spin = findChild< QDoubleSpinBox * >( QString( "%1_width" ).arg( name ) ) )
                spin->setValue( width );
            ++idx;
        }
    }
}
