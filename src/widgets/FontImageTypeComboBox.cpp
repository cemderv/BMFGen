// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontImageTypeComboBox.hpp"

#include "GeneratedFont.hpp"

FontImageTypeComboBox::FontImageTypeComboBox(QWidget* parent)
    : ComboBox(parent)
{
    for (const auto& [key, value] : {
             qMakePair(QStringLiteral("png"), FontExportImageType::Png),
             qMakePair(QStringLiteral("bmp"), FontExportImageType::Bmp),
         })
    {
        addItem(key, static_cast<int>(value));
    }
}

FontExportImageType FontImageTypeComboBox::image_type() const
{
    return static_cast<FontExportImageType>(currentData().toInt());
}

void FontImageTypeComboBox::set_font_image_type(FontExportImageType value)
{
    setCurrentIndex(int(value));
}
