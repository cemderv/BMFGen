// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QSpinBox>

class SpinBox : public QSpinBox
{
    Q_OBJECT

  public:
    explicit SpinBox(QWidget* parent = nullptr);

  private:
    void wheelEvent(QWheelEvent* e) override;
};
