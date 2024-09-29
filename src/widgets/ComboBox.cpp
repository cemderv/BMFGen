// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ComboBox.hpp"
#include <QWheelEvent>

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void ComboBox::wheelEvent(QWheelEvent* e)
{
    if (isEditable())
    {
        if (hasFocus())
        {
            e->ignore();
        }
        else
        {
            QComboBox::wheelEvent(e);
        }
    }
    else
    {
        e->ignore();
    }
}
