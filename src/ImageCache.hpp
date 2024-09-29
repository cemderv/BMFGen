// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QHash>
#include <QImage>
#include <QObject>

class ImageCache final : public QObject
{
    Q_OBJECT

  public:
    ImageCache();

    QImage lookup(const QString& filename);

    void clear();

  private:
    QImage                 m_default_image;
    QHash<QString, QImage> m_images;
};
