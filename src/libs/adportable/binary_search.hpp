// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace adportable {

  template<class _FwdIt, class _Ty>
  inline _FwdIt lower_bound(_FwdIt _First, _FwdIt _Last, const _Ty& _Val) {
    size_t _Count = _Last - _First + 1;
    while ( _Count > 0 ) {
	  size_t _Count2 = _Count / 2;
	  _FwdIt _Mid = _First;
	  std::advance(_Mid, _Count2);
	  if ( *_Mid < _Val )
		_First = ++_Mid, _Count -= _Count2 + 1;
	  else
		_Count = _Count2;
    }
    return _First;
  }
  
}
