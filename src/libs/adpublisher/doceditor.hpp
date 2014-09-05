/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DOCEDITOR_HPP
#define DOCEDITOR_HPP

#include <QMainWindow>
#include <memory>
#include <array>
#include "adpublisher_global.hpp"

namespace pugi { class xml_document; }

class QAction;
class QComboBox;
class QCompleter;
class QToolBar;
class QFontComboBox;
class QTextCharFormat;
class QPrinter;
class QAbstractItemModel;
class QStackedWidget;

namespace adpublisher {

    class document;
    class docTree;
    class docEdit;
    class docBrowser;

    class ADPUBLISHERSHARED_EXPORT docEditor : public QMainWindow {
        Q_OBJECT
        docEditor( const docEditor& ) = delete;
    public:
        ~docEditor();
        explicit docEditor(QWidget *parent = 0);

        std::shared_ptr< adpublisher::document > document();
        void setDocument( std::shared_ptr< adpublisher::document >& );
        void setOutput( const QString& );

        void setupEditActions( QMenu* );
        void setupTextActions( QMenu* );

        void onInitialUpdate();
        
        enum idAction {
            idActionSave
            , idActionTextBold
            , idActionTextUnderline
            , idActionTextItalic
            , idActionTextColor
            , idActionAlignLeft
            , idActionAlignCenter
            , idActionAlignRight
            , idActionAlignJustify
            , idActionUndo
            , idActionRedo
            , idActionCut
            , idActionCopy
            , idActionPaste
            , nIdActions
        };
        void setAction( idAction, QAction * );
        QString currentStylesheet() const;

    private:
        std::shared_ptr< adpublisher::document > doc_;
        std::unique_ptr< docTree > tree_;
        std::unique_ptr< docEdit > text_;
        std::unique_ptr< docBrowser > browser_;
        std::array< QAction *, nIdActions > actions_;

        QComboBox *comboStyle;
        QFontComboBox *comboFont;
        QComboBox *comboSize;
        QStackedWidget * stacked_;

        QToolBar *tb;
        QString fileName;
        QCompleter *completer;

        bool load(const QString &f);
        bool maybeSave();
        void setCurrentFileName(const QString &fileName);
        void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
        void fontChanged(const QFont &f);
        void colorChanged(const QColor &c);
        void alignmentChanged(Qt::Alignment a);
        QAbstractItemModel * modelFromFile(const QString& fileName);

    signals:

    public slots:

        void fileNew();
        void fileOpen();
        bool fileSave();
        bool fileSaveAs();
        void filePrint();
        void filePrintPreview();
        void filePrintPdf();
    private slots:

        void textBold();
        void textUnderline();
        void textItalic();
        void textFamily(const QString &f);
        void textSize(const QString &p);
        void textStyle(int styleIndex);
        void textColor();
        void textAlign(QAction *a);
        void currentCharFormatChanged(const QTextCharFormat &format);
        void cursorPositionChanged();
        void currentPageChanged(int);

        void clipboardDataChanged();
        void about();
        void printPreview(QPrinter *);
    };

}

#endif // DOCEDITOR_HPP
