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

#ifndef DELEGATEHELPER_HPP
#define DELEGATEHELPER_HPP

class QPainter;
class QStyleOptionViewItem;
class QString;
class QModelIndex;
#include <QSize>
#include <QString>
#include "adwidgets_global.hpp"

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT DelegateHelper  {
    public:
        DelegateHelper();
        static void render_html2( QPainter * painter, const QStyleOptionViewItem& option, const QString& text );
        static void render_html( QPainter * painter, const QStyleOptionViewItem& option, const QString& text
                                 , const QString& css = QString() );
        static QSize html_size_hint( const QStyleOptionViewItem& option, const QModelIndex& index );
    };

}

#endif // DELEGATEHELPER_HPP
