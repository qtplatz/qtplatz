// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCEPLUGIN_H
#define SEQUENCEPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace adportable {
    class Configuration;
}

namespace sequence {
  namespace internal {

    class SequenceManager;

    class SequencePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT;
    public:
      ~SequencePlugin();
      explicit SequencePlugin();

      bool initialize(const QStringList& arguments, QString* error_message);
      void extensionsInitialized();
      void shutdown();

    signals:

    public slots:

    private:
      boost::shared_ptr<SequenceManager> manager_;
      QWidget * CreateSequenceWidget( const std::wstring&, const adportable::Configuration& );
    };
    //------
  }
}

#endif // SEQUENCEPLUGIN_H
