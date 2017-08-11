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

#pragma once

#include <QMediaPlayer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractSlider;
class QComboBox;
QT_END_NAMESPACE

class RecorderControls : public QWidget {
    Q_OBJECT

public:
    RecorderControls( QWidget *parent = 0 );

    QMediaPlayer::State state() const;
    // int volume() const;
    // bool isMuted() const;
    // qreal playbackRate() const;
    void setTime( double );

public slots:
    void setState( QMediaPlayer::State state );
    // void setVolume(int volume);
    // void setMuted(bool muted);
    // void setPlaybackRate(float rate);
    // void setNumberOfFrames( size_t );
    //void setCurrentFrame( size_t );

signals:
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    //void changeVolume( int volume );
    void changeMuting( bool muting );
    void changeRate( qreal rate );
    void changeFrame( int );

private slots:
    void playClicked();
    void muteClicked();
    //void updateRate();
    //void onVolumeSliderValueChanged();

private:
    QMediaPlayer::State playerState;
    bool playerMuted;
    QAbstractButton *playButton;
    QAbstractButton *stopButton;
    QAbstractButton *nextButton;
    QAbstractButton *previousButton;
    QAbstractButton *muteButton;
    //QAbstractSlider *volumeSlider;
    QAbstractSlider *frameSlider;
    QComboBox *rateBox;
};


