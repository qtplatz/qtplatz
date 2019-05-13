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

#include "hvwidget.hpp"
#include "json_value.hpp"
#include "constants.hpp"
#include <adurl/dg.hpp>
#include <adurl/sse.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QtGlobal>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cmath>
#include <sstream>
#include <memory>
#include <mutex>

namespace infitofwidgets {

    class hvWidget::Impl : public QObject {

        Q_OBJECT

        hvWidget * parent_;
        std::string server_;
        std::string port_;
        std::unique_ptr< adurl::old::sse > sse_;

    public:
        Impl( const std::string& server
              , const std::string& port
              , hvWidget * parent ) : parent_( parent )
                                    , server_( server )
                                    , port_( port ) {

            sse_ = std::make_unique< adurl::old::sse >( server_.c_str(), "/hv/api$events", port_.c_str() );

            sse_->exec( [this]( const char * event, const char * data ) {
                    if ( ( std::strcmp( event, "event: hv.tick" ) == 0 ) && ( std::strncmp( data, "data:", 5 ) == 0 ) ) {
                        data += 5;
                        while ( *data == ' ' || *data == '\t' )
                            ++data;
                        QByteArray a( data, std::strlen( data ) );
                        emit onReply( event, data );
                    }
                });
        }

        ~Impl() {
            if ( sse_ )
                sse_->stop();
        }

        const std::string& server() const { return server_; }

    signals:
        void onReply( const QString&, const QByteArray& );
        void onProtocols( const QByteArray& );
    };

    static const std::tuple< const std::string, QString, double > item_names [] = {
        { "Ifil",         "Filament(mA)",    5000 }
        , { "Vion",         "Ionization(V)",  100 }
        , { "detector",     "Detector(V)",   5000 }
    };

    using qtwrapper::make_widget;

    //static const std::pair< const std::string, QString > item2_names [] = {
    static const std::tuple< const std::string, QString, std::function< QWidget*(void) > >item2_factories [] = {
        { "Vtrp", "Vtrp",                     [](){ return make_widget< QDoubleSpinBox >("act.Vtrp"); } }
        , { "Iion",         "Ionization(mA)", [](){ return make_widget< QDoubleSpinBox >("act.Iion"); } }
        , { "DFP.IS",       "DFP.IS(mA)",     [](){ return make_widget< QDoubleSpinBox >("act.DFP.IS"); } }
        , { "DFP.Analyzer", "DFP.Analyzer",   [](){ return make_widget< QDoubleSpinBox >("act.DFP.Analyzer"); } }
        , { "PStemp",       "P.S. Temp(<sup>o</sup>C)", [](){ return make_widget< QDoubleSpinBox >("act.PStemp"); } }
        , { "Rev.FPGA",     "Rev.FPGA",       [](){ return make_widget< QLineEdit >("act.Rev.FPGA"); } }
        , { "Rev.CPLD",     "Rev.CPLD",       [](){ return make_widget< QLineEdit >("act.Rev.CPLD"); } }
    };

    static const std::tuple< const std::string, QString, std::function< QWidget*(void) > > switch_factories [] = {
        { "isovalve",       "Isolation Valve",   [](){ return make_widget< QPushButton >("switch.isovalve"); } }
        , { "refinlet",     "Reference Inlet",   [](){ return make_widget< QPushButton >("switch.refinlet"); } }
        , { "filsel",       "Filament Sel.",     [](){ return make_widget< QPushButton >("switch.filsel"); } }
    };
}

#include "hvwidget.moc"

using namespace infitofwidgets;

hvWidget::hvWidget( const QString& server
                    , const QString& port
                    , QWidget * parent ) : QFrame( parent )
                                         , impl_( new Impl( server.toStdString(), port.toStdString(), this ) )
{
    using namespace qtwrapper;

    resize( 200, 100 );

    if ( auto layout = new QVBoxLayout( this ) ) {

        if ( auto grid = new QGridLayout() ) {
            int row = 0, col = 0;

            grid->addWidget( make_widget< QLabel >( "pressure.analyzer", "Analyzer(Pa)" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "pressure.analyzer", "" ), row, col++ );

            grid->addWidget( make_widget< QLabel >( "pressure.ionsource", "Ion Source(Pa)" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "pressure.ionsource" ), row, col++ );

            auto onoff = make_widget< QPushButton >( "switch.onoff" );

            grid->addWidget( onoff, row, col++ );

            grid->setColumnStretch( 0, 1 ); // analyzer label
            grid->setColumnStretch( 1, 1 ); // analyzer value
            grid->setColumnStretch( 2, 1 ); // ionsource label
            grid->setColumnStretch( 3, 1 ); // ionsource value
            grid->setColumnStretch( 4, 2 ); // onoff button

            layout->addLayout( grid );
        }

        if ( auto hor = new QHBoxLayout() ) {

            if ( auto gridLayout = new QGridLayout() ) {
                // Left
                gridLayout->setVerticalSpacing( 0 );

                int row = 0;
                int col = 0;
                for ( const auto& name: item_names ) {
                    ++row;
                    col = 0;
                    gridLayout->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                    gridLayout->addWidget( make_widget< QDoubleSpinBox >( ("set." + std::get< 0 >( name ) ).c_str() ), row, col++ );
                    gridLayout->addWidget( make_widget< QDoubleSpinBox >( ("act." + std::get< 0 >( name ) ).c_str() ), row, col++ );
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

        if ( auto grid = new QGridLayout() ) {
            int row = 0;
            int col = 0;
            for ( const auto& name: switch_factories ) {
                grid->addWidget( make_widget< QLabel >( std::get< 0 >( name ).c_str(), std::get< 1 >( name ) ), row, col++ );
                grid->addWidget( std::get< 2 >( name )(), row, col++ );
            }
            layout->addLayout( grid );
        }

        if ( auto grid = new QGridLayout() ) {
            int row = 0, col = 0;

            grid->addWidget( make_widget< QTextEdit >( "alarms" ), row, col, 1, 4 );

            ++row;
            col = 0;
            grid->addWidget( make_widget< QLabel >( "tick", "Elapsd time:" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "eventText", "tick" ), row, col++ );

            grid->addWidget( make_widget< QLabel >( "url", "URL" ), row, col++ );
            grid->addWidget( make_widget< QLineEdit >( "url" ), row, col++ );

            layout->addLayout( grid );
        }

    }

    ////////////////////////////////////////////////////////
    // set widget attributes
    for ( auto label: findChildren< QLabel * >() ) {
        label->setAlignment( Qt::AlignRight | Qt::AlignCenter );
        label->setTextFormat( Qt::RichText );
    }

    for ( auto& item: item_names ) {
        if ( auto spin = findChild< QDoubleSpinBox * >( ( "set." + std::get< 0 >( item ) ).c_str() ) ) {
            spin->setMaximum( std::get< 2 >( item ) );
            spin->setDecimals( 0 );
            spin->setKeyboardTracking( false );
            connect( spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]( double value ){ handleValueChanged( spin, value ); } );
        }
        if ( auto spin = findChild< QDoubleSpinBox * >( ( "act." + std::get< 0 >( item ) ).c_str() ) ) {
            spin->setMaximum( std::get< 2 >( item ) );
            spin->setDecimals( 1 );
            spin->setAlignment( Qt::AlignRight | Qt::AlignCenter );
        }
    }

    for ( auto widget: findChildren< QLineEdit * >( QRegExp( "act\\..*|pressure\\..*" ) ) ) {
        widget->setReadOnly( true );
        widget->setFocusPolicy( Qt::ClickFocus );
    }

    for ( auto widget: findChildren< QAbstractSpinBox * >( QRegExp( "act\\..*|pressure\\..*" ) ) ) {
        widget->setReadOnly( true );
        widget->setFocusPolicy( Qt::ClickFocus );
    }

    for ( auto widget: findChildren< QTextEdit * >( "alarms" ) ) {
        widget->setReadOnly( true );
        widget->setFocusPolicy( Qt::ClickFocus );
    }

    for ( auto& button: findChildren< QPushButton * >( QRegExp( "switch\\..*" ) ) ) {
        button->setCheckable( true );
        button->setChecked( false );
        handleSwitchToggled( button, false );
        connect( button, &QPushButton::toggled, this, [=]( bool checked ){ handleSwitchToggled( button, checked ); } );
    }

    setStyleSheet(
        "QPushButton { background-color: green; }"
        "QPushButton:hover { background-color: yellow; }"
        "QPushButton#switch\\.onoff:checked { background-color: red; }"
        "QPushButton:checked { background-color: blue; }"
        );

    connect( impl_, &Impl::onReply, this, &hvWidget::handleReply );
}

hvWidget::~hvWidget()
{
    delete impl_;
}

void
hvWidget::OnCreate( const adportable::Configuration& )
{
}

void
hvWidget::OnInitialUpdate()
{
    if ( auto edit = findChild< QLineEdit * >( "url" ) )
        edit->setText( QString( "http://%1/hv/api$events" ).arg( impl_->server().c_str() ) );
}

void
hvWidget::OnFinalClose()
{
}

bool
hvWidget::getContents( boost::any& ) const
{
    return false;
}

bool
hvWidget::setContents( boost::any&& )
{
    return false;
}

QSize
hvWidget::sizeHint() const
{
    return QSize( this->size().width(), 80 );
}

void
hvWidget::handleReply( const QString& ev, const QByteArray& data )
{
    static size_t count = 0;

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
                o << boost::format( ".%03d") % ( nsec.get() / 1000000 );

            if ( auto edit = findChild< QLineEdit * >( "eventText" ) )
                edit->setText( QString::fromStdString( o.str() ) );
        }
    }

    if ( auto values = pt.get_child_optional( "tick.hv.values" ) ) {
        BOOST_FOREACH( const boost::property_tree::ptree::value_type& child, values.get() ) {

            const boost::property_tree::ptree& item = child.second;
            std::pair< uint32_t, uint32_t > aux1;

            if ( auto name = item.get_optional< std::string >( "name" ) ) {

                if ( name.get() == "aux1" ) {  // id = "0"

                    // aux1 = json_value::get< uint32_t >( item );

                } else if ( name.get() == "pump/valve" ) { // id="15"
                    if ( auto act = item.get_optional< uint32_t >( "act" ) ) {
                        bool isovalve = act.get() & 0x1000;
                        bool stdinlet = act.get() & 0x0800;
                        bool filsel   = act.get() & 0x0001;
                        if ( auto w = findChild< QPushButton * >( "isovalve" ) )
                            w->setChecked( isovalve );
                        if ( auto w = findChild< QPushButton * >( "refinlet" ) )
                            w->setChecked( stdinlet );
                        if ( auto w = findChild< QPushButton * >( "filsel" ) )
                            w->setChecked( filsel );
                    }

                } else if ( name.get() == "aux2" ) {

                    // auto pair = json_value::get_optional< uint32_t >( item );

                } else if ( name.get() == "Guage" ) {

                    if ( auto a_press = item.get_optional< double >( "act" ) ) {
                        double p = std::pow( 10.0, ( ( a_press.get() - 7.75 ) / 0.75 ) + 2 );
                        if ( auto w = findChild< QLineEdit * >( "pressure.analyzer" ) )
                            w->setText( QString::number( p ) );
                    }

                } else if ( name.get() == "Rev.FPGA" || name.get() == "Rev.CPLD" ) {

                    if ( auto act = item.get_optional< std::string >( "act" ) ) {
                        if ( auto w = findChild< QLineEdit * >( QString::fromStdString( "act." + name.get() ) ) )
                            w->setText( QString::fromStdString( act.get() ) );
                    }

                } else {
                    auto pair = json_value::get< double >( item );
                    if ( auto w = findChild< QDoubleSpinBox * >( QString::fromStdString( "act." + name.get() ) ) )
                        w->setValue( pair.first );
                    if ( auto w = findChild< QDoubleSpinBox * >( QString::fromStdString( "act." + name.get() ) ) )
                        w->setValue( pair.second );
                }
            }
        }
    }

    if ( auto alarm = pt.get_optional< std::string >( "tick.alarms.alarm.text" ) ) {
        if ( auto w = findChild< QTextEdit * >( "alarms" ) )
            w->setText( QString::fromStdString( alarm.get() ) );
    }

}

void
hvWidget::handleProtocols( const QByteArray& json )
{
    if ( auto tab = findChild< QTabWidget * >() ) {

        adio::dg::protocols< adio::dg::protocol<> > p;
        std::istringstream is( json.constData() );

        if ( p.read_json( is, p ) ) {
        }
    }
}


void
hvWidget::handleSwitchToggled( QObject * obj, bool checked )
{
    ADDEBUG() << "handleSwitchToggled: " << obj->objectName().toStdString() << " checked=" << checked;

    if ( auto btn = qobject_cast< QPushButton * >( obj ) ) {

        if ( obj->objectName() == "switch.onoff" ) {
            btn->setText( checked ? "HV is ON" : "HV is OFF" );
        } else if ( obj->objectName() == "switch.filsel" ) {
            btn->setText( checked ? "A" : "B" );
        } else {
            btn->setText( checked ? "Close" : "Open" );
        }
    }
}


void
hvWidget::handleValueChanged( QObject * obj, double value )
{
    ADDEBUG() << "handleValueChanged: " << obj->objectName().toStdString() << " value=" << value;

    if ( auto sbx = qobject_cast< QDoubleSpinBox * >( obj ) ) {
    }
}
