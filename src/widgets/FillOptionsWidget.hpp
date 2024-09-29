// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "FontModel.hpp"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FillOptionsWidget;
}
QT_END_NAMESPACE

class FillOptionsWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit FillOptionsWidget(QWidget* parent = nullptr);

    ~FillOptionsWidget() noexcept override;

    void set_font_model(FontModel* font, const FontFill& fill);

    void set_fill_enabled(bool value);

  Q_SIGNALS:
    void fill_changed(const FontFill& value);

  private Q_SLOTS:
    void on_open_fill_image();

    void on_fill_type_changed();

    void on_solid_color_changed(QColor color);

    void on_gradient_changed(const QGradientStops& stops);

    void on_gradient_angle_changed(int value);

    void on_gradient_offset_x_changed(int value);

    void on_gradient_offset_y_changed(int value);

    void on_gradient_radius_changed(int value);

  private:
    void update_fill_group_visibility();

    Ui::FillOptionsWidget* ui;
    FontModel*             m_font;
    FontFill               m_fill;
};
