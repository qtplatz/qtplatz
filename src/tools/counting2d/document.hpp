/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QObject>
#include <opencv2/ml.hpp>
#include <vector>
#include <memory>

namespace adprocessor { class dataprocessor; }
namespace adcontrols { class MappedSpectra; class MappedDataFrame; }

namespace counting2d {

    class document : public QObject {

        Q_OBJECT
        document( const document& ) = delete;
        document& operator = ( const document& ) = delete;
        document();
    public:
        static document * instance();

        typedef std::vector< std::shared_ptr< adcontrols::MappedDataFrame > >::const_iterator const_iterator;
        
        bool setDataprocessor( std::shared_ptr< adprocessor::dataprocessor > );
	
        bool fetch();

        inline void rewind() {
            cursor_ = mappedDataFrame_.begin();
        }

        inline const_iterator forward() {
            if ( cursor_ != mappedDataFrame_.end() )
                return ++cursor_;
            return mappedDataFrame_.end();
        }
        
        inline const_iterator begin() const {
            return mappedDataFrame_.begin();
        }
	
        inline const_iterator end() const {
            return mappedDataFrame_.end();
        }

        inline std::shared_ptr< const adcontrols::MappedDataFrame > back() const {
            if ( cursor_ != end() )
                return *cursor_;
            return nullptr;
        }

        void em( std::shared_ptr< const adcontrols::MappedDataFrame > df );
        void kmean( std::shared_ptr< const adcontrols::MappedDataFrame > df );
        void contours( std::shared_ptr< const adcontrols::MappedDataFrame > df );
        const cv::Mat& mat() const;
        
    signals:
        void dataChanged();
        
    private:
        std::shared_ptr< adprocessor::dataprocessor > processor_;
        std::vector< std::shared_ptr< adcontrols::MappedDataFrame > > mappedDataFrame_;
        const_iterator cursor_;
        cv::Ptr< cv::ml::EM > em_model_;
        cv::Mat mat_;
        void makeGrayScale( const adcontrols::MappedDataFrame& );
    };
    
}
