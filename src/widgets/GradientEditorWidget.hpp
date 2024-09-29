// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QGradientStops>
#include <QWidget>

class QColorDialog;

class GradientEditorWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit GradientEditorWidget(QWidget* parent = nullptr);

    ~GradientEditorWidget();

    QGradientStops stops() const;

    void set_stops(const QGradientStops& colors);

  Q_SIGNALS:
    void background_changed(const QBrush&);

    void stops_changed(const QGradientStops&);

  protected:
    void paintEvent(QPaintEvent* ev) override;

    void mousePressEvent(QMouseEvent* ev) override;

    void mouseMoveEvent(QMouseEvent* ev) override;

    void mouseReleaseEvent(QMouseEvent* ev) override;

    void leaveEvent(QEvent* event) override;

    void mouseDoubleClickEvent(QMouseEvent* event) override;

  private:
    void refresh_gradient();

    qreal paint_pos(const QGradientStop& stop) const;

    int closest(QPoint p);

    qreal move_pos(QPoint p) const;

    void add_stop_data(int& index, qreal& pos, QColor& color);

    void remove_selected_stop(QPoint point);

    void on_color_changed_in_dialog(const QColor& color);

    QGradientStops  m_stops;
    QBrush          m_back;
    int             m_highlighted{};
    QLinearGradient m_gradient;
    int             m_selected{};
    int             m_drop_index{};
    QColor          m_drop_color;
    qreal           m_drop_pos{};
    int             m_dialog_selected{};
    QColorDialog*   m_color_dialog{};
};
