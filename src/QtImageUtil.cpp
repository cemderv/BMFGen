// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "QtImageUtil.hpp"

#include <QImage>

bool QtImageUtil::is_monochromatic(const QImage& image)
{
    const int width  = image.width();
    const int height = image.height();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (const QColor color = image.pixelColor(x, y);
                color.red() != color.green() || color.green() != color.blue())
            {
                return false;
            }
        }
    }

    return true;
}
