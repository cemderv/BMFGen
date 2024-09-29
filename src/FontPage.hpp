// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QImage>

struct Glyph;

class FontPage
{
  public:
    explicit FontPage(QSize size);

    QImage           image;
    QList<qsizetype> glyph_indices;
};
