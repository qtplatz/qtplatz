/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QWidget>
#include <memory>

class QStandardItemModel;
class QTreeView;

namespace acqrscontrols {
    namespace ap240 { class method; }
    namespace aqdrv4 {
        class acqiris_method;
        struct trigger_method;
        struct vertical_method;
        struct horizontal_method;
    }
}

namespace acqrswidgets {
        
    class AcqirisWidget : public QWidget {
        
        Q_OBJECT
        
    public:
        explicit AcqirisWidget( QWidget *parent = 0 );
        ~AcqirisWidget();
        
        enum ConfigItem {
            horStartDelay, horWidth, horSamplingRate, horMode
            , verFullScale, verOffset, verCoupling, verBandWidth
            , trigClass, trigSource, trigCoupling, trigLevel1, trigLevel2
        };

        void onInitialUpdate();
        
        void setContents( std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > );
        void getContents( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > ) const;

        void setContents( std::shared_ptr< const acqrscontrols::ap240::method > );
        void getContents( std::shared_ptr< acqrscontrols::ap240::method > ) const;        

        bool setContents( const acqrscontrols::ap240::method& );
        bool getContents( acqrscontrols::ap240::method& ) const;
        
    private:
        class delegate;
        void setContents( const acqrscontrols::aqdrv4::trigger_method& );
        void setContents( const acqrscontrols::aqdrv4::horizontal_method& );
        void setContents( const acqrscontrols::aqdrv4::vertical_method&, int row );
        void getContents( acqrscontrols::aqdrv4::trigger_method& ) const;
        void getContents( acqrscontrols::aqdrv4::horizontal_method& ) const;
        void getContents( acqrscontrols::aqdrv4::vertical_method&, int row ) const;
        
    signals:
        void dataChanged( const AcqirisWidget *, int subType );
        void stateChanged( const QModelIndex&, bool );
                                                              
    private slots:
        
    private:
        std::unique_ptr< QStandardItemModel > model_;
        QTreeView * tree_;
        void initialUpdate( QStandardItemModel& );
    };

}
