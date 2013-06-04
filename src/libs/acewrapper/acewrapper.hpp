// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef ACEWRAPPER_H
#define ACEWRAPPER_H

template<class T, class X> class ACE_Singleton;
class ACE_Recursive_Thread_Mutex;

namespace acewrapper {

  class instance_manager {
  private:
      ~instance_manager();
      instance_manager();
      void initialize_i();
      void finalize_i();
  public:
      static void initialize();
      static void finalize();
      friend class ACE_Singleton<instance_manager, ACE_Recursive_Thread_Mutex>;
  };
  typedef ACE_Singleton<instance_manager, ACE_Recursive_Thread_Mutex> instance_manager_t;
}

#endif // ACEWRAPPER_H
