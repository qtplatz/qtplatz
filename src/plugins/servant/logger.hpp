/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <adextension/ilogger.hpp>
#include <chrono>

namespace servant {

    class Logger : public adextension::iLogger {
        Q_OBJECT
    public:
        ~Logger();
        explicit Logger(QObject *parent = 0);

        void appendLog( const std::string&, bool ) override;

        void operator()( int, const std::string&, const std::string& file, int line, const std::chrono::system_clock::time_point& );

    signals:
        void onLogging( const QString&, bool );

    public slots:

    };

}

#endif // LOGGER_HPP
