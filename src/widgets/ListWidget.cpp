// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ListWidget.hpp"
#include <QWheelEvent>

ListWidget::ListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void ListWidget::wheelEvent(QWheelEvent* e)
{
    if (!hasFocus())
    {
        e->ignore();
    }
    else
    {
        QListWidget::wheelEvent(e);
    }
}
