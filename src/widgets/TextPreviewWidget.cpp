// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "TextPreviewWidget.hpp"

#include "FontModel.hpp"
#include "GeneratedFont.hpp"
#include "ui_TextPreviewWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleHints>

template <typename Action>
static void for_each_glyph(const GeneratedFont::Variation& variation,
                           const QString&                  text,
                           const Action&                   action)
{
    if (text.isEmpty())
    {
        return;
    }

    QPoint pen{};

    const Glyph* const space_glyph = variation.find_glyph(' ');
    const int          line_height = variation.line_height;

    for (const QChar ch : text)
    {
        if (ch == ' ')
        {
            if (space_glyph)
            {
                pen.setX(pen.x() + space_glyph->horizontal_advance);
            }
        }
        else if (ch == '\r')
        {
            // Nothing to do
        }
        else if (ch == '\n')
        {
            pen.setX(0);
            pen.setY(pen.y() + line_height);
        }
        else
        {
            const Glyph* glyph = variation.find_glyph(ch);

            if (!glyph)
            {
                glyph = space_glyph;
            }

            if (glyph)
            {
                action(pen, *glyph);
                pen += QPoint{glyph->horizontal_advance, 0};
            }
        }
    }
}

static QSize measure_text(const GeneratedFont::Variation& variation, const QString& text)
{
    QSize ret{};

    for_each_glyph(variation, text, [&](QPoint position, const Glyph& glyph) {
        const int right  = position.x() + glyph.rect.width();
        const int bottom = position.y() + glyph.rect.height();

        if (right > ret.width())
        {
            ret.setWidth(right);
        }

        if (bottom > ret.height())
        {
            ret.setHeight(bottom);
        }
    });

    return ret;
}

static QImage draw_text_into_image(const GeneratedFont::Variation& variation, const QString& text)
{
    const QSize img_size = measure_text(variation, text);

    QImage ret{img_size, QImage::Format_RGBA8888};
    ret.fill(Qt::transparent);

    QPainter painter{&ret};

    for_each_glyph(variation, text, [&](QPoint position, const Glyph& glyph) {
        painter.drawImage(position.x(), position.y(), glyph.image);
    });

    return ret;
}

TextPreviewWidget::TextPreviewWidget(QWidget* parent)
    : QFrame(parent)
    , ui(new Ui::TextPreviewWidget())
    , m_is_preview_image_dirty(true)
    , m_current_variation_scale(1.0)
{
    ui->setupUi(this);
    ui->background_color_picker->set_has_opacity(false);

    setFrameStyle(NoFrame);

    connect(ui->txt_preview_text,
            &QPlainTextEdit::textChanged,
            this,
            &TextPreviewWidget::preview_text_changed);
}

void TextPreviewWidget::set_font_model(FontModel* font)
{
    Q_ASSERT(font);
    m_font = font;

    ui->txt_preview_text->setPlainText(m_font->preview_text());

    ui->background_color_picker->set_selected_color(m_font->preview_background_color());

    connect(m_font, &FontModel::generated_font_changed, this, [this] {
        m_is_preview_image_dirty = true;

        if (!m_font->generated_font())
        {
            return;
        }

        const auto variations_visible = m_font->generated_font()->variation_count() > 1;
        ui->lbl_variation->setVisible(variations_visible);
        ui->cmb_variation->setVisible(variations_visible);

        if (variations_visible)
        {
            ui->cmb_variation->clear();

            QList<qreal> variations;
            variations.reserve(m_font->variations().size());
            for (const double& var : m_font->variations())
            {
                variations.push_back(var);
            }
            std::ranges::sort(variations);

            for (const qreal& var : variations)
            {
                ui->cmb_variation->addItem(tr("%1x Scale").arg(var), var);
            }

            ui->cmb_variation->setCurrentIndex(ui->cmb_variation->findData(1.0));
            m_current_variation_scale = 1.0;
        }

        update();
    });
}

void TextPreviewWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    const GeneratedFont* generated_font = m_font->generated_font();

    if (generated_font == nullptr || generated_font->variation_count() == 0)
    {
        return;
    }

    const GeneratedFont::Variation& variation =
        generated_font->variation_by_scale(m_current_variation_scale);

    if (m_is_preview_image_dirty)
    {
        const auto preview_text = m_font->preview_text();

        m_preview_image =
            preview_text.isEmpty() ? QImage() : draw_text_into_image(variation, preview_text);

        m_is_preview_image_dirty = false;
    }

    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    {
        QPainterPath path;
        path.addRoundedRect(rect(), 3.0, 3.0);
        painter.fillPath(path, ui->background_color_picker->selected_color());
    }

    if (!m_preview_image.isNull())
    {
        const QSize image_size = m_preview_image.size() / devicePixelRatioF();

        const QRect image_rect{
            (width() - image_size.width()) / 2,
            (height() - image_size.height()) / 2,
            image_size.width(),
            image_size.height(),
        };

        painter.drawImage(image_rect, m_preview_image);
    }

    QFrame::paintEvent(event);
}

void TextPreviewWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    update_resolution_label();
}

void TextPreviewWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    update_resolution_label();
}

void TextPreviewWidget::preview_text_changed()
{
    const QString plain_text = ui->txt_preview_text->toPlainText();
    ui->txt_preview_text->setMaximumHeight(plain_text.contains('\n') ? 60 : 25);
    m_font->set_preview_text(plain_text);
    m_is_preview_image_dirty = true;
    update();
}

void TextPreviewWidget::background_color_changed(QColor color)
{
    m_font->set_preview_background_color(color);
    update_label_colors();
    update();
}

void TextPreviewWidget::on_font_variation_changed(int index)
{
    bool   is_ok{};
    double scale = ui->cmb_variation->itemData(index).toDouble(&is_ok);

    if (!is_ok)
    {
        scale = 1.0;
    }

    if (scale != m_current_variation_scale)
    {
        m_current_variation_scale = scale;
        m_is_preview_image_dirty  = true;
        update();
    }
}

void TextPreviewWidget::mousePressEvent(QMouseEvent* e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
    {
        setFocus(Qt::MouseFocusReason);
    }
}

void TextPreviewWidget::update_resolution_label()
{
    const QSize scaled_size = size() * devicePixelRatioF();

    ui->lbl_resolution->setText(
        QStringLiteral("View size: %1x%2").arg(scaled_size.width()).arg(scaled_size.height()));
}

void TextPreviewWidget::update_label_colors()
{
    const QColor color =
        ui->background_color_picker->selected_color().lightnessF() < 0.5 ? Qt::white : Qt::black;

    const QString stylesheet = QStringLiteral("color: rgb(%1,%2,%3)")
                                   .arg(color.red())
                                   .arg(color.green())
                                   .arg(color.blue());

    ui->lbl_resolution->setStyleSheet(stylesheet);
    ui->lbl_background_color->setStyleSheet(stylesheet);
}
