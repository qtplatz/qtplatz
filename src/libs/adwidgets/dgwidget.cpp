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

#include "dgwidget.hpp"
#include "dgprotocolform.hpp"
#include "hostaddrdialog.hpp"
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycle.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QMouseEvent>
#include <QTabWidget>
#include <QtGlobal>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <chrono>
#include <memory>
#include <mutex>

namespace adwidgets {
    class MouseEventFilter : public QObject {
        Q_OBJECT
        bool eventFilter( QObject * obj, QEvent * ev ) Q_DECL_OVERRIDE {
            if ( (ev->type() == QEvent::MouseButtonRelease ) && obj->isWidgetType() )
                if ( static_cast< QMouseEvent * >(ev)->button() == Qt::RightButton )
                    emit mouseEvent( qobject_cast< QWidget * >( obj ), static_cast< QMouseEvent * >( ev ) );
            return false;
        }
    public:
        MouseEventFilter( QObject * parent = 0 ) : QObject( parent ) {}
        void installOn( QWidget * w ) {
            w->installEventFilter( this );
        }
    signals:
        void mouseEvent( QWidget *, QMouseEvent * );
    };
}

using namespace adwidgets;

dgWidget::dgWidget( QWidget * parent ) : QFrame( parent )
{
    using namespace qtwrapper;

    resize( 200, 200 );
    
    if ( auto layout = new QVBoxLayout( this ) ) {

        if ( auto gridLayout = new QGridLayout() ) {

            gridLayout->setVerticalSpacing( 0 );
            
            const int row = 0;
            int col = 0;

            // url
            gridLayout->addWidget( new QLabel( "URL" ), row, col++ );
            
            if ( auto url = make_widget< QLineEdit >( "url" ) ) {
                gridLayout->addWidget( url, row, col++ );

                eventFilter_ = std::make_unique< MouseEventFilter >();
                eventFilter_->installOn( url );
                connect( eventFilter_.get(), &MouseEventFilter::mouseEvent, [&](QWidget *, QMouseEvent * ev){
                        QMenu menu;
                        menu.addAction( tr("Edit"), [this]{
                                HostAddrDialog dlg( this );
                                dlg.setModal( true );
                                if ( auto w = findChild< QLineEdit * >( "url" ) ) {
                                    dlg.setUrl( w->text() );
                                    if ( dlg.exec() )
                                        emit hostChanged( dlg.hostAddr().first, dlg.hostAddr().second );
                                }
                            });
                        menu.exec( ev->globalPos() );
                    });
            }

            // interval
            gridLayout->addWidget( new QLabel( "Interval (ms)" ), row, col++ );
            if ( auto spin = make_widget< QDoubleSpinBox >("interval") ) {
            	spin->setMaximum( 10000.0 ); // 10s
            	spin->setMinimum( 0.0 );
            	gridLayout->addWidget( spin, row, col++ );
            }

            //tick
            gridLayout->addWidget( new QLabel( "Tick" ), row, col++ );
            gridLayout->addWidget( make_widget< QLineEdit >( "eventText", "tick" ), row, col++ );

            gridLayout->setColumnStretch(1,2);

            layout->addLayout( gridLayout );
        }
        
        if ( auto tab = new QTabWidget() ) {
            tab->addTab( new dgProtocolForm(), "Proto1" );
            layout->addWidget( tab );
            tab->setStyleSheet( "QTabBar::tab { height: 25px }" );
        }
        setStyleSheet( "QTableView: { color: blue; font-size: 8pt}" );
    }
    
    // connect( impl_, &impl::onReply, this, &dgWidget::handleReply );
    // connect( impl_, &impl::onProtocols, this, &dgWidget::handleProtocols );
    if ( auto url = findChild< QLineEdit *>( "url" ) ) {
        url->setReadOnly( true );
        url->setContextMenuPolicy( Qt::CustomContextMenu );
    }

    setStyleSheet("QLineEdit[readOnly=\"true\"] {"
                  "color: #808080;"
                  "background-color: #F0F0F0;"
                  "border: 1px solid #B0B0B0;"
                  "border-radius: 2px;}");    
}

dgWidget::~dgWidget()
{
}

void
dgWidget::OnCreate( const adportable::Configuration& )
{
}

void
dgWidget::OnInitialUpdate()
{
    // if ( auto edit = findChild< QLineEdit * >( "url" ) )
    //     edit->setText( QString( "http://%1/dg/ctl?events" ).arg( impl_->server().c_str() ) );
}

void
dgWidget::OnFinalClose()
{
}

bool
dgWidget::getContents( boost::any& ) const
{
    return false;
}

bool
dgWidget::setContents( boost::any&& )
{
    return false;
}

QSize
dgWidget::sizeHint() const
{
    return QSize( this->size().width(), 80 );
}

void
dgWidget::handleJson( const QJsonDocument& jdoc )
{
    if ( jdoc.isNull() ) {
        return;
    }
    auto jobj = jdoc.object();
    auto tick = jobj["tick"].toObject();
    auto data = tick["data"].toString();
    if ( !data.isNull() )
        ADDEBUG() << data.toStdString();
    //qDebug() << tick;
#if 0    
    auto adc = tick["adc"].toObject();
    auto nacc = adc["nacc"].toString().toLong();
    auto tp   = adc["tp"].toString().toLongLong();
    auto advalues = adc["values"].toArray();
    double adc_x = ( advalues[0].toDouble() - 1.250 ) * 2;
    double adc_y = ( advalues[1].toDouble() - 1.250 ) * 2;
    ADDEBUG() << "nacc=" << nacc << ", tp=" << tp << " adc_x=" << adc_x << " y=" << adc_y;
#endif
}

void
dgWidget::handleSSE( const QByteArray data )
{
    //auto tp0 = std::chrono::steady_clock::now();
    auto jdoc = QJsonDocument::fromJson( data );
    if ( jdoc.isNull() ) {
        if ( data.size() ) {
            if ( auto w = findChild< QLineEdit * >( "eventText" ) )
                w->setText( QString::fromLatin1( data.data() ) );
        }
        //ADDEBUG() << "Empty json, " << data.size() << "\n" << data.constData();
        return;
    }
    
#if 0
    auto dur0 = std::chrono::steady_clock::now() - tp0;
    //std::istringstream in( data.constData() );
    boost::iostreams::basic_array_source< char > device( data.constData(), data.size() );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > in( device );
    try {
        auto tp1 = std::chrono::steady_clock::now();
        boost::property_tree::ptree pt;
        boost::property_tree::read_json( in, pt );
        auto dur1 = std::chrono::steady_clock::now() - tp1;
        //ADDEBUG() << pt;
        ADDEBUG() << std::chrono::nanoseconds( dur0 ).count()
                  << ", " << std::chrono::nanoseconds( dur1 ).count()
                  << " delta=" << double( std::chrono::nanoseconds( dur1 - dur0 ).count() ) / 1000 << "us";
    } catch ( std::exception& ex ) {
        ADDEBUG() << ex.what();
    }
#endif
}

void
dgWidget::setURL( const QString& url )
{
    if ( auto edit = findChild< QLineEdit * >("url") )
        edit->setText( url );
}

#include "dgwidget.moc"
