// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QImage>

struct Glyph
{
    QChar  character;
    QRect  rect;
    int    page_index{};
    int    horizontal_advance{};
    int    left_bearing{};
    int    right_bearing{};
    QImage image;
};
