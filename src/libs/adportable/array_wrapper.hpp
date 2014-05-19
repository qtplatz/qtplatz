// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

namespace adportable {

  template<class T> 
  class array_wrapper {
  private:
    T * pv_;
    const size_t size_;
  public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
	
  public:
    array_wrapper(T * pv, size_t size) : pv_(pv), size_(size) {}
	
	inline const T* get() const { return pv_; }
    inline iterator begin() { return pv_; }
    inline const_iterator begin() const { return pv_; }
    inline iterator end() { return pv_ + size_; }
    inline const_iterator end() const { return pv_ + size_; }
    inline reference operator[](size_t i) { return pv_[i]; }
    inline const_reference operator[](size_t i) const { return pv_[i]; }
	
    //
    inline size_type size() const { return size_; }
    inline size_type max_size() const { return size_; }
	
    //
    inline operator T* () { return pv_; }
  };
  
}
