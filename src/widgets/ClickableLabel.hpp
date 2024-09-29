// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT

  public:
    explicit ClickableLabel(QWidget* parent = nullptr);

  signals:
    void clicked();

  private:
    void mousePressEvent(QMouseEvent* e) override;
};
