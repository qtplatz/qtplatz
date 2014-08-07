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

#include "quanqueryform.hpp"
#include "ui_quanqueryform.h"
#include <QStringList>

using namespace quan;

QuanQueryForm::QuanQueryForm(QWidget *parent) :  QWidget(parent)
                                              , ui(new Ui::QuanQueryForm)
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

QuanQueryForm::~QuanQueryForm()
{
    delete ui;
}

void
QuanQueryForm::setSQL( const QString& t )
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->insertPlainText( t );
}

QString
QuanQueryForm::sql() const
{
    return ui->plainTextEdit->toPlainText();
}

void 
QuanQueryForm::on_plainTextEdit_textChanged()
{
}

void 
QuanQueryForm::on_pushButton_pressed()
{
    emit triggerQuery( ui->plainTextEdit->toPlainText() );
}

void 
QuanQueryForm::on_comboBox_currentIndexChanged(int index)
{
    int idx = 0;
    if ( index == idx++ ) { // view all (simple)
        setSQL("\
SELECT dataSource, QuanSample.name, level, formula, mass, intensity, sampletype FROM QuanSample, QuanResponse \
WHERE QuanSample.id = QuanResponse.idSample ORDER BY mass" );

    } else if ( index == idx++ ) { // view STD (simple)

        setSQL("\
SELECT dataSource, QuanSample.name, level, formula, mass, intensity, sampletype FROM QuanSample, QuanResponse \
WHERE QuanSample.id = QuanResponse.idSample AND sampleType = 1 ORDER BY mass" );

    } else if ( index == idx++ ) { // view UNK (simple)

        setSQL("\
SELECT dataSource, QuanSample.name, level, formula, mass, intensity, sampletype FROM QuanSample, QuanResponse \
WHERE QuanSample.id = QuanResponse.idSample AND sampleType = 0 ORDER BY mass" );

    } else if ( index == idx++ ) { // view amounts

        setSQL("\
SELECT QuanCompound.id, QuanCompound.formula, QuanCompound.description, QuanAmount.level, QuanAmount.amount\n\
FROM QuanAmount, QuanCompound WHERE QuanAmount.idCompound = QuanCompound.id ORDER BY QuanCompound.id" );

    } else if ( index == idx++ ) { // view all

        setSQL("\
SELECT QuanSample.name, QuanSample.level, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass, intensity \
, QuanCompound.description, sampletype, dataSource \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanSample.id = QuanResponse.idSample \
AND QuanResponse.idCmpd = QuanCompound.uuid \
ORDER BY QuanCompound.id, QuanSample.level");

    } else if ( index == idx++ ) { // view STD

        setSQL("\
SELECT QuanSample.name, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass\
, QuanCompound.mass - QuanResponse.mass AS 'error(Da)', intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource \
FROM QuanSample, QuanResponse, QuanCompound, QuanAmount \
WHERE QuanSample.id = QuanResponse.idSample \
AND QuanResponse.idCmpd = QuanCompound.uuid \
AND sampleType = 1 \
AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level \
ORDER BY QuanCompound.id, QuanSample.level");

    } else if ( index == idx++ ) { // view UNK

        setSQL("\
SELECT QuanSample.name, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass\
, QuanCompound.mass - QuanResponse.mass AS 'error(Da)', intensity, QuanResponse.amount, QuanCompound.description, dataSource \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanSample.id = QuanResponse.idSample \
AND QuanResponse.idCmpd = QuanCompound.uuid \
AND sampleType = 0 \
ORDER BY QuanCompound.id");

    } else if ( index == idx++ ) { // Calibration

        setSQL("\
SELECT idCompound, formula, description, n\
, a AS 'Y = a'\
, b AS '+ b&sdot;X'\
, c AS '+ c&sdot;X<sup>2</sup>'\
, d AS '+ d&sdot;X<sup>3</sup>'\
, e AS '+ e&sdot;X<sup>4</sup>'\
, f AS '+ f&sdot;X<sup>5</sup>'\
, min_x, max_x, date  from QuanCalib, QuanCompound WHERE idCompound = QuanCompound.id" );
    };

}

bool
QuanQueryForm::eventFilter( QObject * object, QEvent * event )
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
