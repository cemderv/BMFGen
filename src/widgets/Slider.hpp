// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT

  public:
    explicit Slider(QWidget* parent = nullptr);

  private:
    void wheelEvent(QWheelEvent* event) override;
};
