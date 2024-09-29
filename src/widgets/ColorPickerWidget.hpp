// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QColor>
#include <QWidget>

class QColorDialog;

class ColorPickerWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit ColorPickerWidget(QWidget* parent = nullptr);

    QColor selected_color() const noexcept;

    void set_selected_color(QColor value);

    QSize sizeHint() const override;

    void set_has_opacity(bool value);

  signals:
    void color_changed(QColor color);

  private:
    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;

    void update_text();

    void on_color_changed_in_dialog(const QColor& color);

    QString m_current_text;
    QColor  m_current_color = Qt::white;
    bool    m_has_opacity   = true;
};
