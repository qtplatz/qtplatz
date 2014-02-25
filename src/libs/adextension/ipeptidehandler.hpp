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

#ifndef IPEPTIDEHANDLER_HPP
#define IPEPTIDEHANDLER_HPP

#pragma once

#include <QObject>
#include "adextension_global.hpp"

namespace adprot { class digestedPeptides; }

namespace adextension {

    class ADEXTENSIONSHARED_EXPORT iPeptideHandler : public QObject {
        Q_OBJECT
    public:
        explicit iPeptideHandler(QObject *parent = 0);
        virtual void onProteinSelected( const adprot::digestedPeptides& ) const = 0;
    signals:

    public slots:

    };

}

#endif // IPEPTIDEHANDLER_HPP
