// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QComboBox>

class ComboBox : public QComboBox
{
    Q_OBJECT

  public:
    explicit ComboBox(QWidget* parent = nullptr);

  private:
    void wheelEvent(QWheelEvent* e) override;
};
