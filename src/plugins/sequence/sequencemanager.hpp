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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H

#include <QObject>
#include <boost/smart_ptr.hpp>
#include <vector>

class QDockWidget;
class QMainWindow;

namespace Utils {
    class FancyMainWindow;
}

namespace adportable {
    class Configuration;
}

namespace sequence {
  namespace internal {

    class SequenceManager : public QObject {
      Q_OBJECT
    public:
      ~SequenceManager();
      explicit SequenceManager(QObject *parent = 0);

      QMainWindow * mainWindow() const;
      void init( const std::wstring& apppath, adportable::Configuration& acquire, adportable::Configuration& dataproc );
      void setSimpleDockWidgetArrangement();

      void OnInitialUpdate();
      void OnFinalClose();

    signals:

    public slots:

    private:
      Utils::FancyMainWindow * mainWindow_;
      std::vector< QDockWidget * > dockWidgetVec_;
    };

  }
}

#endif // SEQUENCEMANAGER_H
