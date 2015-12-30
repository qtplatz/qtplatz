/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once;

#include <adcontrols/datareader.hpp>

namespace acqrsinterpreter {

    class DataReader;

    class DataReader_index : public adcontrols::DataReader_index {
        const DataReader& reader_;
    public:
        virtual ~DataReader_index();
        DataReader_index( const DataReader& );

        int64_t pos() const override;
        int64_t elapsed_time() const override;
        double time_since_inject() const override;
        virtual void operator ++() override;
        virtual bool operator == ( adcontrols::DataReader_index& t ) const override;
        virtual bool operator != ( adcontrols::DataReader_index& t ) const override;
    };

    class DataReader_iterator : public adcontrols::DataReader_iterator<> {
        DataReader_iterator() = delete;
    public:
        virtual ~DataReader_iterator();
        DataReader_iterator( std::unique_ptr< DataReader_index >& );
    };
    
}


