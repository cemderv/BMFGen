// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

class QImage;

class QtImageUtil final
{
  public:
    QtImageUtil() = delete;

    static bool is_monochromatic(const QImage& image);
};
