// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QFrame>
#include <QImage>

class QPainter;
class FontModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class TextPreviewWidget;
}
QT_END_NAMESPACE

class TextPreviewWidget : public QFrame
{
    Q_OBJECT

  public:
    explicit TextPreviewWidget(QWidget* parent = nullptr);

    void set_font_model(FontModel* font);

    void paintEvent(QPaintEvent* event) override;

    void showEvent(QShowEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

  private slots:
    void preview_text_changed();

    void background_color_changed(QColor color);

    void on_font_variation_changed(int index);

  private:
    void mousePressEvent(QMouseEvent* e) override;

    void update_resolution_label();

    void update_label_colors();

    Ui::TextPreviewWidget* ui     = nullptr;
    FontModel*                  m_font = nullptr;
    bool                   m_is_preview_image_dirty{};
    QImage                 m_preview_image;
    qreal                  m_current_variation_scale{};
};
