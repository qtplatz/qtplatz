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

#include <utils/fancymainwindow.h>
#include "constants.hpp"
#include <array>
#include <functional>
#include <memory>
#include <adwidgets/mspeaktable.hpp> // callback type

namespace adportable { class Configuration; }
namespace adcontrols { class datafile; class ProcessMethod; class MSAssignedMasses; class MSPeaks; class MassSpectrum; }
namespace adprot { class digestedPeptides; }
namespace portfolio { class Folium; }
namespace Core { class IMode; }
namespace Utils { class StyledBar; }
class QString;
class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QStackedWidget;
class QLineEdit;
class QComboBox;

namespace adextension { class iController; class iSequenceImpl; }

namespace lipidid {

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        ~MainWindow();
        explicit MainWindow(QWidget *parent = 0);

        static MainWindow * instance();
        QWidget * createContents( Core::IMode * );

        void activateLayout();
        void setSimpleDockWidgetArrangement();

        void OnInitialUpdate();
        void OnFinalClose();

    private:

    public slots:
        void hideDock( bool );

    private slots:

    private:
        class impl;
        std::unique_ptr< impl > impl_;

    signals:
    };

}
