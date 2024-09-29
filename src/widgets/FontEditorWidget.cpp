// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontEditorWidget.hpp"

#include "FontGenWorkerThread.hpp"
#include "FontModel.hpp"
#include "TextPreviewWidget.hpp"
#include "ui_FontEditorWidget.h"
#include <QDir>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>

FontEditorWidget::FontEditorWidget(QWidget* parent, const QString& font_filename)
    : QWidget(parent)
    , ui(new Ui::FontEditorWidget())
{
    m_font = new FontModel(this, font_filename);

    if (!*m_font)
    {
        return;
    }

    m_last_saved_font_name = m_font->name();

    ui->setupUi(this);

    m_font_gen_context = std::make_unique<FontGenContext>(&m_all_characters);

    ui->fontWidget->set_font_model(m_font);

    connect(m_font, &FontModel::properties_changed, this, [this] {
        try
        {
            generate_preview_font();
            emit preview_font_generation_done(this);
        }
        catch (const std::exception& ex)
        {
            emit preview_font_generation_error(this, ex);
        }
    });

    connect(m_font, &FontModel::properties_only_relevant_for_save_changed, this, [this] {
        set_is_unsaved(true);
    });

    {
        QSignalBlocker blocker{this};
        generate_preview_font();
    }

    set_is_unsaved(false);
}

FontEditorWidget::~FontEditorWidget()
{
    qDebug("Font editor '%s' destroyed", qPrintable(m_font->name()));
}

void FontEditorWidget::generate_preview_font()
{
    qDebug("Generating preview font");

    if (m_font->characters().isEmpty())
    {
        throw NoCharactersSelectedError();
    }

    const QString preview_text = m_font->preview_text();

    QSet<QChar> chars;
    chars.reserve(preview_text.size());

    for (const QChar ch : preview_text)
    {
        chars.insert(ch);
    }

    const std::shared_ptr<GeneratedFont> generated_font =
        m_font_gen_context->generate_font(*m_font, chars);

    m_font->set_generated_font(generated_font);
    set_is_unsaved(false);
}

void FontEditorWidget::set_is_unsaved(bool value)
{
    if (m_is_unsaved != value)
    {
        m_is_unsaved = value;
        emit save_state_changed(this);
    }
}

bool FontEditorWidget::is_unsaved() const
{
    return m_is_unsaved;
}

void FontEditorWidget::cancel_generation()
{
    m_font_gen_context->cancel();
}

void FontEditorWidget::save_font(const QString& filename)
{
    m_font->save_to_file(filename);
    m_last_saved_font_name = m_font->name();
    set_is_unsaved(false);
}

void FontEditorWidget::export_font()
{
    qDebug("Exporting font");

    gsl::owner<FontGenWorkerThread*> worker_thread =
        new FontGenWorkerThread(m_font, m_font_gen_context.get());

    connect(worker_thread,
            &FontGenWorkerThread::started,
            this,
            &FontEditorWidget::font_export_started);

    connect(worker_thread, &FontGenWorkerThread::finished, [this, worker_thread] {
        worker_thread->deleteLater();

        const std::shared_ptr<GeneratedFont> generated_font = worker_thread->generated_font();

        set_is_unsaved(false);

        const QString font_size_str = QString::number(m_font->qfont().pixelSize());

        QString output_dir{m_font->export_directory()};
        {
            output_dir.replace("$(FONT_NAME)", m_font->name());
            output_dir.replace("$(FONT_SIZE)", font_size_str);
            output_dir = QDir::cleanPath(m_font->directory() + QDir::separator() + output_dir);
        }

        generated_font->export_to_disk(output_dir,
                                       m_font->desc_type(),
                                       m_font->image_type(),
                                       m_font->should_flip_images_upside_down(),
                                       m_font->allow_monochromatic_images());
    });

    connect(worker_thread, &FontGenWorkerThread::finished, [this] { emit font_export_done(this); });

    worker_thread->start();
}

FontModel* FontEditorWidget::font_model()
{
    return m_font;
}

const FontModel* FontEditorWidget::font_model() const
{
    return m_font;
}

QString FontEditorWidget::display_name() const
{
    return m_is_unsaved ? m_last_saved_font_name + '*' : m_last_saved_font_name;
}
