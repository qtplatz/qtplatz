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

#pragma once

#include "adwidgets_global.hpp"
#include <QtGlobal>
#include <QMediaPlayer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractSlider;
class QComboBox;
QT_END_NAMESPACE

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT PlayerControls : public QWidget {
        Q_OBJECT

    public:
        PlayerControls( QWidget *parent = 0 );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QMediaPlayer::State state() const;
#else
        QMediaPlayer::PlaybackState state() const;
#endif
        qreal playbackRate() const;

    public slots:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        void setState(QMediaPlayer::State state);
#else
        void setState(QMediaPlayer::PlaybackState state);
#endif

        void setPlaybackRate(float rate);
        void setNumberOfFrames( size_t );
        void setCurrentFrame( size_t );
        void setPos( double /* avi ratio 0..1.0 */ );
        void setTime( double seconds );
        void setName( const QString& );
        void setDuration( double seconds );

    signals:
        void play();
        void pause();
        void stop();
        void next();
        void previous();
        void changeRate( double rate );
        void changeFrame( int );

    private slots:
        void playClicked();
        void updateRate();
    private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QMediaPlayer::State playerState;
#else
        QMediaPlayer::PlaybackState playerState;
#endif
        bool playerMuted;
        QAbstractButton *playButton;
        QAbstractButton *stopButton;
        QAbstractButton *nextButton;
        QAbstractButton *previousButton;
        QAbstractSlider *frameSlider;
        QComboBox *rateBox;
        size_t sliderScaleFactor_;
    };

}
