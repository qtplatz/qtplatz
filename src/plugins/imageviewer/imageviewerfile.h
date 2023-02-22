// Copyright (C) 2016 Denis Mingulov.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/idocument.h>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
class QMovie;
class QPixmap;

#ifndef QT_NO_SVG
class QGraphicsSvgItem;
#endif

QT_END_NAMESPACE

namespace ImageViewer::Internal {

class ImageViewerFile : public Core::IDocument
{
    Q_OBJECT

public:
    enum ImageType {
        TypeInvalid,
        TypeSvg,
        TypeMovie,
        TypePixmap
    };

    ImageViewerFile();
    ~ImageViewerFile() override;

    OpenResult open(QString *errorString, const Utils::FilePath &filePath,
                    const Utils::FilePath &realFilePath) override;

    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const override;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) override;

    bool isPaused() const;
    void setPaused(bool paused);

    QGraphicsItem *createGraphicsItem() const;
    ImageType type() const;

    void updateVisibility();

signals:
    void openFinished(bool success);
    void imageSizeChanged(const QSize &size);
    void isPausedChanged(bool paused);

private:
    void cleanUp();
    OpenResult openImpl(QString *errorString, const Utils::FilePath &filePath);

    ImageType m_type = TypeInvalid;
#ifndef QT_NO_SVG
    mutable QGraphicsSvgItem *m_tempSvgItem = nullptr;
#endif
    QMovie *m_movie = nullptr;
    QPixmap *m_pixmap = nullptr;
    bool m_isPaused = false;
};

} // ImageViewer::Internal
