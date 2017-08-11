/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "recordercontrols.hpp"
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QLineEdit>
#include <QSlider>
#include <QStyle>
#include <QTime>
#include <QToolButton>
#include <QComboBox>
#include <QAudio>
#include <chrono>

RecorderControls::RecorderControls(QWidget *parent)
    : QWidget(parent)
    , playerState(QMediaPlayer::StoppedState)
    , playerMuted(false)
    , playButton(0)
    , stopButton(0)
    , nextButton(0)
    , previousButton(0)
    , muteButton(0)
      //, volumeSlider(0)
    , rateBox(0)
    , frameSlider(0)
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

    muteButton = new QToolButton(this);
    muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    connect(muteButton, SIGNAL(clicked()), this, SLOT(muteClicked()));

    // volumeSlider = new QSlider(Qt::Horizontal, this);
    // volumeSlider->setRange(0, 100);
    //connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(onVolumeSliderValueChanged()));

    frameSlider = new QSlider(Qt::Horizontal, this);
    frameSlider->setRange(0, 100);

    connect( frameSlider, &QSlider::valueChanged, this, [&]( int value ){ emit changeFrame( value ); });
    
    // rateBox = new QComboBox(this);
    // rateBox->addItem("0.5x", QVariant(0.5));
    // rateBox->addItem("1.0x", QVariant(1.0));
    // rateBox->addItem("2.0x", QVariant(2.0));
    // rateBox->setCurrentIndex(1);

    connect(rateBox, SIGNAL(activated(int)), SLOT(updateRate()));

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(stopButton);
    layout->addWidget(previousButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
    layout->addWidget(muteButton);
    //layout->addWidget(volumeSlider);
    layout->addWidget(frameSlider);
    //layout->addWidget(rateBox);
    if ( auto edit = new QLineEdit ) {
        edit->setReadOnly( true );
        edit->setText( QTime( 0, 0 ).toString( "hh:mm:ss" ) );
        layout->addWidget( edit );
    }
    
    layout->setStretchFactor( frameSlider, 10 );
    setLayout(layout);
}

QMediaPlayer::State
RecorderControls::state() const
{
    return playerState;
}

void
RecorderControls::setState(QMediaPlayer::State state)
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
RecorderControls::playClicked()
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

void
RecorderControls::muteClicked()
{
    emit changeMuting(!playerMuted);
}

void
RecorderControls::setTime( double msec )
{
    if ( auto edit = findChild< QLineEdit * >() ) {
        QTime t = QTime::fromMSecsSinceStartOfDay( int( msec ) );
        edit->setText( t.toString( "hh:mm:ss" ) );
    }
}
