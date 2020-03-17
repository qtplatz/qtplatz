// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2020 MS-Cheminformatics LLC
**
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminfomatics.
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
#include "samplemethodform.hpp"
#include <qtwrapper/make_widget.hpp>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>

namespace {
    struct radioButton {
        static adcontrols::Quan::QuanInlet
        toSampleMethod( const QString& objname ) {
            if ( objname == "radioChromatogr" )
                return adcontrols::Quan::Chromatography;
            else if ( objname == "radioCounting" )
                return adcontrols::Quan::Counting;
            else if ( objname == "radioExport" )
                return adcontrols::Quan::ExportData;
            else
                return {};
        }
    };
}

using namespace quan;

SampleMethodForm::SampleMethodForm( QWidget *parent ) : QWidget( parent )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {
        if ( auto grpBox = new QGroupBox( tr("Sample Method") ) ) {
            if ( auto hLayout = new QHBoxLayout( grpBox ) ) {
                hLayout->addWidget( qtwrapper::make_widget< QRadioButton >( "radioChromatogr", "Chromatography" ) );
                hLayout->addWidget( qtwrapper::make_widget< QRadioButton >( "radioCounting",   "Counting" ) );
                hLayout->addWidget( qtwrapper::make_widget< QRadioButton >( "radioExport",     "Export(Peak Lists)" ) );
            }
            topLayout->addWidget( grpBox );

            for ( auto radio: findChildren< QRadioButton * >() ) {
                connect( radio, &QRadioButton::clicked
                         , [this,radio]( bool value ){
                               if ( value ) {
                                   emit onSampleMethodChanged(  radioButton::toSampleMethod( radio->objectName() ) );
                               }
                           });
            }
        }
    }
}

SampleMethodForm::~SampleMethodForm()
{
}

adcontrols::Quan::QuanInlet
SampleMethodForm::currSelection() const
{
    for ( auto radio: findChildren< QRadioButton * >() ) {
        if ( radio->isChecked() ) {
            return radioButton::toSampleMethod( radio->objectName() );
        }
    }
    return {};
}

void
SampleMethodForm::setSelection( adcontrols::Quan::QuanInlet inlet )
{
    QRadioButton * radio(0);

    using namespace adcontrols;
    switch( inlet ) {
    case adcontrols::Quan::Chromatography:
        radio = findChild< QRadioButton * >( "radioChromatogr" ); break;
    case adcontrols::Quan::Infusion:
    case adcontrols::Quan::Counting:
        radio = findChild< QRadioButton * >( "radioCounting" ); break;
    case adcontrols::Quan::ExportData:
        radio = findChild< QRadioButton * >( "radioExport" ); break;
    }
    if ( radio )
        radio->setChecked( true );
}
