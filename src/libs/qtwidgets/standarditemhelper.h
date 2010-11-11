// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

class QStandardItem;
class QVariant;

namespace qtwidgets {

    class StandardItemHelper {
    public:
        StandardItemHelper();
        static QStandardItem * appendRow( QStandardItem * parent, const char * label, bool editable = false );
        static QStandardItem * appendRow( QStandardItem * parent, const char * label, const QVariant& );
        
    };

}

