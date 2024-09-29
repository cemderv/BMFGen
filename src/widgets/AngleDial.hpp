// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QDial>

class AngleDial : public QDial
{
    Q_OBJECT

  public:
    explicit AngleDial(QWidget* parent = nullptr);

  private:
    void paintEvent(QPaintEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;
};
