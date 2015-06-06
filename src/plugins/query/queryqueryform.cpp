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

#include "queryqueryform.hpp"
#include "ui_queryqueryform.h"
#include <QStringList>

using namespace query;

QueryQueryForm::QueryQueryForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::QueryQueryForm)
                                              , semiColonCaptured_( false )
{
    ui->setupUi(this);
    ui->comboBox->clear();
    ui->comboBox->addItems( QStringList()
                            << tr("View all (simple)")
                            << tr("View STD (simple)")
                            << tr("View UNK (simple)")
                            << tr("View amounts")
                            << tr("Viwe all")
                            << tr("View STD")
                            << tr("View UNK")
                            << tr("Calibration") );
    ui->plainTextEdit->installEventFilter( this );
}

QueryQueryForm::~QueryQueryForm()
{
    delete ui;
}

void
QueryQueryForm::setSQL( const QString& t )
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->insertPlainText( t );
}

QString
QueryQueryForm::sql() const
{
    return ui->plainTextEdit->toPlainText();
}

void 
QueryQueryForm::on_plainTextEdit_textChanged()
{
}

void 
QueryQueryForm::on_pushButton_pressed()
{
    emit triggerQuery( ui->plainTextEdit->toPlainText() );
}

void 
QueryQueryForm::on_comboBox_currentIndexChanged(int index)
{
    int idx = 0;
    if ( index == idx++ ) { // view all (simple)
        setSQL("\
SELECT dataSource, QuerySample.name, level, formula, mass, intensity, sampletype FROM QuerySample, QueryResponse \
WHERE QuerySample.id = QueryResponse.idSample ORDER BY mass" );

    } else if ( index == idx++ ) { // view STD (simple)

        setSQL("\
SELECT dataSource, QuerySample.name, level, formula, mass, intensity, sampletype FROM QuerySample, QueryResponse \
WHERE QuerySample.id = QueryResponse.idSample AND sampleType = 1 ORDER BY mass" );

    } else if ( index == idx++ ) { // view UNK (simple)

        setSQL("\
SELECT dataSource, QuerySample.name, level, formula, mass, intensity, sampletype FROM QuerySample, QueryResponse \
WHERE QuerySample.id = QueryResponse.idSample AND sampleType = 0 ORDER BY mass" );

    } else if ( index == idx++ ) { // view amounts

        setSQL("\
SELECT QueryCompound.id, QueryCompound.formula, QueryCompound.description, QueryAmount.level, QueryAmount.amount\n\
FROM QueryAmount, QueryCompound WHERE QueryAmount.idCompound = QueryCompound.id ORDER BY QueryCompound.id" );

    } else if ( index == idx++ ) { // view all

        setSQL("\
SELECT QuerySample.name, QuerySample.level, QueryCompound.formula, QueryCompound.mass AS \"exact mass\", QueryResponse.mass, intensity \
, QueryCompound.description, sampletype, dataSource \
FROM QuerySample, QueryResponse, QueryCompound \
WHERE QuerySample.id = QueryResponse.idSample \
AND QueryResponse.idCmpd = QueryCompound.uuid \
ORDER BY QueryCompound.id, QuerySample.level");

    } else if ( index == idx++ ) { // view STD

        setSQL("\
SELECT QuerySample.name, QueryCompound.formula, QueryCompound.mass AS \"exact mass\", QueryResponse.mass\
, QueryCompound.mass - QueryResponse.mass AS 'error(Da)', intensity, QuerySample.level, QueryAmount.amount, QueryCompound.description, sampleType, dataSource \
FROM QuerySample, QueryResponse, QueryCompound, QueryAmount \
WHERE QuerySample.id = QueryResponse.idSample \
AND QueryResponse.idCmpd = QueryCompound.uuid \
AND sampleType = 1 \
AND QueryAmount.idCompound = QueryCompound.id AND QueryAmount.level = QuerySample.level \
ORDER BY QueryCompound.id, QuerySample.level");

    } else if ( index == idx++ ) { // view UNK

        setSQL("\
SELECT QuerySample.name, QueryCompound.formula, QueryCompound.mass AS \"exact mass\", QueryResponse.mass\
, QueryCompound.mass - QueryResponse.mass AS 'error(Da)', intensity, QueryResponse.amount, QueryCompound.description, dataSource \
FROM QuerySample, QueryResponse, QueryCompound \
WHERE QuerySample.id = QueryResponse.idSample \
AND QueryResponse.idCmpd = QueryCompound.uuid \
AND sampleType = 0 \
ORDER BY QueryCompound.id");

    } else if ( index == idx++ ) { // Calibration

        setSQL("\
SELECT idCompound, formula, description, n\
, a AS 'Y = a'\
, b AS '+ b&sdot;X'\
, c AS '+ c&sdot;X<sup>2</sup>'\
, d AS '+ d&sdot;X<sup>3</sup>'\
, e AS '+ e&sdot;X<sup>4</sup>'\
, f AS '+ f&sdot;X<sup>5</sup>'\
, min_x, max_x, date  from QueryCalib, QueryCompound WHERE idCompound = QueryCompound.id" );
    };

}

bool
QueryQueryForm::eventFilter( QObject * object, QEvent * event )
{
    if ( object == ui->plainTextEdit && event->type() == QEvent::KeyPress ) {
        if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event) ) {
            if ( keyEvent->key() == ';' )
                semiColonCaptured_ = true;
            else if ( keyEvent->key() == Qt::Key_Return && semiColonCaptured_ )
                emit triggerQuery( ui->plainTextEdit->toPlainText() );
            else
                semiColonCaptured_ = false;
        }
    }
    return QWidget::eventFilter( object, event );
}
