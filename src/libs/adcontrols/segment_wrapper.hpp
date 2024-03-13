// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include <cstddef>

namespace adcontrols {

    class MassSpectrum;

    template< typename T > class segment_iterator {
        size_t pos_;
        T& t_;
    public:
        segment_iterator( T& ms, size_t pos ) : pos_( pos ), t_( ms ) {}
        bool operator != ( const segment_iterator& rhs ) const {
			return pos_ != rhs.pos_;
		}
        const segment_iterator& operator ++ () { ++pos_; return *this; }
        operator T* () const { return pos_ == 0 ? &t_ : &t_.getSegment( pos_ - 1 ); }
    };

	template< typename T = MassSpectrum > class segment_wrapper {
		T& t_;
    public:
		typedef T value_type;
        typedef segment_iterator<T> iterator;
        typedef const segment_iterator<T> const_iterator;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		segment_wrapper( T& ms ) : t_( ms ) {}
        inline iterator begin()             { return segment_iterator<T>(t_, 0); }
        inline const_iterator begin() const { return segment_iterator<T>(t_, 0); }
        inline iterator end()               { return segment_iterator<T>(t_, t_.numSegments() + 1); }
        inline const_iterator end() const   { return segment_iterator<T>(t_, t_.numSegments() + 1); }
		inline reference operator [] ( size_t idx )             { return idx == 0 ? t_ : t_.getSegment( idx - 1 ); }
		inline const_reference operator [] ( size_t idx ) const { return idx == 0 ? t_ : t_.getSegment( idx - 1 ); }
		inline size_type size() const { return t_.numSegments() + 1; }
		inline size_type max_size() const { return t_.numSegments() + 1; }
    };

}
