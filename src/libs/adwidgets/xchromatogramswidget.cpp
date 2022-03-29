/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "xchromatogramswidget.hpp"
#include "xchromatogramsform.hpp"
#include "xchromatogramstable.hpp"
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/xchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSplitter>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <ratio>
#include <cmath>

namespace adwidgets {

    class XChromatogramsWidget::impl {
        XChromatogramsWidget * this_;
    public:
        impl( XChromatogramsWidget * p ) : this_( p ) {
            (void)this_;
        }

        ~impl() {
        }

        void handleContextMenu( const QPoint& pt );
        std::shared_ptr< const adcontrols::MassSpectrometer > spectrometer_;
    };

}

using namespace adwidgets;

XChromatogramsWidget::XChromatogramsWidget(QWidget *parent) : QWidget(parent)
                                                                , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new XChromatogramsForm ) );
            splitter->addWidget( ( new XChromatogramsTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
        auto form = findChild< XChromatogramsForm * >();
        auto table = findChild< XChromatogramsTable * >();
        connect( form, &XChromatogramsForm::polarityToggled, table, &XChromatogramsTable::handlePolarity );
        connect( table, &XChromatogramsTable::valueChanged, this, &XChromatogramsWidget::handleValueChanged );
        connect( table, &XChromatogramsTable::editorValueChanged, this
                 , [&]( const QModelIndex index, double value ){ emit editorValueChanged( index, value ); });
    }
}

XChromatogramsWidget::~XChromatogramsWidget()
{
}

void
XChromatogramsWidget::OnCreate( const adportable::Configuration& )
{
}

void
XChromatogramsWidget::OnInitialUpdate()
{
    if ( auto form = findChild< XChromatogramsForm * >() )
        form->OnInitialUpdate();
    if ( auto table = findChild< XChromatogramsTable * >() )
        table->onInitialUpdate();

    setContents( adcontrols::XChromatogramsMethod{} );
}

void
XChromatogramsWidget::onUpdate( boost::any&& )
{
}

void
XChromatogramsWidget::OnFinalClose()
{
}

bool
XChromatogramsWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( getValue() );
        return true;
    }
    return false;
}

bool
XChromatogramsWidget::setContents( boost::any&& a )
{
    if ( auto pi = adcontrols::ControlMethod::any_cast<>()( a, adcontrols::XChromatogramsMethod::clsid() ) ) {
        adcontrols::XChromatogramsMethod m;
        if ( pi->get( *pi, m ) ) {
            setValue( m );
            return true;
        }
    } else {
        ADDEBUG() << __FUNCTION__ << " --------- XChromatogramsMethod NOT found -----------";
    }
    return false;
}

adcontrols::XChromatogramsMethod
XChromatogramsWidget::getValue() const
{
    adcontrols::XChromatogramsMethod m;
    if ( auto form = findChild< XChromatogramsForm * >() )
        form->getContents( m );
    if ( auto table = findChild< XChromatogramsTable * >() )
        table->getContents( m );
    return m;
}

bool
XChromatogramsWidget::setValue( const adcontrols::XChromatogramsMethod& m )
{
    QSignalBlocker block( this );
    if ( auto form = findChild< XChromatogramsForm *>() )
        form->setContents( m );
    if ( auto table = findChild< XChromatogramsTable * >() )
        table->setValue( m );

    return true;
}

void
XChromatogramsWidget::setDigitizerMode( bool softAverage )
{
    if ( auto form = findChild< XChromatogramsForm * >() ) {
        form->setDigitizerMode( softAverage );
    }
}

void
XChromatogramsWidget::impl::handleContextMenu( const QPoint& pt )
{
}

void
XChromatogramsWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    impl_->spectrometer_ = sp;

    ADDEBUG() << sp->calibrationFilename();

    boost::filesystem::path path( sp->calibrationFilename() );
    if ( auto form = findChild< XChromatogramsForm * >() ) {
        form->setCalibrationFilename( QString::fromStdString( path.stem().string() )
                                      , QString::fromStdString( sp->calibrationFilename() ) );
    }
    if ( auto table = findChild< XChromatogramsTable * >() ) {
        table->setMassSpectrometer( sp );
    }
}

void
XChromatogramsWidget::handleValueChanged()
{
    auto jv = boost::json::value_from( getValue() );
    emit valueChanged( QString::fromStdString( boost::json::serialize( jv ) ) );
}
