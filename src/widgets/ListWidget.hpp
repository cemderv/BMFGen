// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QListWidget>

class ListWidget : public QListWidget
{
  public:
    ListWidget(QWidget* parent = nullptr);

    void wheelEvent(QWheelEvent* e) override;
};
