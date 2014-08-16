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

#include "doceditor.hpp"
#include "doctree.hpp"
#include "doctext.hpp"
#include "document.hpp"
#include <xmlparser/pugixml.hpp>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QSplitter>
#include <QTextCodec>
#include <QTextEdit>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>

#ifdef Q_OS_MAC
const QString qrcpath = ":/adpublisher/images/mac";
#else
const QString qrcpath = ":/adpublisher/images/win";
#endif

namespace adpublisher {
    namespace detail {
    }
}

using namespace adpublisher;

docEditor::~docEditor()
{
}

docEditor::docEditor( QWidget *parent ) : QMainWindow( parent )
                                      , doc_( std::make_shared< adpublisher::document >() )
                                      , tree_( new docTree )
                                      , text_( new docText )
{
    setToolButtonStyle( Qt::ToolButtonFollowStyle );
    //setupFileActions();
    //setupEditActions();
    //setupTextActions();

    auto widget = new QWidget;
    setCentralWidget( widget );
    auto layout = new QVBoxLayout(widget);

    layout->setMargin(0);
    layout->setSpacing(0);

    auto splitter = new QSplitter;
    splitter->setHandleWidth( 1 );
    splitter->setOrientation( Qt::Horizontal );
    splitter->addWidget( tree_.get() );
    splitter->addWidget( text_.get() );
    splitter->setStretchFactor( 1, 3 );

    layout->addWidget( splitter );

    tree_->setDocument( doc_ );
    text_->setDocument( doc_ );
}

void
docEditor::setDocument( std::shared_ptr< adpublisher::document >& t )
{
    doc_ = t;
}

std::shared_ptr< adpublisher::document > 
docEditor::document()
{
    return doc_;
}

void
docEditor::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(tb);

//    QMenu *menu = new QMenu(tr("&File"), this);
//    menuBar()->addMenu(menu);

    QAction *a;

    QIcon newIcon = QIcon::fromTheme("document-new", QIcon(qrcpath + "/filenew.png"));
    a = new QAction( newIcon, tr("&New"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);
    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
    tb->addAction(a);
    //menu->addAction( a );

    a = new QAction(QIcon::fromTheme("document-open", QIcon(qrcpath + "/fileopen.png")),
                    tr("&Open..."), this);
    a->setShortcut(QKeySequence::Open);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    //menu->addAction( a );

    //menu->addSeparator();

    actionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(qrcpath + "/filesave.png")),
                                 tr("&Save"), this);
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);
    //menu->addAction( a );

    a = new QAction(tr("Save &As..."), this);
    a->setPriority(QAction::LowPriority);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    //menu->addAction(a);
    //menu->addSeparator();

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(qrcpath + "/fileprint.png")),
                    tr("&Print..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);
    //menu->addAction( a );

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(qrcpath + "/fileprint.png")),
                    tr("Print Preview..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
    //menu->addAction( a );

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(qrcpath + "/exportpdf.png")),
                    tr("&Export PDF..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
    tb->addAction(a);
    //menu->addAction( a );

    //menu->addSeparator();
#endif

    a = new QAction(tr("&Quit"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(a, SIGNAL(triggered()), this, SLOT(close()));
    //menu->addAction( a );
}

void
docEditor::setupEditActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(tb);
    QMenu *menu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(menu);

    QAction *a;
    a = actionUndo = new QAction(QIcon::fromTheme("edit-undo", QIcon(qrcpath + "/editundo.png")),
                                              tr("&Undo"), this);
    a->setShortcut(QKeySequence::Undo);
    tb->addAction(a);
    menu->addAction(a);
    a = actionRedo = new QAction(QIcon::fromTheme("edit-redo", QIcon(qrcpath + "/editredo.png")),
                                              tr("&Redo"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Redo);
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = actionCut = new QAction(QIcon::fromTheme("edit-cut", QIcon(qrcpath + "/editcut.png")),
                                             tr("Cu&t"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Cut);
    tb->addAction(a);
    menu->addAction(a);
    a = actionCopy = new QAction(QIcon::fromTheme("edit-copy", QIcon(qrcpath + "/editcopy.png")),
                                 tr("&Copy"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Copy);
    tb->addAction(a);
    menu->addAction(a);
    a = actionPaste = new QAction(QIcon::fromTheme("edit-paste", QIcon(qrcpath + "/editpaste.png")),
                                  tr("&Paste"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Paste);
    tb->addAction(a);
    menu->addAction(a);
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void
docEditor::setupTextActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Format Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("F&ormat"), this);
    menuBar()->addMenu(menu);

    actionTextBold = new QAction(QIcon::fromTheme("format-text-bold", QIcon(qrcpath + "/textbold.png")),
                                 tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);
    menu->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic",
                                                    QIcon(qrcpath + "/textitalic.png")),
                                   tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);
    menu->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline",
                                                       QIcon(qrcpath + "/textunder.png")),
                                      tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);
    menu->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

    // Make sure the alignLeft  is always left of the alignRight
    if (QApplication::isLeftToRight()) {
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left",
                                                       QIcon(qrcpath + "/textleft.png")),
                                      tr("&Left"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center",
                                                         QIcon(qrcpath + "/textcenter.png")),
                                        tr("C&enter"), grp);
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right",
                                                        QIcon(qrcpath + "/textright.png")),
                                       tr("&Right"), grp);
    } else {
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right",
                                                        QIcon(qrcpath + "/textright.png")),
                                       tr("&Right"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center",
                                                         QIcon(qrcpath + "/textcenter.png")),
                                        tr("C&enter"), grp);
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left",
                                                       QIcon(qrcpath + "/textleft.png")),
                                      tr("&Left"), grp);
    }
    actionAlignJustify = new QAction(QIcon::fromTheme("format-justify-fill",
                                                      QIcon(qrcpath + "/textjustify.png")),
                                     tr("&Justify"), grp);

    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    tb->addActions(grp->actions());
    menu->addActions(grp->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    tb->addAction(actionTextColor);
    menu->addAction(actionTextColor);

    tb = new QToolBar(this);
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    tb->setWindowTitle(tr("Format Actions"));
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    connect(comboStyle, SIGNAL(activated(int)), this, SLOT(textStyle(int)));

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, SIGNAL(activated(QString)), this, SLOT(textFamily(QString)));

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(textSize(QString)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                                                                   .pointSize())));
}
