// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "SpinBox.hpp"
#include <QWheelEvent>

SpinBox::SpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void SpinBox::wheelEvent(QWheelEvent* e)
{
    if (!hasFocus())
    {
        e->ignore();
    }
    else
    {
        QSpinBox::wheelEvent(e);
    }
}
