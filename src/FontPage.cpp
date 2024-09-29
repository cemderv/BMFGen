// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontPage.hpp"

FontPage::FontPage(QSize size)
    : image(size, QImage::Format_RGBA8888)
{
    image.fill(Qt::transparent);
}
