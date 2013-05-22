// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
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
