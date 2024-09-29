// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontSelectionWidget.hpp"

#include "FontModel.hpp"
#include <QBoxLayout>
#include <QFileDialog>
#include <QFontDialog>
#include <gsl/pointers>

FontSelectionWidget::FontSelectionWidget(QWidget* parent)
    : QPushButton(parent)
    , m_font(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setIcon(QIcon::fromTheme("font"));

    connect(this, &QPushButton::clicked, this, &FontSelectionWidget::on_open_font_browser);
}

void FontSelectionWidget::on_open_font_browser()
{
    const QFont previous_font = m_font->qfont();

    gsl::owner<QFontDialog*> dialog = new QFontDialog{this};
    dialog->setModal(true);
    dialog->setCurrentFont(previous_font);

    connect(dialog,
            &QFontDialog::currentFontChanged,
            this,
            &FontSelectionWidget::on_font_changed_in_dialog);

    if (const int result = dialog->exec(); result == QFontDialog::Accepted)
    {
        QFont qfont = m_font->qfont();
        if (qfont.pixelSize() <= 0 && qfont.pointSize() > 0)
        {
            qfont.setPixelSize(qfont.pointSize());
        }
        else if (qfont.pointSize() <= 0 && qfont.pixelSize() > 0)
        {
            qfont.setPointSize(qfont.pixelSize());
        }

        m_font->set_qfont(qfont);

        update_button_text();
    }
    else
    {
        m_font->set_qfont(previous_font);
    }
}

void FontSelectionWidget::set_font_model(FontModel* font)
{
    Q_ASSERT(m_font == nullptr);
    m_font = font;
    update_button_text();
}

void FontSelectionWidget::on_font_changed_in_dialog(const QFont& font)
{
    Q_UNUSED(font);
    m_font->set_qfont(font);
}

void FontSelectionWidget::update_button_text()
{
    const QFont qfont = m_font->qfont();
    setText(tr("%1, %2px").arg(qfont.family()).arg(qfont.pixelSize()));
}
