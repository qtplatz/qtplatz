// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

namespace adportable {

    // wrapper for acqris u5303a and ap240 waveforms, which takes variable value_types
    // depend on number of samples to be averaged.
    
    template< typename value_type, typename waveform_type >
    class waveform_wrapper {

        const waveform_type& t_;

    public:
        waveform_wrapper( const waveform_type& t ) : t_( t ) {
        }

        inline const value_type * begin() const {
            return t_.template begin<value_type>();
        }
        
        inline const value_type * end() const {
            return t_.template end<value_type>();
        }

        inline size_t size() const {
            return t_.size();
        }
    };

}

