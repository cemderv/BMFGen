// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "Constants.hpp"

QString bmfgen::constants::bmfgen_font_extension()
{
    return QStringLiteral(".bmfgen");
}

QString bmfgen::constants::bmfgen_font_dialog_filter()
{
    return QStringLiteral("Font file (*%1)").arg(bmfgen_font_extension());
}
