/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QDialog>
#include <memory>
#include <map>
#include <set>
#include <boost/optional.hpp>

class QMenu;

namespace admtcontrols { class ScanLaw; }
namespace adcontrols { class MSPeaks; class MassSpectrometer; }
namespace boost { namespace uuids { struct uuid; } }

namespace dataproc {

    class lapDeconvDlg : public QDialog {

        Q_OBJECT

    public:
        explicit lapDeconvDlg(QWidget *parent = 0);
        ~lapDeconvDlg();
        void setData( const std::vector< std::tuple< double, int, double > >& );
        void setList( const std::vector< std::tuple< double, int > >& );
        boost::optional< std::tuple< double, int, double > > getSelection() const;

    public slots:

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
