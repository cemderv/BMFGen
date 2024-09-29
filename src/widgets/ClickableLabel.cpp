// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ClickableLabel.hpp"

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

void ClickableLabel::mousePressEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
    emit clicked();
}
