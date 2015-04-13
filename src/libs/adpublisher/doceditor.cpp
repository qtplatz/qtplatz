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
#include "docbrowser.hpp"
#include "doctree.hpp"
#include "docedit.hpp"
#include "document.hpp"
#include "transformer.hpp"
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QCompleter>
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
#include <QStringListModel>
#include <QStackedWidget>
#include <QtConcurrent>
#include <QTextCodec>
#include <QTextEdit>
#include <QTextBrowser>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <boost/filesystem/path.hpp>

#ifdef Q_OS_WIN
const QString qrcpath = ":/adpublisher/images/win";
#else
const QString qrcpath = ":/adpublisher/images/mac";
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
                                        , text_( new docEdit )
                                        , browser_( new docBrowser )
                                        , comboStyle(0)
                                        , comboFont(0)
                                        , comboSize(0)
                                        , stacked_( new QStackedWidget )
                                        , tb(0)
{
    std::fill( actions_.begin(), actions_.end(), static_cast<QAction*>(0) );

    setToolButtonStyle( Qt::ToolButtonFollowStyle );

    stacked_->addWidget( text_.get() );
    stacked_->addWidget( browser_.get() );

    auto widget = new QWidget;
    setCentralWidget( widget );
    auto layout = new QVBoxLayout(widget);

    layout->setMargin(0);
    layout->setSpacing(0);

    auto splitter = new QSplitter;
    splitter->setHandleWidth( 1 );
    splitter->setOrientation( Qt::Horizontal );
    splitter->addWidget( tree_.get() );
    // splitter->addWidget( text_.get() );
    splitter->addWidget( stacked_ );
    connect( stacked_, &QStackedWidget::currentChanged, this, &docEditor::currentPageChanged );

    splitter->setStretchFactor( 1, 3 );

    layout->addWidget( splitter );

    tree_->setDocument( doc_ );
    text_->setDocument( doc_ );

    completer = new QCompleter( this );
    completer->setModel( modelFromFile( ":/adpublisher/wordlist.txt" ) );
    completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    completer->setWrapAround( false );
    text_->setCompleter( completer );

    connect(text_.get(), &docEdit::currentCharFormatChanged, this, &docEditor::currentCharFormatChanged );
    connect(text_.get(), &docEdit::cursorPositionChanged, this, &docEditor::cursorPositionChanged );
    stacked_->setCurrentIndex( 0 ); // Template as default

}

void
docEditor::setAction( idAction id, QAction * action )
{
    actions_[ id ] = action;
}

void
docEditor::onInitialUpdate()
{
    setCurrentFileName( QString() );

    fontChanged( text_->font() );
    colorChanged( text_->textColor() );
    alignmentChanged( text_->alignment() );

    connect(text_->document(), &QTextDocument::modificationChanged,  actions_[ idActionSave ], &QAction::setEnabled );
    connect(text_->document(), &QTextDocument::modificationChanged,  this, &docEditor::setWindowModified );
    connect(text_->document(), &QTextDocument::undoAvailable,  actions_[ idActionUndo ], &QAction::setEnabled );
    connect(text_->document(), &QTextDocument::redoAvailable,  actions_[ idActionRedo ], &QAction::setEnabled );
}

void
docEditor::setDocument( std::shared_ptr< adpublisher::document >& t )
{
    doc_ = t;
    tree_->setDocument( doc_ );
    text_->setDocument( doc_ );
    stacked_->setCurrentIndex(0);
    if ( auto w = findChild< QComboBox * >( "comboBrowser ") )
        w->setCurrentIndex(0);
#if 0
    if ( auto node = doc_->xml_document()->select_single_node( "/article|/book" ) ) {
        text_->setDocument( doc_ );
    }
    else if ( auto node = doc_->xml_document()->select_single_node( "/qtpaltz_document" ) ) { 
    }
#endif
}

void
docEditor::setOutput( const QString& output, const QString& method )
{
    (void)method;
    qtwrapper::waitCursor wait;
    browser_->setOutput( output );
    stacked_->setCurrentIndex( 1 );
}

void
docEditor::setOutput( const QUrl& url )
{
    qtwrapper::waitCursor wait;
    browser_->setOutput( url );
    stacked_->setCurrentIndex( 1 );
}

std::shared_ptr< adpublisher::document > 
docEditor::document()
{
    return doc_;
}

QString
docEditor::currentStylesheet() const
{
    if ( auto w = findChild< QComboBox * >( "stylesCombo" ) )
        return w->currentText();
    return QString();
}

void
docEditor::setupEditActions( QMenu * menu )
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(tb);

    QAction *a;
    a = new QAction(QIcon::fromTheme("edit-undo", QIcon(qrcpath + "/editundo.png")), tr("&Undo"), this);
    actions_[ idActionUndo ] = a;
    a->setShortcut(QKeySequence::Undo);
    tb->addAction(a);
    menu->addAction(a);
    a = actions_[ idActionRedo ] = new QAction(QIcon::fromTheme("edit-redo", QIcon(qrcpath + "/editredo.png")), tr("&Redo"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Redo);
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = actions_[ idActionCut ] = new QAction( QIcon::fromTheme( "edit-cut", QIcon( qrcpath + "/editcut.png" ) ), tr( "Cu&t" ), this );
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Cut);
    tb->addAction(a);
    menu->addAction(a);
    a = actions_[ idActionCopy ] = new QAction( QIcon::fromTheme( "edit-copy", QIcon( qrcpath + "/editcopy.png" ) ), tr( "&Copy" ), this );
    a->setPriority(QAction::LowPriority);
    a->setShortcut( QKeySequence::Copy );
    tb->addAction(a);
    menu->addAction(a);
    a = actions_[ idActionPaste ] = new QAction( QIcon::fromTheme( "edit-paste", QIcon( qrcpath + "/editpaste.png" ) ), tr( "&Paste" ), this );
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Paste);
    tb->addAction(a);
    menu->addAction(a);
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actions_[ idActionPaste ]->setEnabled( md->hasText() );
#endif
}

void
docEditor::setupTextActions( QMenu * menu )
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Format Actions"));
    addToolBar(tb);

    actions_[ idActionTextBold ] = new QAction( QIcon::fromTheme( "format-text-bold", QIcon( qrcpath + "/textbold.png" ) ), tr( "&Bold" ), this );
    actions_[ idActionTextBold ]->setShortcut( Qt::CTRL + Qt::Key_B );
    actions_[ idActionTextBold ]->setPriority( QAction::LowPriority );
    QFont bold;
    bold.setBold(true);
    actions_[ idActionTextBold ]->setFont( bold );
    connect( actions_[ idActionTextBold ], SIGNAL( triggered() ), this, SLOT( textBold() ) );
    tb->addAction( actions_[ idActionTextBold ]);
    menu->addAction( actions_[ idActionTextBold ] );
    actions_[ idActionTextBold ]->setCheckable( true );
    
    actions_[ idActionTextItalic ] = new QAction( QIcon::fromTheme( "format-text-italic", QIcon( qrcpath + "/textitalic.png" ) ), tr( "&Italic" ), this );
    actions_[ idActionTextItalic ]->setPriority( QAction::LowPriority );
    actions_[ idActionTextItalic ]->setShortcut( Qt::CTRL + Qt::Key_I );
    QFont italic;
    italic.setItalic(true);
    actions_[ idActionTextItalic ]->setFont( italic );
    connect( actions_[ idActionTextItalic ], SIGNAL( triggered() ), this, SLOT( textItalic() ) );
    tb->addAction( actions_[ idActionTextItalic ] );
    menu->addAction( actions_[ idActionTextItalic ] );
    actions_[ idActionTextItalic ]->setCheckable( true );

    actions_[ idActionTextUnderline ] = new QAction( QIcon::fromTheme( "format-text-underline", QIcon( qrcpath + "/textunder.png" ) ), tr( "&Underline" ), this );
    actions_[ idActionTextUnderline ]->setShortcut( Qt::CTRL + Qt::Key_U );
    actions_[ idActionTextUnderline ]->setPriority( QAction::LowPriority );
    QFont underline;
    underline.setUnderline(true);
    actions_[ idActionTextUnderline ]->setFont( underline );
    connect( actions_[ idActionTextUnderline ], SIGNAL( triggered() ), this, SLOT( textUnderline() ) );
    tb->addAction(actions_[ idActionTextUnderline ] );
    menu->addAction(actions_[ idActionTextUnderline ] );
    actions_[ idActionTextUnderline ]->setCheckable( true );

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup( this );
    connect( grp, SIGNAL( triggered( QAction* ) ), this, SLOT( textAlign( QAction* ) ) );

    // Make sure the alignLeft  is always left of the alignRight
    if (QApplication::isLeftToRight()) {
        actions_[ idActionAlignLeft ] = new QAction( QIcon::fromTheme( "format-justify-left", QIcon( qrcpath + "/textleft.png" ) ), tr( "&Left" ), grp );
        actions_[ idActionAlignCenter ] = new QAction( QIcon::fromTheme( "format-justify-center", QIcon( qrcpath + "/textcenter.png" ) ), tr( "C&enter" ), grp );
        actions_[ idActionAlignRight ] = new QAction( QIcon::fromTheme( "format-justify-right", QIcon( qrcpath + "/textright.png" ) ), tr( "&Right" ), grp );
    } else {
        actions_[ idActionAlignRight ] = new QAction( QIcon::fromTheme( "format-justify-right", QIcon( qrcpath + "/textright.png" ) ), tr( "&Right" ), grp );
        actions_[ idActionAlignCenter ] = new QAction( QIcon::fromTheme( "format-justify-center", QIcon( qrcpath + "/textcenter.png" ) ), tr( "C&enter" ), grp );
        actions_[ idActionAlignLeft ] = new QAction( QIcon::fromTheme( "format-justify-left", QIcon( qrcpath + "/textleft.png" ) ), tr( "&Left" ), grp );
    }
    actions_[ idActionAlignJustify ] = new QAction( QIcon::fromTheme( "format-justify-fill", QIcon( qrcpath + "/textjustify.png" ) ), tr( "&Justify" ), grp );
    
    actions_[ idActionAlignLeft ]->setShortcut( Qt::CTRL + Qt::Key_L );
    actions_[ idActionAlignLeft ]->setCheckable( true );
    actions_[ idActionAlignLeft ]->setPriority( QAction::LowPriority );
    actions_[ idActionAlignCenter ]->setShortcut( Qt::CTRL + Qt::Key_E );
    actions_[ idActionAlignCenter ]->setCheckable( true );
    actions_[ idActionAlignCenter ]->setPriority( QAction::LowPriority );
    actions_[ idActionAlignRight ]->setShortcut( Qt::CTRL + Qt::Key_R );
    actions_[ idActionAlignRight ]->setCheckable( true );
    actions_[ idActionAlignRight ]->setPriority( QAction::LowPriority );
    actions_[ idActionAlignJustify ]->setShortcut( Qt::CTRL + Qt::Key_J );
    actions_[ idActionAlignJustify ]->setCheckable( true );
    actions_[ idActionAlignJustify ]->setPriority( QAction::LowPriority );

    tb->addActions(grp->actions());
    menu->addActions(grp->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actions_[ idActionTextColor ] = new QAction(pix, tr("&Color..."), this);
    connect( actions_[ idActionTextColor ], SIGNAL( triggered() ), this, SLOT( textColor() ) );
    tb->addAction( actions_[ idActionTextColor ] );
    menu->addAction( actions_[ idActionTextColor ] );

    tb = new QToolBar(this);
    tb->setAllowedAreas( Qt::TopToolBarArea | Qt::BottomToolBarArea );
    tb->setWindowTitle( tr( "Format Actions" ) );
    addToolBarBreak( Qt::TopToolBarArea );
    addToolBar( tb );

    if ( auto xslt = new QComboBox( tb ) ) {
        xslt->setObjectName( "stylesCombo" );
        QStringList list;
        transformer::populateStylesheets( list );
        xslt->addItems( list );
        tb->addWidget( xslt );
    }
    
    if ( auto cbox = new QComboBox( tb ) ) {
        cbox->setObjectName( "browserCombo" );
        cbox->addItems( QStringList() << tr("Template") << tr("Browser") );
        tb->addWidget( cbox );
        connect( cbox, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, [&](int idx){ stacked_->setCurrentIndex( idx ); });
    }

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

////////////////////////////////////////

bool
docEditor::load( const QString &f )
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        text_->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        text_->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

bool
docEditor::maybeSave()
{
    if (!text_->document()->isModified())
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void
docEditor::setCurrentFileName( const QString &fileName )
{
    this->fileName = fileName;
    text_->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Rich Text")));
    setWindowModified(false);
}

void
docEditor::fileNew()
{
    if (maybeSave()) {
        text_->clear();
        setCurrentFileName(QString());
    }
}

void
docEditor::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."),
                                              QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
    if (!fn.isEmpty())
        load(fn);
}

bool
docEditor::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    boost::filesystem::path path( fileName.toStdWString() );
    if ( path.extension() == ".xml" ) {
        return doc_->save_file( path.string().c_str() );
    }
    else {

        QTextDocumentWriter writer( fileName );
        bool success = writer.write( text_->document() );
        if ( success )
            text_->document()->setModified( false );
        return success;
    }
}

bool
docEditor::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName( this
                                               , tr( "Save as..." )
                                               , QString()
                                               , tr( "Template files (*.xml);;ODF files (*.odt);;HTML-Files (*.htm *.html);;All Files (*)" ) );
    if (fn.isEmpty())
        return false;
    /*
    if (!(fn.endsWith(".odt", Qt::CaseInsensitive)
          || fn.endsWith(".htm", Qt::CaseInsensitive)
          || fn.endsWith(".html", Qt::CaseInsensitive))) {
        fn += ".odt"; // default
    }
*/
    setCurrentFileName(fn);
    return fileSave();
}

void docEditor::filePrint()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (text_->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        text_->print(&printer);
    delete dlg;
}

void docEditor::filePrintPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
    preview.exec();
}

void
docEditor::printPreview(QPrinter *printer)
{
    text_->print(printer);
}


void docEditor::filePrintPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this
                                                    , "Export PDF",
                                                    QString(), "*.pdf");
    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        text_->document()->print(&printer);
    }
}

void docEditor::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight( actions_[ idActionTextBold ]->isChecked() ? QFont::Bold : QFont::Normal );
    mergeFormatOnWordOrSelection(fmt);
}

void docEditor::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline( actions_[ idActionTextUnderline ]->isChecked() );
    mergeFormatOnWordOrSelection(fmt);
}

void docEditor::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic( actions_[ idActionTextItalic ]->isChecked() );
    mergeFormatOnWordOrSelection(fmt);
}

void docEditor::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily( f );
    mergeFormatOnWordOrSelection( fmt );
}

void docEditor::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize( pointSize );
        mergeFormatOnWordOrSelection( fmt );
    }
}

void docEditor::textStyle(int styleIndex)
{
    QTextCursor cursor = text_->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void docEditor::textColor()
{
    QColor col = QColorDialog::getColor(text_->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void docEditor::textAlign(QAction *a)
{
    if ( a == actions_[ idActionAlignLeft ] )
        text_->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if ( a == actions_[ idActionAlignCenter ] )
        text_->setAlignment(Qt::AlignHCenter);
    else if ( a == actions_[ idActionAlignRight ] )
        text_->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if ( a == actions_[ idActionAlignJustify ] )
        text_->setAlignment(Qt::AlignJustify);
}

void docEditor::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void docEditor::cursorPositionChanged()
{
    alignmentChanged(text_->alignment());
}

void docEditor::clipboardDataChanged()
{
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actions_[ idActionPaste ]->setEnabled( md->hasText() );
}

void docEditor::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
        "rich text editing facilities in action, providing an example "
        "document for you to experiment with."));
}

void docEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = text_->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    text_->mergeCurrentCharFormat(format);
}

void docEditor::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actions_[ idActionTextBold ]->setChecked( f.bold() );
    actions_[ idActionTextItalic ]->setChecked( f.italic() );
    actions_[ idActionTextUnderline ]->setChecked( f.underline() );
}

void docEditor::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actions_[ idActionTextColor ]->setIcon( pix );
}

void
docEditor::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actions_[ idActionAlignLeft ]->setChecked( true );
    else if (a & Qt::AlignHCenter)
        actions_[ idActionAlignCenter ]->setChecked( true );
    else if (a & Qt::AlignRight)
        actions_[ idActionAlignRight ]->setChecked( true );
    else if (a & Qt::AlignJustify)
        actions_[ idActionAlignJustify ]->setChecked( true );
}

QAbstractItemModel *
docEditor::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }


    QApplication::restoreOverrideCursor();

    return new QStringListModel(words, completer);
}

void
docEditor::currentPageChanged( int )
{
}
