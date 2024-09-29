// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "widgets/ComboBox.hpp"

enum class FontExportImageType;

class FontImageTypeComboBox : public ComboBox
{
    Q_OBJECT

  public:
    FontImageTypeComboBox(QWidget* parent = nullptr);

    FontExportImageType image_type() const;

    void set_font_image_type(FontExportImageType value);
};
