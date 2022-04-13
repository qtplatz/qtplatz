/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "hvtunewidget.hpp"
#include "json_value.hpp"
#include "hvfsm.hpp"
#include "constants.hpp"
#include <qtwrapper/make_widget.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <adurl/ajax.hpp>
#include <adurl/sse.hpp>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QLCDNumber>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QtGlobal>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <bitset>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

namespace infitofwidgets {
    typedef QDoubleSpinBox act_widget_t;
#define USING_QDOUBLESPINBOX 1
#define USING_QLCDNUMBER 0

    template< typename T = bool >
    struct scoped_flag {
        T& flag;
        scoped_flag( T& f ) : flag( f ) {}
        ~scoped_flag() {
            flag = !flag;
        }
    };

    struct sector_voltage {
        std::pair< double, double > outer; // (+) setpt,actual
        std::pair< double, double > inner; // (-) setpt,actual
    };

    enum idSector { idVinj, idVext, idVtn, idVgate, idError };
    enum idPolarity { idOuter, idInner, idNone };

    struct sector_id {
        // conversion table for MSI voltage array index
        // idSector = __idList[ msi_id ]
        constexpr static const std::tuple< idSector
                                           , uint32_t
                                           , idPolarity // outer|inner|none
                                           , const std::pair< uint32_t, uint32_t > > __idList [] = {
            { idError,     0, idNone,  { 0, 0 }} // 0 error
            , { idError,   0, idNone,  { 0, 0 }} // 1 error
            , { idVext,    3, idInner, { 3, 2 }} // 2 Vext.in
            , { idVext,    2, idOuter, { 3, 2 }} // 3 Vext.out
            , { idVgate,   5, idInner, { 5, 4 }} // 4 Vgate.in
            , { idVgate,   4, idOuter, { 5, 4 }} // 5 Vgate.out
            , { idVtn,     7, idInner, { 7, 6 }} // 6 Vtn.in
            , { idVtn,     6, idOuter, { 7, 6 }} // 7 Vtn.out
            , { idError,   0, idNone,  { 0, 0 }} // 8 error
            , { idVinj,   10, idInner, {10, 9 }} // 9 Vinj.in
            , { idVinj,    9, idOuter, {10, 9 }} // 10 VInj.out
        };

        // counter part (outer|inner) MSI voltage vector id
        inline static uint32_t buddy( uint32_t id ) {
            if ( id < sizeof( __idList ) / sizeof( __idList[ 0 ] ) )
                return std::get< 1 >( __idList[id] );
            return 0; // error
        }

        // idVinj, ...
        inline static idSector sectorId( uint32_t id ) {
            if ( id < sizeof( __idList ) / sizeof( __idList[ 0 ] ) )
                return std::get< 0 > ( __idList[id] );
            return idError; // error
        }

        inline static idPolarity polarity( uint32_t id ) {
            if ( id < sizeof( __idList ) / sizeof( __idList[ 0 ] ) )
                return std::get< 2 > ( __idList[id] );
            return idNone;
        }

        // MSI voltage vector id pair (outer,inner)
        inline static const std::pair< uint32_t, uint32_t > sector_idpair( uint32_t id ) {
            if ( id < sizeof( __idList ) / sizeof( __idList[ 0 ] ) )
                return std::get< 3 > ( __idList[id] );
            return { 0, 0 };
        }
    };

    constexpr const std::tuple< idSector
                                , uint32_t
                                , idPolarity // outer|inner|none
                                , const std::pair< uint32_t, uint32_t > > sector_id::__idList [];

    class hvTuneWidget::impl : public QObject
                             , public hvfsm::handler {

        Q_OBJECT

        hvTuneWidget * parent_;
    public:
        std::string server_;
        std::string port_;
        //std::unique_ptr< adurl::old::sse > sse_;
        std::unique_ptr< adurl::sse_handler > sse_;
        boost::asio::io_context io_context_;
        std::vector< std::thread > threads_;
        std::bitset< 16 > aux1_;
        std::bitset< 12 > alarm1_;
        std::bitset< 16 > aux2_;
        std::bitset< 12 > alarm2_;
        boost::msm::back::state_machine< hvfsm::controller_ > fsm_;
        bool isRelative_;
        bool isLoading_;

        std::array< sector_voltage, 4 > sector_voltages_;  // absolute voltages
        std::mutex mutex_;

    public:
        impl( hvTuneWidget * parent
              , const std::string& server
              , const std::string& port ) : parent_( parent )
                                          , server_( server )
                                          , port_( port )
                                          , fsm_( this )
                                          , isRelative_( false )
                                          , isLoading_( false ) {

            // sse_ = std::make_unique< adurl::old::sse >( server_.c_str(), "/hv/api$events", port_.c_str() );

            // sse_->exec( [this]( const char * event, const char * data ) {
            //         if ( ( std::strcmp( event, "event: hv.tick" ) == 0 ) && ( std::strncmp( data, "data:", 5 ) == 0 ) ) {
            //             data += 5;
            //             while ( *data == ' ' || *data == '\t' )
            //                 ++data;
            //             QByteArray a( data, std::strlen( data ) );
            //             emit onReply( event, data );
            //         }
            // });

            if ( ( sse_ = std::make_unique< adurl::sse_handler >( io_context_ ) ) ) {
                sse_->connect( "/hv/api$events"
                               , server, port
                               , [&]( adurl::sse_event_data_t&& ev ) {
                                   std::string event, data;
                                   std::tie( event, std::ignore, data ) = std::move( ev );
                                   if ( event == "hv.tick" )
                                       emit onReply( QString::fromStdString( event ), QByteArray( data.c_str() ) );
                                   // ADDEBUG() << "event: " << event << "\tdata: " << data.substr( 0, 120 );
                               }, false );
                threads_.emplace_back( [&](){ io_context_.run(); } );
            }
        }

        ~impl() {
            io_context_.stop();
            for ( auto& t: threads_ )
                t.join();
            // if ( sse_ )
            //     sse_->stop();
        }

        void setSectorVoltage( idSector id, bool isInner, std::pair< double , double >&& pair ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            if ( isInner )
                sector_voltages_[ id ].inner = pair;
            else
                sector_voltages_[ id ].outer = pair;
        }

        void setSectorSetpt( idSector id, idPolarity polarity, double value ) {
            if ( polarity == idOuter )
                sector_voltages_[ id ].outer.first = value;
            else if ( polarity == idInner )
                sector_voltages_[ id ].inner.first = value;
        }

        std::pair< double, double > sectorSetpt( idSector id ) {
            if ( id < sizeof( sector_voltages_ ) / sizeof( sector_voltages_[ 0 ] ) ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                return std::make_pair( sector_voltages_[ id ].outer.first, sector_voltages_[ id ].inner.first );
            }
            return std::make_pair(0,0);
        }

        std::pair< double, double > sectorDisplayVoltage( const std::string& name ) {
            constexpr const char * const __names [] = { "Vinj", "Vext", "Vtn", "Vgate" };
            constexpr const size_t size = sizeof( __names )/sizeof(__names[0] );
            auto it = std::find_if( __names, &__names[size], [&](const char * a){ return name.find(a) != std::string::npos; } );
            auto idx = std::distance( __names, it );
            if ( idx < size ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                bool inner = ( name.find( ".in" ) != std::string::npos );
                if ( isRelative_ ) {
                    double delta = sector_voltages_[ idx ].outer.first - sector_voltages_[ idx ].inner.first;
                    if ( inner ) {
                        return std::make_pair( delta, sector_voltages_[ idx ].inner.second );
                    } else {
                        return std::make_pair( sector_voltages_[ idx ].outer.first - delta / 2, sector_voltages_[ idx ].outer.second );
                    }
                } else {
                    return ( inner ) ? sector_voltages_[ idx ].inner : sector_voltages_[ idx ].outer;  // pair(set,act)
                }
            }
            return std::make_pair( 0.0, 0.0 );
        }

        // handler
        void fsm_action_stop() override {
            ADDEBUG() << __FUNCTION__;
            if ( auto button = parent_->findChild< QPushButton * >( "switch.onoff" ) )
                button->setChecked( false );
        }
        void fsm_action_start()  override  {
            ADDEBUG() << __FUNCTION__;
            if ( auto button = parent_->findChild< QPushButton * >( "switch.onoff" ) )
                button->setChecked( true );
        }
        void fsm_state( bool, hvfsm::idState, int id_state = 0 )  override {
            ADDEBUG() << __FUNCTION__;
        }
        void fsm_no_transition( int state )  override  {
            ADDEBUG() << __FUNCTION__;
        }
        void fsm_exception_caught( const char *, const std::exception& )  override {
            ADDEBUG() << __FUNCTION__;
        }
        bool fsm_diag_result()  override {
            ADDEBUG() << __FUNCTION__;
            return true;
        }

        const std::string& server() const { return server_; }
        const std::string& port() const { return port_; }
    public:

    signals:
        void onReply( const QString&, const QByteArray& );
        void onProtocols( const QByteArray& );
    };

    static const std::tuple< const std::string, QString, double, QString, int > sector_names [] = {
        { "Vinj.out",        "INJECTION(+V)", 2000.0, "INJECTION", 10 }
        , { "Vinj.in",       "(-V)",          2000.0, "(&delta;)",  9 }
        , { "Vext.out",      "EJECTION(+V)",  2000.0, "EJECTION",   3 }
        , { "Vext.in",       "(-V)",          2000.0, "(&delta;)",  2 }
        , { "Vtn.out",       "ORBIT(+V)",     2000.0, "ORBIT",      7 }
        , { "Vtn.in",        "(-V)",          2000.0, "(&delta;)",  6 }
        , { "Vgate.out",     "GATE(+V)",      2000.0, "GATE",       5 }
        , { "Vgate.in",      "(-V)",          2000.0, "(&delta;)",  4 }
    };

    std::string sector_name_from_id( int id ) {
        const auto sector_names_end = sector_names + sizeof( sector_names ) / sizeof( sector_names[ 0 ] );
        auto it = std::find_if( sector_names, sector_names_end, [&]( auto& a ){ return id ==  std::get< 4 >( a ); } );
        if ( it != sector_names_end )
            return std::get< 0 >(*it);
        return "";
    }

    static const std::tuple< const std::string, QString, double, int > item_names [] = {
        { "Vmatsuda",       "Matsuda(V)",    2000.0,  8 }
        , { "Veinzel",      "Einzel(V)",     5000.0, 11 }
        , { "Vacc",         "Acc(V)",        5000.0, 12 }
        , { "Vpush",        "Push(V)",       2000.0, 17 }
        , { "Ifil",         "Filament(mA)",  5000.0, 20 }  // mA
        , { "Vion",         "Ionization(V)",  100.0, 19 }
        , { "Vdet",         "Detector(V)",   5000.0,  1 }
    };

    using namespace qtwrapper;

    static const std::tuple< const std::string, QString, std::function< QWidget*(void) > > item2_factories [] = {
        { "Vtrp", "Vtrp",                     [](){ return make_widget< act_widget_t >("act.Vtrp"); } }
        //, { "Theat50",      "Heater(50W)",    [](){ return make_widget< act_widget_t >("act.Theat50"); } }
        //, { "Theat100",     "Heater(100W)",   [](){ return make_widget< act_widget_t >("act.Theat100"); } }
        , { "Iion",         "Ionization(mA)", [](){ return make_widget< act_widget_t >("act.Iion"); } }
        //, { "Theat20",      "Heater(20W)",    [](){ return make_widget< act_widget_t >("act.Theat20"); } }
        , { "DFP.IS",       "DFP.IS(mA)",     [](){ return make_widget< act_widget_t >("act.DFP.IS"); } }
        , { "DFP.Analyzer", "DFP.Analyzer",   [](){ return make_widget< act_widget_t >("act.DFP.Analyzer"); } }
        //, { "PStemp",       "P.S. Temp(<sup>o</sup>C)", [](){ return make_widget< act_widget_t >("act.PStemp"); } }
        , { "Rev.FPGA",     "Rev.FPGA",       [](){ return make_widget< QLineEdit >("act.Rev.FPGA"); } }
        , { "Rev.CPLD",     "Rev.CPLD",       [](){ return make_widget< QLineEdit >("act.Rev.CPLD"); } }
    };
#if 0
    static const std::tuple< const std::string, QString, std::function< QWidget*(void) > > switch_factories [] = {
        { "switch.isovalve",       "Isolation Valve",   [](){
                auto w = make_widget< QPushButton >("switch.isovalve"); w->setProperty( "id", "switch-isovalve" ); return w; } }
        , { "switch.refinlet",     "Reference Inlet",   [](){
                auto w = make_widget< QPushButton >("switch.refinlet"); w->setProperty( "id", "switch-stdinlet" ); return w; } }
        , { "switch.filsel",       "Filament Sel.",     [](){
                auto w = make_widget< QPushButton >("switch.filsel"); w->setProperty( "id", "switch-filsel" ); return w; }  }
    };
#endif
}

#include "hvtunewidget.moc"

using namespace infitofwidgets;

hvTuneWidget::hvTuneWidget( const QString& server
                            , const QString& port
                            , QWidget * parent ) : QWidget( parent )
                                                 , impl_( std::make_unique< impl >( this, server.toStdString(), port.toStdString() ) )
{
    using namespace qtwrapper;

    resize( 200, 100 );

    if ( auto layout = new QVBoxLayout( this ) ) {
        //layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        if ( auto grid = new QGridLayout() ) {
            int row = 0, col = 0;

            grid->setVerticalSpacing( 0 );

            grid->addWidget( make_widget< QLabel >( "pressure.analyzer", "Analyzer(Pa)" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "pressure.analyzer", "" ), row, col++ );

            grid->addWidget( make_widget< QLabel >( "pressure.ionsource", "Ion Source(Pa)" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "pressure.ionsource" ), row, col++ );

            if ( auto onoff = make_widget< QPushButton >( "switch.onoff", "HV" ) ) {
                onoff->setProperty( "id", "switch-hv" );
                grid->addWidget( onoff, row, col++ );
            }

            grid->setColumnStretch( 0, 1 ); // analyzer label
            grid->setColumnStretch( 1, 1 ); // analyzer value
            grid->setColumnStretch( 2, 1 ); // ionsource label
            grid->setColumnStretch( 3, 1 ); // ionsource value
            grid->setColumnStretch( 4, 2 ); // onoff button

            layout->addLayout( grid );
        }

        if ( auto hor = new QHBoxLayout() ) {

            hor->setContentsMargins(0, 0, 0, 0);
            //hor->setSpacing(0);

            if ( auto gridLayout = new QGridLayout() ) {
                // Left
                gridLayout->setVerticalSpacing( 0 );
                int row = 0;
                int col = 0;

                auto mode = make_widget< QComboBox >( "mode" );
                mode->addItems( { "Absolute", "Relative" } );
                connect( mode, QOverload<int>::of( &QComboBox::activated ), this, &hvTuneWidget::handleModeChanged );

                gridLayout->addWidget( mode, row, col++ );
                if ( auto cbx = make_widget< QCheckBox >( "fine", "0.1V step" ) ) {
                    gridLayout->addWidget( cbx, row, col++, Qt::AlignLeft );
                    connect( cbx, &QCheckBox::toggled, this
                             , [&](bool flag){
                                   for ( auto w: findChildren< QDoubleSpinBox * >() )
                                       w->setSingleStep( flag ? 0.1 : 1.0 );
                               });
                }

                for ( const auto& name: sector_names ) {
                    ++row;
                    col = 0;
                    gridLayout->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                    gridLayout->addWidget( make_widget< QDoubleSpinBox >( ("set." + std::get< 0 >( name ) ).c_str() ), row, col++ );
                    gridLayout->addWidget( make_widget< act_widget_t >( ("act." + std::get< 0 >( name ) ).c_str() ), row, col++ );
                }

                hor->addLayout( gridLayout );
            }

            if ( auto gridLayout = new QGridLayout() ) {
                // Middle
                gridLayout->setVerticalSpacing( 0 );
                int row = 0;
                int col = 0;
                for ( const auto& name: item_names ) {
                    ++row;
                    col = 0;
                    gridLayout->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                    gridLayout->addWidget( make_widget< QDoubleSpinBox >( ("set." + std::get< 0 >( name ) ).c_str() ), row, col++ );
                    gridLayout->addWidget( make_widget< act_widget_t >( ("act." + std::get< 0 >( name ) ).c_str() ), row, col++ );
                }

                hor->addLayout( gridLayout );
            }
            if ( auto gridLayout = new QGridLayout() ) {
                // Right
                gridLayout->setVerticalSpacing( 0 );

                int row = 0;
                int col = 0;
                for ( const auto& name: item2_factories ) {
                    ++row;
                    col = 0;
                    gridLayout->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                    gridLayout->addWidget( std::get< 2 >( name )(), row, col++ );
                }

                hor->addLayout( gridLayout );
            }
            hor->setStretch( 0, 1 );
            hor->setStretch( 1, 1 );
            hor->setStretch( 2, 0 );

            layout->addLayout( hor );
        }
#if 0
        if ( auto grid = new QGridLayout() ) {
            int row = 0;
            int col = 0;
            grid->setVerticalSpacing( 0 );
            for ( const auto& name: switch_factories ) {
                //++row;
                //col = 0;
                grid->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                grid->addWidget( std::get< 2 >( name )(), row, col++ );
            }
            layout->addLayout( grid );
        }
#endif
#if 0
        if ( auto grid = new QGridLayout() ) {
            int row = 0, col = 0;
            grid->setVerticalSpacing( 0 );
            grid->addWidget( make_widget< QLabel >( "tick", "Elapsed time:" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "eventText", "tick" ), row, col++ );

            grid->addWidget( make_widget< QLabel >( "url", "URL" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "url" ), row, col++ );

            row++;
            col = 0;
            grid->addWidget( make_widget< QTextEdit >( "alarms" ), row, col, 1, 4 );

            layout->addLayout( grid );
        }
#endif
    }

    ////////////////////////////////////////////////////////
    // set widget attributes
    for ( auto label: findChildren< QLabel * >() ) {
        label->setAlignment( Qt::AlignRight | Qt::AlignCenter );
        label->setTextFormat( Qt::RichText );
    }

    for ( auto widget: findChildren< QLineEdit * >( QRegExp( "act\\..*|pressure\\..*|tick|url" ) ) ) {
        widget->setReadOnly( true );
        widget->setFocusPolicy( Qt::ClickFocus );
        widget->setAlignment( Qt::AlignHCenter );
    }

    for ( auto widget: findChildren< act_widget_t * >( QRegExp( "act\\..*" ) ) ) {
#if USING_QDOUBLESPINB0X
        widget->setReadOnly( true );
#endif
        widget->setFocusPolicy( Qt::ClickFocus );
    }


    for ( auto widget: findChildren< QTextEdit * >() ) {
        widget->setReadOnly( true );
        widget->setFocusPolicy( Qt::ClickFocus );
    }

    /////////////////////////////////////////////////
    // sectors
    for ( auto& sector: sector_names ) {
        std::string key;
        double maxValue;
        int id;
        std::tie( key, std::ignore, maxValue, std::ignore, id ) = sector;
        if ( auto sbox = findChild< QDoubleSpinBox * >( ( "set." + key ).c_str() ) ) {
            sbox->setMaximum( maxValue );
            if ( key.find( ".in" ) != std::string::npos )
                sbox->setMinimum( -maxValue );
            sbox->setDecimals( 1 );
            sbox->setKeyboardTracking( false );
            sbox->setProperty( "id", id );
            connect( sbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged)
                     , [sbox,this]( double value ){ handleSectorValueChanged( sbox, value ); } );
        }
        if ( auto sbox = findChild< QDoubleSpinBox * >( ( "act." + key ).c_str() ) ) {
            sbox->setMaximum( maxValue );
            sbox->setDecimals( 1 );
            sbox->setAlignment( Qt::AlignRight | Qt::AlignCenter );
        }
    }

    for ( auto& item: item_names ) {
        if ( auto sbox = findChild< QDoubleSpinBox * >( ( "set." + std::get< 0 >( item ) ).c_str() ) ) {
            sbox->setMaximum( std::get< 2 >( item ) );
            sbox->setDecimals( 1 );
            sbox->setKeyboardTracking( false );
            sbox->setProperty( "id", std::get< 3 >( item ) );
            connect( sbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged)
                     , [sbox,this]( double value ){ handleValueChanged( sbox, value ); } );
        }
#if ! USING_QDOUBLESPINB0X
        if ( auto sbox = findChild< act_widget_t * >( ( "act." + std::get< 0 >( item ) ).c_str() ) ) {
            sbox->setMaximum( std::get< 2 >( item ) );
            sbox->setAlignment( Qt::AlignRight | Qt::AlignCenter );
        }
#endif
    }

    for ( auto widget: findChildren< act_widget_t * >( QRegExp( "act\\..*") ) ) {
#if USING_QLCDNUMBER
        widget->setDigitCount( 7 );
        widget->setSegmentStyle( QLCDNumber::Flat );
#endif
        widget->setButtonSymbols( QAbstractSpinBox::NoButtons );
        widget->setStyleSheet( "color: lime; background-color: navy;" );
    }

    for ( auto widget: findChildren< QDoubleSpinBox * >() ) {
        widget->setAlignment( Qt::AlignRight );
    }

    for ( auto& button: findChildren< QPushButton * >( QRegExp( "switch\\..*" ) ) ) {
        button->setCheckable( true );
        button->setChecked( false );
        handleSwitchToggled( button, false );
        connect( button, &QPushButton::toggled, this, [button,this]( bool checked ){ handleSwitchToggled( button, checked ); } );
        connect( button, &QPushButton::clicked, this, [button,this]( bool checked ){ handleSwitchClicked( button, checked ); } );
    }

    setStyleSheet(
        "QPushButton { background-color: green; }"
        "QPushButton:hover { background-color: yellow; }"
        "QPushButton#switch\\.onoff:checked { background-color: red; }"
        "QPushButton:checked { background-color: blue; }"
        );

    setUrl( QString( "http://%1:%2/hv/api$events" ).arg( impl_->server().c_str(), impl_->port().c_str() ) );

    connect( impl_.get(), &impl::onReply, this, &hvTuneWidget::handleReply );
}

hvTuneWidget::~hvTuneWidget()
{
}

void
hvTuneWidget::setUrl( const QString& url )
{
    if ( auto edit = findChild< QLineEdit * >( "url" ) )
        edit->setText( url );
}

void
hvTuneWidget::uiUpdateSectorVoltages()
{
    for ( auto& sector: sector_names ) {
        const std::string& name = std::get< 0 >( sector );
        auto pair = impl_->sectorDisplayVoltage( name );
        if ( auto w = findChild< QDoubleSpinBox * >( QString::fromStdString( "set." + name ) ) ) {
            QSignalBlocker block(w);
            if ( ! w->hasFocus() )
                w->setValue( pair.first );
        }
        if ( auto w = findChild< act_widget_t * >( QString::fromStdString( "act." + name ) ) ) {
            QSignalBlocker block(w);
            w->setValue( pair.second );
        }
    }
}


void
hvTuneWidget::handleReply( const QString& ev, const QByteArray& data )
{
    boost::property_tree::ptree pt;
    try {
        std::stringstream is( data.constData() );
        boost::property_tree::read_json( is, pt  );
    } catch ( std::exception& ex ) {
        ADDEBUG() << ex.what();
    }

    if ( auto data = pt.get_child_optional( "tick" ) ) {
        if ( auto tc = data->get_optional< int >( "tick" ) ) {
            std::ostringstream o;
            // o << "tick: " << tc.get();
            if ( auto time = data->get_optional<int>( "time" ) ) {
                int days = time.get() / (3600 * 24);
                int hh = ( time.get() % (3600 * 24) ) / 3600;
                int mm = ( time.get() / 60 ) % 60;
                int ss = time.get() % 60;
                if ( days >= 2 )
                    o << boost::format( "%ddays %02d:%02d:%02d" ) % days % hh % mm % ss;
                else if ( days >= 1 )
                    o << boost::format( "%dday %02d:%02d:%02d" ) % days % hh % mm % ss;
                else
                    o << boost::format( "%02d:%02d:%02d" ) % hh % mm % ss;
            }

            if ( auto nsec = data->get_optional<int>( "nsec" ) )
                o << boost::format( ".%03d" ) % ( nsec.get() / 1000000 );

            if ( auto edit = findChild< QLineEdit * >( "eventText" ) )
                edit->setText( QString::fromStdString( o.str() ) );
        }
    }

    if ( auto values = pt.get_child_optional( "tick.hv.values" ) ) {

        BOOST_FOREACH( const boost::property_tree::ptree::value_type& child, values.get() ) {

            const boost::property_tree::ptree& item = child.second;

            if ( auto name = item.get_optional< std::string >( "name" ) ) {

                if ( name.get() == "aux1" ) {  // id = "0"
                    if ( auto set = item.get_optional< uint32_t >( "set" ) ) {
                        std::bitset< 16 > onoff( set.get() );
                        if ( onoff != impl_->aux1_ ) {
                            if ( onoff.any() )
                                impl_->fsm_.process_event( hvfsm::Start() );
                            else
                                impl_->fsm_.process_event( hvfsm::Stop() );
                            impl_->aux1_ = onoff;
                        }
                    }
                    if ( auto alarm1 = item.get_optional< uint32_t >( "act" ) ) {
                        impl_->alarm1_ = std::bitset< 12 >( alarm1.get() );
                    }

                } else if ( name.get() == "pump/valve" ) { // id="15"
                    if ( auto set = item.get_optional< uint32_t >( "set" ) ) {
                        if ( auto w = findChild< QPushButton * >( "switch.isovalve" ) ) {
                            QSignalBlocker block( w );
                            w->setChecked( set.get() & 0x1000 );
                        }
                        if ( auto w = findChild< QPushButton * >( "switch.refinlet" ) ) {
                            QSignalBlocker block( w );
                            w->setChecked( set.get() & 0x0800 );
                        }
                    }
                } else if ( name.get() == "aux2" ) {
                    if ( auto set = item.get_optional< uint32_t >( "set" ) ) {
                        if ( auto w = findChild< QPushButton * >( "switch.filsel" ) ) {
                            QSignalBlocker block( w );
                            w->setChecked( set.get() & 0x0001 );
                        }
                    }

                } else if ( name.get() == "Guage" ) {
                    if ( auto a_press = item.get_optional< double >( "act" ) ) {
                        double p = std::pow( 10.0, ( ( a_press.get() - 7.75 ) / 0.75 ) + 2 );
                        if ( auto w = findChild< QLineEdit * >( "pressure.analyzer" ) )
                            w->setText( QString::number( p, 'e', 3 ) );
                    }

                } else if ( name.get() == "Rev.FPGA" || name.get() == "Rev.CPLD" ) {
                    if ( auto act = item.get_optional< std::string >( "act" ) ) {
                        if ( auto w = findChild< QLineEdit * >( QString::fromStdString( "act." + name.get() ) ) )
                            w->setText( QString::fromStdString( act.get() ) );
                    }

                } else if ( name.get().find( "Vinj" ) != std::string::npos ) {
                    impl_->setSectorVoltage( idVinj, name->find( ".in" ) != std::string::npos, json_value::get< double >( item ) );
                } else if ( name.get().find( "Vext" ) != std::string::npos ) {
                    impl_->setSectorVoltage( idVext, name->find( ".in" ) != std::string::npos, json_value::get< double >( item ) );
                } else if ( name.get().find( "Vtn" ) != std::string::npos ) {
                    impl_->setSectorVoltage( idVtn, name->find( ".in" ) != std::string::npos, json_value::get< double >( item ) );
                } else if ( name.get().find( "Vgate" ) != std::string::npos ) {
                    impl_->setSectorVoltage( idVgate, name->find( ".in" ) != std::string::npos, json_value::get< double >( item ) );
                } else {
                    auto pair = json_value::get< double >( item );
                    if ( auto w = findChild< QDoubleSpinBox * >( QString::fromStdString( "set." + name.get() ) ) ) {
                        QSignalBlocker block( w );
                        if ( ! w->hasFocus() )
                            w->setValue( pair.first );
                    }
                    if ( auto w = findChild< act_widget_t * >( QString::fromStdString( "act." + name.get() ) ) ) {
                        w->setValue( pair.second );
                    }
                }
            }
        }
        uiUpdateSectorVoltages();
    }

    if ( auto adc = pt.get_child_optional( "tick.adc" ) ) {
        // "tp", "nacc"
        if ( auto values = adc->get_child_optional( "values" ) ) {

            std::vector< double > voltages;

            for ( auto& item: values.get() )
                voltages.emplace_back( item.second.get_value< double >() );

            if ( ! voltages.empty() ) {
                double vGuage = voltages[ 0 ] * 4;
                if ( auto w = findChild< QLineEdit * >( "pressure.ionsource" ) ) {
                    double p = std::pow( 10.0, ( ( vGuage - 7.75 ) / 0.75 ) + 2 );
                    w->setText( QString("%1(%2V)" )
                                .arg( QString::number( p, 'e', 3 ), QString::number( vGuage, 'f', 2 ) ) );
                }
            }
        }
    }

    if ( auto alarm = pt.get_optional< std::string >( "tick.alarms.alarm.text" ) ) {
        if ( auto w = findChild< QTextEdit * >( "alarms" ) )
            w->setText( QString::fromStdString( alarm.get() ) );
    }
}

QString
hvTuneWidget::server() const
{
    return QString::fromStdString( impl_->server() );
}


void
hvTuneWidget::handleSwitchToggled( QObject * obj, bool checked )
{
    if ( auto btn = qobject_cast< QPushButton * >( obj ) ) {

        if ( obj->objectName() == "switch.onoff" ) {
            btn->setText( btn->isChecked() ? "HV is ON" : "HV is OFF" );
        } else if ( obj->objectName() == "switch.filsel" ) {
            btn->setText( btn->isChecked() ? "B" : "A" );
        } else {
            btn->setText( btn->isChecked() ? "Open" : "Close" );
        }
    }
    emit dataChanged();
}

void
hvTuneWidget::handleSwitchClicked( QObject * obj, bool checked )
{
    if ( auto btn = qobject_cast< QPushButton * >( obj ) ) {
        ADDEBUG() << __FUNCTION__ << "::" << obj->objectName().toStdString() << " checked=" << checked;

        adurl::ajax ajax( impl_->server_, impl_->port_ );

        boost::property_tree::ptree pt;
        boost::property_tree::ptree child;
        do {
            boost::property_tree::ptree set;
            set.put( "id", obj->property( "id" ).toString().toStdString() );
            set.put( "value", checked );
            child.push_back( std::make_pair( "", set ) );
        } while ( 0 );
        pt.add_child( "checkbox", child );

        std::ostringstream o;
        boost::property_tree::write_json( o, pt, false );
        std::string json = o.str().substr( 0, o.str().find_first_of( "\r\n" ) );

        ADDEBUG() << json;

        if ( auto res = ajax( "POST", "/hv/api$checkbox", std::move( json ), "application/json" ) ) {
            ADDEBUG() << "POST success: " << res.get(); // ajax.status_code() << ", " << ajax.status_message();
        } else {
            ADDEBUG() << "POST failed: " << ajax.status_code() << ", " << ajax.status_message();
        }
    }
    emit dataChanged();
}

void
hvTuneWidget::handleValueChanged( QObject * obj, double value )
{
    const int id = obj->property( "id" ).toInt();
    adurl::ajax ajax( impl_->server_, impl_->port_ );

    boost::property_tree::ptree pt;
    boost::property_tree::ptree child;
    boost::property_tree::ptree set;
    set.put( "id", id );
    set.put( "value", value );
    child.push_back( std::make_pair( "", set ) );

    pt.add_child( "voltage", child );

    std::ostringstream o;
    boost::property_tree::write_json( o, pt, false );
    std::string json = o.str().substr( 0, o.str().find_first_of( "\r\n" ) );

    if ( auto res = ajax( "POST", "/hv/api$setvoltage", std::move( json ), "application/json" ) ) {
#if !defined NDEBUG
        ADDEBUG() << "POST success: " << res.get(); // ajax.status_code() << ", " << ajax.status_message();
#endif
    } else {
        ADDEBUG() << "POST failed: " << ajax.status_code() << ", " << ajax.status_message();
    }
    emit dataChanged();
}

void
hvTuneWidget::handleSectorValueChanged( QObject * obj, double value )
{
    int id = obj->property( "id" ).toInt();

    idSector idSector = sector_id::sectorId( id );
    idPolarity polarity = sector_id::polarity( id );

    if ( idSector == idError || polarity == idNone )
        return;

    auto idpair = sector_id::sector_idpair( id );

    boost::property_tree::ptree pt;
    boost::property_tree::ptree child;
    if ( impl_->isRelative_ ) {
        // Relative
        double delta(0), outer, inner;
        std::tie( outer, inner ) = impl_->sectorSetpt( idSector );

        if ( polarity == idInner ) {
            double centre = outer - ( outer - inner ) / 2.0;
            outer = centre + value / 2.0;  // value is delta = outer - inner
            inner = centre - value / 2.0;
        } else {         // idOuter
            delta = ( outer - inner );
            outer = value + delta / 2.0;  // value is centre voltage
            inner = value - delta / 2.0;
        }
        for ( auto pair: { std::make_pair( idpair.first, outer ), std::make_pair( idpair.second, inner ) } ) {
            boost::property_tree::ptree set;
            set.put( "id", pair.first );
            set.put( "value", pair.second );
            child.push_back( std::make_pair( "", set ) );
        }
    } else {
        // Absolute
        boost::property_tree::ptree set;
        set.put( "id", id );
        set.put( "value", value );
        child.push_back( std::make_pair( "", set ) );
    }
    pt.add_child( "voltage", child );

    std::ostringstream o;
    boost::property_tree::write_json( o, pt, false );
    std::string json = o.str().substr( 0, o.str().find_first_of( "\r\n" ) );
#ifndef NDEBUG
    ADDEBUG() << json;
#endif
    adurl::ajax ajax( impl_->server_, impl_->port_ );

    if ( auto res = ajax( "POST", "/hv/api$setvoltage", std::move( json ), "application/json" ) ) {
#ifndef NDEBUG
        ADDEBUG() << "POST success: " << res.get(); // ajax.status_code() << ", " << ajax.status_message();
#endif
    } else {
        ADDEBUG() << "POST failed: " << ajax.status_code() << ", " << ajax.status_message();
    }

    if ( auto sbx = qobject_cast< QDoubleSpinBox * >( obj ) ) {
    }

    emit dataChanged();
}

void
hvTuneWidget::handleModeChanged( int mode )
{
    if ( mode == 0 ) {
        // absolute
        impl_->isRelative_ = false;
        for ( auto& sector: sector_names ) {
            if ( auto label = findChild< QLabel * >( std::get< 0 >( sector ).c_str() ) )
                label->setText( std::get< 1 >( sector ) );
        }

    } else {
        // relative
        impl_->isRelative_ = true;
        for ( auto& sector: sector_names ) {
            if ( auto label = findChild< QLabel * >( std::get< 0 >( sector ).c_str() ) )
                label->setText( std::get< 3 >( sector ) );
        }
    }
}


QString
hvTuneWidget::setpts() const
{
    boost::property_tree::ptree pt;
    boost::property_tree::ptree voltages, switches;

    for ( auto& sector: sector_names ) {
        uint32_t id = std::get< 4 >( sector ); // msi-id
        if ( sector_id::sectorId( id ) != idError ) {

            boost::property_tree::ptree set;
            const auto sector_voltage = impl_->sector_voltages_[ sector_id::sectorId( id ) ];

            if ( sector_id::polarity( id ) == idOuter ) {
                set.put( "id", sector_id::sector_idpair( id ).first );
                set.put( "name", sector_name_from_id( sector_id::sector_idpair( id ).first ) );
                set.put( "value", sector_voltage.outer.first );
                voltages.push_back( std::make_pair( "", set ) );
            } else {
                set.put( "id", sector_id::sector_idpair( id ).second );
                set.put( "name", sector_name_from_id( sector_id::sector_idpair( id ).second ) );
                set.put( "value", sector_voltage.inner.first );
                voltages.push_back( std::make_pair( "", set ) );
            }
        }
    }

    for ( auto& item: item_names ) {
        const std::string& name = std::get< 0 >( item );
        uint32_t id = std::get< 3 >( item );
        if ( auto w = findChild< QDoubleSpinBox * >( QString::fromStdString( "set." + name ) ) ) {
            boost::property_tree::ptree set;
            double value = w->value();
            set.put( "id", id );
            set.put( "name", name );
            set.put( "value", value );
            voltages.push_back( std::make_pair( "", set ) );
        }
    }
    pt.add_child( "voltage", voltages );

    for ( auto objid: { "switch.isovalve", "switch.refinlet", "switch.filsel" } ) {
        if ( auto w = findChild< QPushButton * >( objid ) ) {
            boost::property_tree::ptree set;
            set.put( "id", w->property( "id" ).toString().toStdString() );
            set.put( "value", w->isChecked() );
            switches.push_back( std::make_pair( "", set ) );
        }
    }
    pt.add_child( "checkbox", switches );

    std::ostringstream o;
    boost::property_tree::write_json( o, pt, true );

    return QString::fromStdString( o.str() );
}

void
hvTuneWidget::setSetpts( const boost::property_tree::ptree& pt )
{
    impl_->fsm_.process_event( hvfsm::Stop() );

    impl_->isLoading_ = true;
    scoped_flag< bool > flag( impl_->isLoading_ );

    adurl::ajax ajax( impl_->server_, impl_->port_ );

    if ( auto voltages = pt.get_child_optional( "voltage" ) ) {
        boost::property_tree::ptree top;
        top.add_child( "voltage", voltages.get() );
        std::ostringstream o;
        boost::property_tree::write_json( o, top, false );
        std::string json = o.str().substr( 0, o.str().find_first_of( "\r\n" ) );

        if ( auto res = ajax( "POST", "/hv/api$setvoltage", std::move( json ), "application/json" ) ) {
#ifndef NDEBUG
            ADDEBUG() << "POST reply: " << res.get();
#endif
        } else {
            ADDEBUG() << "POST failed: " << ajax.status_code() << ", " << ajax.status_message();
        }
    }

    if ( auto checkboxes = pt.get_child_optional( "checkbox" ) ) {
        boost::property_tree::ptree top;
        top.add_child( "checkbox", checkboxes.get() );
        std::ostringstream o;
        boost::property_tree::write_json( o, top, false );
        std::string json = o.str().substr( 0, o.str().find_first_of( "\r\n" ) );

        if ( auto res = ajax( "POST", "/hv/api$checkbox", std::move( json ), "application/json" ) ) {
#ifndef NDEBUG
            ADDEBUG() << "POST reply: " << res.get();
#endif
        } else {
            ADDEBUG() << "POST failed: " << ajax.status_code() << ", " << ajax.status_message();
        }
    }
}
