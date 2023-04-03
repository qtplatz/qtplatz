/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "playercontrols.hpp"
#include <qtwrapper/make_widget.hpp>
#include <adportable/debug.hpp>
#include <QAudio>
#include <QBoxLayout>
#include <QComboBox>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyle>
#include <QTime>
#include <QToolButton>
#include <QLineEdit>
#include <ratio>

using namespace adwidgets;

PlayerControls::PlayerControls( QWidget *parent )
    : QWidget( parent )
    , playerState( QMediaPlayer::StoppedState )
    , playerMuted( false )
    , playButton( 0 )
    , stopButton( 0 )
    , nextButton( 0 )
    , previousButton( 0 )
    , rateBox( 0 )
    , frameSlider( 0 )
    , sliderScaleFactor_( 10 )
{
    playButton = new QToolButton(this);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(playButton, SIGNAL(clicked()), this, SLOT(playClicked()));

    stopButton = new QToolButton(this);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setEnabled(false);

    connect(stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));

    nextButton = new QToolButton(this);
    nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    connect(nextButton, SIGNAL(clicked()), this, SIGNAL(next()));

    previousButton = new QToolButton(this);
    previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    connect(previousButton, SIGNAL(clicked()), this, SIGNAL(previous()));

    frameSlider = new QSlider(Qt::Horizontal, this);
    frameSlider->setRange(0, 100);

    connect( frameSlider, &QSlider::valueChanged, this, [&]( int value ){ emit changeFrame( value ); });

    rateBox = new QComboBox(this);
    rateBox->addItem("0.5x", QVariant(0.5));
    rateBox->addItem("1.0x", QVariant(1.0));
    rateBox->addItem("2.0x", QVariant(2.0));
    rateBox->setCurrentIndex(1);

    connect(rateBox, SIGNAL(activated(int)), SLOT(updateRate()));

    QBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins( {} );
    layout->addWidget(stopButton);
    layout->addWidget(previousButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);

    layout->addWidget(frameSlider);
    layout->addWidget(rateBox);

    if ( auto edit = qtwrapper::make_widget< QLineEdit >( "Time" ) ) {
        edit->setReadOnly( true );
        edit->setText( QTime( 0, 0 ).toString( "hh:mm:ss" ) );
        layout->addWidget( edit );
    }

    if ( auto edit = new QLineEdit ) {
        edit->setObjectName( "Name" );
        edit->setReadOnly( true );
        layout->addWidget( edit );
    }

    layout->setStretchFactor( frameSlider, 10 );

    setLayout(layout);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QMediaPlayer::State
#else
QMediaPlayer::PlaybackState
#endif
PlayerControls::state() const
{
    return playerState;
}

void
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
PlayerControls::setState( QMediaPlayer::State state )
#else
PlayerControls::setState( QMediaPlayer::PlaybackState state )
#endif
{
    if (state != playerState) {
        playerState = state;

        switch (state) {
        case QMediaPlayer::StoppedState:
            stopButton->setEnabled(false);
            playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        case QMediaPlayer::PlayingState:
            stopButton->setEnabled(true);
            playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
            stopButton->setEnabled(true);
            playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        }
    }
}

void
PlayerControls::setPos( double aviRatio )
{
    QSignalBlocker block( frameSlider );
    frameSlider->setValue( int( aviRatio * 100 ) );
}

void
PlayerControls::playClicked()
{
    switch (playerState) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        emit play();
        break;
    case QMediaPlayer::PlayingState:
        emit pause();
        break;
    }
}

qreal
PlayerControls::playbackRate() const
{
    return rateBox->itemData(rateBox->currentIndex()).toDouble();
}

void
PlayerControls::setPlaybackRate(float rate)
{
    for (int i = 0; i < rateBox->count(); ++i) {
        if (qFuzzyCompare(rate, float(rateBox->itemData(i).toDouble()))) {
            rateBox->setCurrentIndex(i);
            return;
        }
    }

    rateBox->addItem(QString("%1x").arg(rate), QVariant(rate));
    rateBox->setCurrentIndex(rateBox->count() - 1);
}

void
PlayerControls::updateRate()
{
    emit changeRate(playbackRate());
}

void
PlayerControls::setNumberOfFrames( size_t value )
{
}

void
PlayerControls::setCurrentFrame( size_t value )
{
}

void
PlayerControls::setTime( double sec )
{
    if ( auto edit = findChild< QLineEdit * >( "Time" ) ) {
        QTime t = QTime::fromMSecsSinceStartOfDay( int( sec * std::milli::den ) );
        edit->setText( t.toString( "hh:mm:ss" ) );

        frameSlider->setSliderPosition( int( sec * sliderScaleFactor_ ) );
    }
}

void
PlayerControls::setDuration( double seconds )
{
    frameSlider->setRange( 0, int( seconds * sliderScaleFactor_ ) ); // 100ms resolution
}

void
PlayerControls::setName( const QString& name )
{
    if ( auto edit = findChild< QLineEdit * >( "Name" ) ) {
        edit->setText( name );
    }
}
