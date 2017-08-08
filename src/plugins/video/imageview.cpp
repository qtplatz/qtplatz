/**************************************************************************
** Copyright (C) 2016-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "imageview.hpp"
#include "document.hpp"
#include "cvmat.hpp"
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QGraphicsView>
#include <QLabel>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QRect>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QtOpenGL>
#include <iostream>

class QPaintEvent;

namespace video {

    class GraphicsView : public QGraphicsView
    {
        Q_OBJECT
    public:
        GraphicsView(ImageView *v) : QGraphicsView(), view(v) { }

    protected:
        void wheelEvent( QWheelEvent * e ) override {
            if (e->modifiers() & Qt::ControlModifier) {
                if (e->delta() > 0)
                    view->zoomIn(6);
                else
                    view->zoomOut(6);
                e->accept();
            } else {
                QGraphicsView::wheelEvent(e);
            }
        }
    private:
        ImageView *view;
    };

}

using namespace video;

ImageView::ImageView( int index
                      , QWidget * parent ) : QFrame( parent )
                                           , mat_( cv::Mat() )
                                           , index_( index )
{
    setFrameStyle(Sunken | StyledPanel);
    graphicsView_ = new GraphicsView(this);

    graphicsView_->setRenderHint(QPainter::Antialiasing, false);
    graphicsView_->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView_->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView_->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/VIDEO/images/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/VIDEO/images/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);
    zoomSlider_ = new QSlider;
    zoomSlider_->setMinimum(0);
    zoomSlider_->setMaximum(1000);
    zoomSlider_->setValue(250);
    zoomSlider_->setTickPosition(QSlider::TicksRight);

    // Zoom slider layout
    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomSlider_);
    zoomSliderLayout->addWidget(zoomOutIcon);

    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setIcon(QPixmap(":/VIDEO/images/rotateleft.png"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setIcon(QPixmap(":/VIDEO/images/rotateright.png"));
    rotateRightIcon->setIconSize(iconSize);

    rotateSlider_ = new QSlider;
    rotateSlider_->setOrientation(Qt::Horizontal);
    rotateSlider_->setMinimum(1);
    rotateSlider_->setMaximum(1000);
    rotateSlider_->setValue(100);
    rotateSlider_->setTickPosition(QSlider::TicksBelow);

    // Rotate slider layout
    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
    rotateSliderLayout->addWidget(rotateLeftIcon);
    rotateSliderLayout->addWidget(rotateSlider_);
    rotateSliderLayout->addWidget(rotateRightIcon);

    resetButton_ = new QToolButton;
    resetButton_->setText(tr("0"));
    resetButton_->setEnabled(false);

    // Label layout
    QHBoxLayout *labelLayout = new QHBoxLayout;

    QButtonGroup *pointerModeGroup = new QButtonGroup(this);
    pointerModeGroup->setExclusive(true);
    for ( auto name: { "Select", "Drag" } ) {
        if ( auto btn = new QToolButton ) {
            btn->setText( name );
            btn->setObjectName( name );
            btn->setCheckable(true);
            pointerModeGroup->addButton( btn );
            labelLayout->addWidget( btn );
        }
    }

    if ( auto btn = findChild< QToolButton * >( "Select" ) )
        btn->setChecked(true);
    
    printButton_ = new QToolButton;
    printButton_->setIcon(QIcon(QPixmap(":/VIDEO/images/fileprint.png")));

    for ( auto name: { "Gray", "Log" } ) {
        if ( auto btn = new QToolButton ) {
            btn->setText( name );
            btn->setObjectName( name );
            labelLayout->addWidget( btn );
            btn->setCheckable( true );
            btn->setChecked( false );
            connect( btn, &QToolButton::toggled, this, [=]( bool toggle ) { emit toggled( this, name, toggle ); } );
        }
    }
    
    labelLayout->addStretch();

    QButtonGroup * exclsiveButtons = new QButtonGroup( this );
    exclsiveButtons->setExclusive( true );

    for ( auto name: { "DFT", "RAW", "0-Fill", "Blur" } ) {
        if ( auto btn = new QToolButton ) {
            btn->setText( name );
            btn->setObjectName( name );
            btn->setCheckable(true);
            btn->setChecked( false );
            labelLayout->addWidget( btn );
            exclsiveButtons->addButton( btn );
            connect( btn, &QToolButton::toggled, this, [=]( bool toggle ) { emit toggled( this, name, toggle ); } );
        }
    }

    if ( auto btn = findChild< QToolButton * >( "RAW" ) )
        btn->setChecked( true );
    
    labelLayout->addWidget(printButton_);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(labelLayout, 0, 0);
    topLayout->addWidget(graphicsView_, 1, 0);
    topLayout->addLayout(zoomSliderLayout, 1, 1);
    topLayout->addLayout(rotateSliderLayout, 2, 0);
    topLayout->addWidget(resetButton_, 2, 1);
    setLayout(topLayout);

    connect( resetButton_, SIGNAL(clicked()), this, SLOT(resetView()) );
    connect( zoomSlider_, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()) );

    connect( rotateSlider_, &QSlider::valueChanged, this, [&]( int z ){
            emit zValue( this, z );
        } );
    
    connect( graphicsView_->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()) );
    connect( graphicsView_->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()) );

    connect( findChild< QToolButton * >("Select"), SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()) );
    connect( findChild< QToolButton * >("Drag"), SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()) );

    connect( rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()) );
    connect( rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()) );

    connect( zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()) );
    connect( zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()) );
    connect( printButton_, SIGNAL(clicked()), this, SLOT(print()) );

    setupMatrix();
}

ImageView::~ImageView()
{
}

QSize
ImageView::sizeHint() const
{
    return qimg_.size();
}

QSize
ImageView::minimumSizeHint() const
{
    return qimg_.size();
}

void
ImageView::setImage( const QImage& image )
{
    qimg_ = image;
    auto scene = new QGraphicsScene;
    graphicsView_->setScene( scene );
    scene->addPixmap( QPixmap::fromImage( qimg_ ) );
}

void
ImageView::setImage( const cv::Mat& m )
{
    constexpr int Scale = 8;
    raw_ = cv::Mat();

    switch( m.type() ) {
    case CV_8UC1:
        cv::cvtColor( m, mat_, CV_GRAY2RGB );
        break;
    case CV_8UC3:
        cv::cvtColor( m, mat_, CV_BGR2RGB );
        break;
    case CV_32F:
        raw_ = m;
        cv::cvtColor( video::cvColor()( m ), mat_, CV_BGR2RGB );
        break;        
    }
    qimg_ = QImage( static_cast< const unsigned char *>(mat_.data), mat_.cols, mat_.rows, mat_.step, QImage::Format_RGB888 );
    auto scene = new QGraphicsScene;
    graphicsView_->setScene( scene );
    scene->addPixmap( QPixmap::fromImage( qimg_ ) );
}

void
ImageView::resetButtons()
{
    if ( auto btn = findChild< QToolButton * >( "DFT" ) )
        btn->setChecked( false );

    if ( auto btn = findChild< QToolButton * >( "Log" ) )
        btn->setChecked( false );    

    if ( auto btn = findChild< QToolButton * >( "RAW" ) )
        btn->setChecked( true );    
}

void
ImageView::drawComponents()
{
    const static QPen pen[] = { QPen( Qt::blue ), QPen( Qt::green ), QPen( Qt::red ) };
    
    auto scene = graphicsView_->scene();

    int count(0);
    // for ( auto& pair: ocr::document::instance()->compBB() ) {
    //     auto item = new QGraphicsRectItem( QRect( pair.first, pair.second ) );
    //     item->setPen( pen[ count++ % ( sizeof( pen ) / sizeof( pen[0] ) ) ] );
    //     scene->addItem( item );
    // }
}

 void
 ImageView::resetView()
 {
     zoomSlider_->setValue(250);
     rotateSlider_->setValue(0);
     setupMatrix();
     graphicsView_->ensureVisible(QRectF(0, 0, 0, 0));

     resetButton_->setEnabled(false);
 }

 void
 ImageView::setResetButtonEnabled()
 {
     resetButton_->setEnabled(true);
 }

 void
 ImageView::setupMatrix()
 {
     qreal scale = qPow(qreal(2), (zoomSlider_->value() - 128) / qreal(50));

     // ADDEBUG() << scale << ", " << zoomSlider_->value();

     QMatrix matrix;
     matrix.scale(scale, scale);
     // matrix.rotate(rotateSlider_->value());

     graphicsView_->setMatrix(matrix);
     setResetButtonEnabled();
 }

void
ImageView::togglePointerMode()
{
    if ( auto btn = findChild< QToolButton * >( "Select" ) ) {
        graphicsView_->setDragMode( btn->isChecked() ? QGraphicsView::RubberBandDrag : QGraphicsView::ScrollHandDrag );
        graphicsView_->setInteractive( btn->isChecked() );
    }
}

void
ImageView::clearImage()
{
    if ( auto button = findChild< QToolButton * >( "Detect" ) ) {
        button->setEnabled( true );
        button->setChecked( false );
    }
}

QGraphicsView *
ImageView::graphicsView()
{
    return graphicsView_;
}

void
ImageView::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        graphicsView_->render(&painter);
    }
}

void
ImageView::zoomIn(int level)
{
    zoomSlider_->setValue( zoomSlider_->value() + level );
}

void
ImageView::zoomOut(int level)
{
    zoomSlider_->setValue( zoomSlider_->value() - level );
}

void
ImageView::rotateLeft()
{
    // rotateSlider_->setValue( rotateSlider_->value() - 10 );
}

void
ImageView::rotateRight()
{
    // rotateSlider_->setValue( rotateSlider_->value() + 10 );
}

void
ImageView::setMaxZ( int z )
{
    rotateSlider_->setMaximum( z );
}

int
ImageView::z() const
{
    return rotateSlider_->value();
}

bool
ImageView::isChecked( const QString& name ) const
{
    if ( auto btn = findChild< QToolButton * >( name ) )
        return btn->isChecked();
    return false;
}

#include "imageview.moc"
