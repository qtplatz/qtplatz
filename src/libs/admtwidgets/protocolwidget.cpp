/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "protocolwidget.hpp"
#include "protocolform.hpp"
#include <admtcontrols/orbitprotocol.hpp>
// #include "scanlawlookupwidget.hpp"
// #include "acqprotocoldetailsform.hpp"
// #include <infitofcontrols/method.hpp>
// #include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <admtcontrols/constants.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msmoltable.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/tableview.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QBoxLayout>
#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMetaType>
#include <QPainter>
#include <QPair>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextDocument>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <functional>
#include <algorithm>

Q_DECLARE_METATYPE( admtcontrols::OrbitProtocol )

using namespace admtwidgets;

namespace admtwidgets {

    class protocolWidget::impl {
    public:
        protocolWidget * pThis_;
        ~impl() {
        }

        impl( protocolWidget * p ) : pThis_( p ) {
        }
    };

#if 0
    struct protocol {
        static bool commit( const QJsonObject& jobj ) {
            if ( jobj.isEmpty() )
                return false;

            auto interval = jobj["interval"].toVariant().toDouble();
            auto pulse_size = jobj["protocol"].toArray().at(0).toObject()["pulses"].toArray().size();

            //if ( fpga::instance()->model_number() >= 0x20190610 ) {  // TMC
                enum { number_of_channels = 9 };

                if ( __debug_mode__ ) {
                    // ADDEBUG() << QJsonDocument( jobj ).toJson().toStdString();
                    ADDEBUG() << "interval=: " << interval << ", pulse.size: " << pulse_size;
                }

                std::vector< adio::dg::protocol< number_of_channels > > protocols;

                for ( const auto& jproto: jobj["protocol"].toArray() ) {
                    adio::dg::protocol< number_of_channels > proto;
                    auto index = jproto.toObject()["index"].toVariant().toInt();
                    proto.setReplicates( jproto.toObject()["replicates"].toVariant().toInt() );

                    size_t idx(0);
                    for ( const auto& jpulse: jproto.toObject()["pulses"].toArray() ) {
                        auto delay = jpulse.toObject()["delay"].toVariant().toDouble();
                        auto width = jpulse.toObject()["width"].toVariant().toDouble();

                        if ( __debug_mode__ && proto.replicates() )
                            ADDEBUG() << boost::format( "CH-%d\t%.3f\t%3f" ) % idx % (delay * 1.0e6) % (width * 1.0e6);

                        if ( idx < number_of_channels )
                            proto[ idx++ ] = std::make_pair( delay, width );

                    }
                    protocols.emplace_back( proto );
                }
                fpga::instance()->commit( protocols, interval );
                return true;
            }
            return false;
        }
    };
#endif

} // infwidgets


protocolWidget::~protocolWidget()
{
    delete impl_;
}

protocolWidget::protocolWidget( QWidget * parent ) : QFrame( parent )
                                                   , impl_( new impl( this ) )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {

        topLayout->setContentsMargins( {} );
        topLayout->setSpacing(0);

        if ( auto center = new QWidget ) {
            topLayout->addWidget( center );
            auto hLayout = new QHBoxLayout( center );
            hLayout->setContentsMargins( {} );
            hLayout->setSpacing( 0 );
            hLayout->addWidget( qtwrapper::make_widget< admtwidgets::protocolForm >( "p.0", 0) );
            hLayout->addWidget( qtwrapper::make_widget< admtwidgets::protocolForm >( "p.1", 1) );
            hLayout->addWidget( qtwrapper::make_widget< admtwidgets::protocolForm >( "p.2", 2) );
            hLayout->addWidget( qtwrapper::make_widget< admtwidgets::protocolForm >( "p.3", 3) );
        }

        if ( auto hLayout = new QHBoxLayout() ) {

            if ( auto applyButton = qtwrapper::make_widget<QPushButton>( "applyButton", tr( "Apply" ) ) ) {
                connect( applyButton, &QPushButton::pressed
                         , [&] () {
                               emit applyTriggered();
                           });
                hLayout->addWidget( applyButton );
            }
            if ( auto fetchButton = qtwrapper::make_widget<QPushButton>( "fetchButton", tr( "Fetch" ) ) ) {
                connect( fetchButton, &QPushButton::pressed
                         , [&] () {
                               emit fetchTriggered();
                           });
                hLayout->addWidget( fetchButton );
            }
            topLayout->addLayout( hLayout );
        }
    }
}

void
protocolWidget::setDirty( bool dirty )
{
    for ( auto form: findChildren< admtwidgets::protocolForm * >() )
        form->setDirty( false );
}

QSize
protocolWidget::sizeHint() const
{
    return QSize( 400, 200 );
}

// adplugin::LifeCycle
void
protocolWidget::OnCreate( const adportable::Configuration& )
{
}

void
protocolWidget::OnInitialUpdate()
{
    setStyleSheet( "* { font-size: 9pt; }" );
}

void
protocolWidget::OnFinalClose()
{
}

bool
protocolWidget::getContents( boost::any& a ) const
{
#if 0
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);

        infitofcontrols::method m;

        auto it = ptr->find( ptr->begin(), ptr->end(), infitofcontrols::method::clsid() );
        if ( it != ptr->end() )
            it->get( *it, m );

        getMethod( m );
        ptr->append( m );

        return true;
    }
    else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {

        auto pi = boost::any_cast<adcontrols::ControlMethod::MethodItem * >( a );

        infitofcontrols::method m;
        getMethod( m );
        pi->set<>( *pi, m );

        return true;
    }
#endif
    return false;
}

bool
protocolWidget::setContents( boost::any&& a )
{
#if 0
    if ( auto pi = adcontrols::ControlMethod::any_cast<>( )( a, infitofcontrols::AvgrMethod::clsid() ) ) {
        infitofcontrols::method m;
        getMethod( m );
        infitofcontrols::AvgrMethod avgr;
        if ( pi->get<>( *pi, avgr ) ) {
            m.tof() = avgr;
            setMethod( m, true );
            return true;
        }
    }
    if ( auto pi = adcontrols::ControlMethod::any_cast<>( )( a, infitofcontrols::method::clsid() ) ) {
        infitofcontrols::method m;
        if ( pi->get<>( *pi, m ) ) {
            setMethod( m, true );
            return true;
        }
    }
#endif
    return false;
}

void
protocolWidget::showContextMenu( const QPoint& pt )
{
}

QByteArray
protocolWidget::toJson( bool pritty ) const
{
    QJsonArray protocols;
    for ( auto& name: { "p.0", "p.1", "p.2", "p.3" } ) {
        if ( auto w = findChild< admtwidgets::protocolForm * >( name ) ) {
            protocols.push_back( w->json() );
        }
    }
    return QByteArray( QJsonDocument{ QJsonObject{ { "protocol", protocols } } }.toJson( pritty ? QJsonDocument::Indented : QJsonDocument::Compact ) );
}

void
protocolWidget::fromJson( const QByteArray& ba )
{
    auto obj = QJsonDocument::fromJson( ba ).object();

    if ( obj.contains( "protocols" ) ) { // fetched
        // dgmod server json format -- from ajax
        auto jv = boost::json::parse( ba.toStdString() );
        // boost::property_tree::ptree pt;
        // std::istringstream in( ba.toStdString() );
        // boost::property_tree::read_json( in, pt );
        auto jproto = adportable::json_helper::find( jv, "protocols.protocol" );
        if ( jproto.is_array() ) {
            for ( const auto& v: jproto.as_array() ) {
                auto index = boost::json::value_to< int >( v.as_object().at( "index" ) );
                if ( auto w = findChild< admtwidgets::protocolForm * >( QString("p.%1").arg( index ) ) ) {
                    w->setJson( v );
                }
            }
        }

        // for ( const auto& v: pt.get_child( "protocols.protocol" ) ) {
        //     if ( auto index = v.second.get_optional< int >( "index" ) ) {
        //         if ( auto w = findChild< admtwidgets::protocolForm * >( QString("p.%1").arg( index.get() ) ) ) {
        //             w->setJson( v.second );
        //         }
        //     }
        // }

    } else {
        // local json format -- from ads54jplugin::document
        auto protocol = QJsonDocument::fromJson( ba ).object()[ "protocol" ].toArray();
        if ( !protocol.empty() ) {
            for ( const auto& p: protocol ) {
                auto jobj = p.toObject();
                if ( jobj.contains( "index" ) ) {
                    if ( auto w = findChild< admtwidgets::protocolForm * >( QString("p.%1").arg( jobj[ "index" ].toInt() ) ) ) {
                        w->setJson( jobj );
                        w->setDirty( false );
                    }
                }
            }
        }
    }
}
