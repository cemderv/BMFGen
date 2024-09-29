// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ImageCache.hpp"

#include <QIcon>

ImageCache::ImageCache()
{
    m_default_image = QIcon::fromTheme("image").pixmap(32, 32).toImage();
}

QImage ImageCache::lookup(const QString& filename)
{
    auto it = m_images.find(filename);

    if (it == m_images.end())
    {
        const QImage img{filename};
        if (img.isNull())
        {
            return m_default_image;
        }
        it = m_images.insert(filename, img);
    }

    return it.value();
}

void ImageCache::clear()
{
    m_images.clear();
}
