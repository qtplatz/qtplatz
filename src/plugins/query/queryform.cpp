/**************************************************************************
** Copyright (C) 2010-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "queryform.hpp"
#include "document.hpp"
#include "sqledit.hpp"
#include <utils/styledbar.h>
#include <adportable/debug.hpp>
#include <qtwrapper/make_widget.hpp>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QCompleter>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QStringList>
#include <QStyledItemDelegate>

using namespace query;

QueryForm::QueryForm(QWidget *parent) : QWidget(parent)
                                      , semiColonCaptured_( false )
{
    resize( 200, 100 );

    auto vLayout = new QVBoxLayout( this );

    if ( auto textEditor = new SqlEdit() ) {
        textEditor->installEventFilter( this );
        vLayout->addWidget( textEditor );
    }

    if ( auto toolBar = new Utils::StyledBar ) {
        toolBar->setProperty( "topBorder", true );
        toolBar->setSingleRow( true );
        //toolBar->setLightColored( false );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(2);

        if ( auto combo = new QComboBox() ) {
            combo->setObjectName( "tableList" );
            toolBarLayout->addWidget( combo );
            connect( combo, qOverload< const QString &>(&QComboBox::currentIndexChanged), this, &QueryForm::on_comboBox_currentIndexChanged );
            toolBarLayout->setStretchFactor( combo, 1 );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

        if ( auto label = new QLabel( "TOF(&micro;s)" ) ) {
            label->setTextFormat( Qt::RichText );
            toolBarLayout->addWidget( label );
        }

        if ( auto sbox = qtwrapper::make_widget< QDoubleSpinBox >("sboxTOF") ) {
            sbox->setDecimals( 4 );
            sbox->setSingleStep( 0.001 );
            sbox->setMinimum( 0 );
            sbox->setMaximum( 1000.0 );
            sbox->setValue( document::instance()->tof() * 1.0e6 );
            toolBarLayout->addWidget( sbox );
            connect( sbox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this
                     , [&](double v){
                           document::instance()->setTof( v / 1.0e6 );
                           if ( auto combo = findChild< QComboBox * >("tableList") )
                               on_comboBox_currentIndexChanged( combo->currentText() );
                       } );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        if ( auto label = new QLabel( "Width(ns)" ) )
            toolBarLayout->addWidget( label );

        if ( auto sbox = qtwrapper::make_widget< QDoubleSpinBox >("sboxWidth") ) {
            sbox->setDecimals( 2 );
            sbox->setSingleStep( 0.1 );
            sbox->setMinimum( 0 );
            sbox->setMaximum( 1000.0 );
            sbox->setValue( document::instance()->width() * 1.0e9 );
            toolBarLayout->addWidget( sbox );
            connect( sbox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this
                     , [&](double v){
                           document::instance()->setWidth( v / 1.0e9 );
                           if ( auto combo = findChild< QComboBox * >("tableList") )
                               on_comboBox_currentIndexChanged( combo->currentText() );
                       } );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        if ( auto button = new QPushButton( "Execute query" ) ) {
            toolBarLayout->addWidget( button );
            connect( button, &QPushButton::pressed, this, [&]{ emit on_pushButton_pressed(); } );
        }

        if ( auto button = new QPushButton( "Plot..." ) ) {
            toolBarLayout->addWidget( button );
            connect( button, &QPushButton::pressed, this, [&]{ emit plotButtonPressed(); } );
        }

        vLayout->addWidget( toolBar );
    }
}

QueryForm::~QueryForm()
{
}

void
QueryForm::setSQL( const QString& t )
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )     {
        textEdit->clear();
        textEdit->insertPlainText( t );
    }
}

void
QueryForm::setTableList( const QList< QString >& list )
{
    if ( auto combo = findChild< QComboBox * >( "tableList" ) ) {
        combo->clear();
        combo->addItems( list );
    }
}

// void
// QueryForm::setSqlHistory( const QStringList& list )
// {
//     if ( auto combo = findChild< QComboBox * >( "history" ) ) {
//         combo->clear();
//         combo->addItems( list );
//     }
// }

QString
QueryForm::sql() const
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )
        return textEdit->toPlainText();

    return QString();
}

void
QueryForm::on_plainTextEdit_textChanged()
{
}

void
QueryForm::on_pushButton_pressed()
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )
        emit triggerQuery( textEdit->toPlainText() );
}

void
QueryForm::on_comboBox_currentIndexChanged( const QString& itemText )
{
    double tof = document::instance()->tof();
    double width = document::instance()->width();
    const std::pair< double, double > range{ tof - width / 2, tof + width / 2 };

    ADDEBUG() << "tof: " << tof << ", width: " << width;
    ADDEBUG() << itemText.toStdString() << "; " << range;

    if ( itemText == "{Counting}" ) {
        setSQL( QString( "SELECT ROUND(peak_time, 9) AS time, COUNT(*), protocol  FROM peak,trigger WHERE id=idTrigger GROUP BY time ORDER BY time" ) );
    } else if ( itemText == "{CountRate}" ) {
        setSQL( QString( "SELECT idSample,dataSource,formula,cast(timeCounts AS REAL)/trigCounts as CountRate"
                         " FROM QuanResponse,QuanSample WHERE QuanResponse.idSample=QuanSample.id" ) );
    } else if ( itemText == "{TOF/Intensities -- min(peak_time)}" ) {
        queryItem_ = itemText;
        if ( tof >= 10.0e-9 ) {
            setSQL( QString( "SELECT MIN(peak_time),peak_intensity FROM trigger,peak WHERE id=idTrigger"
                             " AND peak_time > %1 AND peak_time < %2 GROUP BY id" ).arg( QString::number( range.first )
                                                                                         , QString::number( range.second ) ) );
        } else {
            setSQL( QString( "SELECT MIN(peak_time),peak_intensity FROM trigger,peak WHERE id=idTrigger GROUP BY id" ) );
        }
    } else if ( itemText == "{Frequency -- min(peak_time)}" ) {
        queryItem_ = itemText;
        if ( tof >= 10.0e-9 ) {
            setSQL( QString( "SELECT *,COUNT(*) AS COUNTS FROM\n"
                             " (SELECT MIN(peak_time),ROUND(peak_intensity/10)*10 AS Threshold"
                             " FROM trigger,peak WHERE id=idTrigger AND peak_time > %1 AND peak_time < %2 GROUP BY id)\n"
                             " GROUP BY Threshold" ).arg( QString::number( range.first ), QString::number( range.second ) ) );
        } else {
            setSQL( QString( "SELECT *,COUNT(*) AS COUNTS FROM\n"
                             " (SELECT MIN(peak_time),ROUND(peak_intensity/10)*10 AS Threshold"
                             " FROM trigger,peak WHERE id=idTrigger GROUP BY id)\n"
                             " GROUP BY Threshold" ) );
        }
    } else if ( itemText == "{Frequency}" ) {
        queryItem_ = itemText;
        if ( tof >= 10.0e-9 ) {
            setSQL( QString( "SELECT *,COUNT(*) AS COUNTS FROM\n"
                             " (SELECT ROUND(peak_intensity/10)*10 AS Threshold,* FROM trigger,peak WHERE id=idTrigger"
                             " AND peak_time > %1 AND peak_time < %2 AND Threshold < 0.0)\n"
                             " GROUP by Threshold" ).arg( QString::number( range.first ), QString::number( range.second ) ) );
        } else {
            setSQL( QString( "SELECT *,COUNT(*) AS COUNTS FROM\n"
                             " (SELECT ROUND(peak_intensity/10)*10 AS Threshold,* FROM trigger,peak WHERE id=idTrigger AND Threshold < 0.0)\n"
                             " GROUP by Threshold" ) );
        }
    } else if ( itemText == "{Peak Height}" ) {
        queryItem_ = itemText;
        if ( tof >= 10.0e-9 )
            setSQL( QString( "SELECT ROUND(peak_intensity/10)*10 AS Threshold,* FROM trigger,peak WHERE id=idTrigger"
                             " AND peak_time > %1 AND peak_time < %2" ).arg( QString::number( range.first ), QString::number( range.first ) ) );
        else
            setSQL( QString( "SELECT ROUND(peak_intensity/10)*10 AS Threshold,* FROM trigger,peak WHERE id=idTrigger" ) );
    } else if ( itemText == "{CountRatio}" ) {
        setSQL( QString(
                    "SELECT t1.uuid as 'uuid'"
                    ", t1.id as id"
                    ", t1.idSample as idSample"
                    ", t1.formula as formula"
                    ", t1.mass as mass"
                    ", t1.error as 'error(mDa)'"
                    ", t1.CountRate"
                    ", t2.formula as formula"
                    ", t2.CountRate"
                    ", t1.CountRate/t2.CountRate AS 'Ratio'"
                    ", amount"
                    ", trigCounts"
                    ", replace(dataSource,rtrim(dataSource,replace(dataSource,'/','')),'') AS dataSource"
                    "\n FROM"
                    "\n (SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name,idSample"
                    ", sampleType, QuanSample.level, QuanCompound.formula"
                    ", QuanCompound.mass AS 'exact mass', QuanResponse.mass"
                    ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error'"
                    ", timeCounts * 60000. / trigCounts as 'CountRate', trigCounts, QuanResponse.amount, QuanCompound.description, dataSource"
                    " FROM QuanSample, QuanResponse, QuanCompound"
                    " WHERE QuanSample.id = idSample"
                    " AND QuanResponse.idCmpd = QuanCompound.uuid AND isISTD=0) t1"
                    "\n LEFT JOIN"
                    "\n(SELECT idSample, timeCounts * 60000. / trigCounts as 'CountRate',QuanResponse.formula,QuanResponse.mass"
                    " FROM QuanResponse,QuanCompound"
                    " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) t2"
                    "\n ON t1.idSample=t2.idSample ORDER BY t1.idSample") );
    } else {
        setSQL( QString( "SELECT * FROM %1" ).arg( itemText ));
    }
}

void
QueryForm::on_history_currentIndexChanged( const QString& itemText )
{
    if ( auto combo = findChild< QComboBox * >( "history" ) )
        setSQL( itemText );
}

bool
QueryForm::eventFilter( QObject * object, QEvent * event )
{
    auto textEdit = qobject_cast<QPlainTextEdit *>( object );

    if ( textEdit && event->type() == QEvent::KeyPress ) {
        if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event) ) {
            if ( keyEvent->key() == ';' ) {
                semiColonCaptured_ = true;
            } else if ( keyEvent->key() == Qt::Key_Return && semiColonCaptured_ ) {
                emit triggerQuery( textEdit->toPlainText() );
            } else {
                semiColonCaptured_ = false;
            }
        }
    }
    return QWidget::eventFilter( object, event );
}

void
QueryForm::setCompleter( QCompleter *completer )
{
    if ( auto textEditor = findChild< SqlEdit * >() ) {
        textEditor->setCompleter( completer );
    }
}

QCompleter *
QueryForm::completer() const
{
    if ( auto textEditor = findChild< SqlEdit * >() )
        return textEditor->completer();
    return nullptr;
}
