// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QPushButton>
#include <QWidget>

class FontModel;

class FontSelectionWidget : public QPushButton
{
    Q_OBJECT

  public:
    explicit FontSelectionWidget(QWidget* parent = nullptr);

    void set_font_model(FontModel* font);

  private slots:
    void on_open_font_browser();

    void on_font_changed_in_dialog(const QFont& font);

  private:
    void update_button_text();

    FontModel* m_font;
};
