// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "RightAlignedLabel.hpp"

RightAlignedLabel::RightAlignedLabel(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(alignment() | Qt::AlignRight);
}
