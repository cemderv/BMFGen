#pragma once

#include <QWidget>

/**
 * \brief Display an analog widget that allows the selection of a HSV color
 *
 * It has an outer wheel to select the hue and an intenal square to select
 * saturation and Lightness.
 */
class ColorWheelWidget final : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(
        QColor color READ color WRITE set_color NOTIFY color_changed DESIGNABLE true STORED false)
    Q_PROPERTY(float hue READ hue WRITE set_hue DESIGNABLE false)
    Q_PROPERTY(float saturation READ saturation WRITE set_saturation DESIGNABLE false)
    Q_PROPERTY(float value READ value WRITE set_value DESIGNABLE false)
    Q_PROPERTY(unsigned wheelWidth READ wheel_width WRITE set_wheel_width NOTIFY wheel_width_changed
                   DESIGNABLE true)

  public:
    explicit ColorWheelWidget(QWidget* parent = nullptr);

    ~ColorWheelWidget() override;

    QColor color() const;

    QSize sizeHint() const override;

    qreal hue() const;

    qreal saturation() const;

    qreal value() const;

    unsigned int wheel_width() const;

    void set_wheel_width(unsigned int w);

    void paintEvent(QPaintEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;

    void dropEvent(QDropEvent* event) override;

  public slots:
    void set_color(QColor c);

    void set_hue(qreal h);

    void set_saturation(qreal s);

    void set_value(qreal v);

  signals:
    void color_changed(QColor);

    void ColorSelected(QColor);

    void wheel_width_changed(unsigned);

  private:
    enum class MouseStatus
    {
        Nothing,
        DragCircle,
        DragSquare
    };

    qreal outer_radius() const;

    qreal inner_radius() const;

    qreal square_size() const;

    qreal triangle_height() const;

    qreal triangle_side() const;

    QLineF line_to_point(const QPoint& p) const;

    void init_buffer(QSize size);

    void render_inner_selector();

    QPointF selector_image_offset() const;

    QSizeF selector_size() const;

    qreal selector_image_angle() const;

    void render_ring();

    void set_selected_color(const QColor& c);

    void draw_ring_editor(double editorHue, QPainter& painter, QColor color) const;

    qreal                 m_hue;
    qreal                 m_saturation;
    qreal                 m_value;
    bool                  m_is_background_dark;
    unsigned int          m_wheel_width;
    MouseStatus           m_mouse_status;
    QPixmap               m_hue_ring;
    QImage                m_inner_selector;
    std::vector<uint32_t> m_inner_selector_buffer;
};
