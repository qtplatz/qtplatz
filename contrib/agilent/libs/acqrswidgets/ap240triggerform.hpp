/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef AP240TRIGGERFORM_HPP
#define AP240TRIGGERFORM_HPP

#include <QWidget>

namespace Ui {
class ap240TriggerForm;
}

namespace acqrscontrols { namespace ap240 { class method; } }

class ap240TriggerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ap240TriggerForm(QWidget *parent = 0);
    ~ap240TriggerForm();

    enum idItem { idTrigClass, idTrigPattern, idTrigCoupling, idTrigSlope, idTrigLevel1, idTrigLevel2 };

    void set( const acqrscontrols::ap240::method& );
    void get( acqrscontrols::ap240::method& ) const;

signals:
    void valueChanged( idItem, const QVariant& );

private:
    Ui::ap240TriggerForm *ui;
};

#endif // AP240TRIGGERFORM_HPP
