// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

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
