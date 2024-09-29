// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "FontGenContext.hpp"
#include <QSet>
#include <QWidget>
#include <gsl/pointers>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FontEditorWidget;
}
QT_END_NAMESPACE

class QTimer;
class FontModel;

class FontEditorWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit FontEditorWidget(QWidget* parent, const QString& font_filename);

    ~FontEditorWidget() override;

    bool is_unsaved() const;

    void cancel_generation();

    void save_font(const QString& filename);

    void export_font();

    FontModel* font_model();

    const FontModel* font_model() const;

    QString display_name() const;

  signals:
    void font_export_started();

    void font_export_done(FontEditorWidget* editor);

    void preview_font_generation_done(FontEditorWidget* editor);

    void preview_font_generation_error(FontEditorWidget* editor, const std::exception& ex);

    void save_state_changed(FontEditorWidget* editor);

  private slots:
    void generate_preview_font();

  private:
    void set_is_unsaved(bool value);

    Ui::FontEditorWidget*           ui{};
    gsl::owner<FontModel*>          m_font{};
    bool                            m_is_unsaved{};
    QString                         m_last_saved_font_name;
    QSet<QChar>                     m_all_characters{};
    std::unique_ptr<FontGenContext> m_font_gen_context{};
    uint64_t                        m_generation_count{};
};
