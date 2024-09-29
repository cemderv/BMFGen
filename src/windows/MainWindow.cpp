// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "MainWindow.hpp"

#include "Constants.hpp"
#include "FontModel.hpp"
#include "ui_MainWindow.h"
#include "widgets/FontEditorWidget.hpp"
#include "widgets/StatusLabel.hpp"
#include "windows/AboutDialog.hpp"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyleHints>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_waiting_spinner(nullptr)
    , m_prg_bar_font_gen_action(nullptr)
    , m_status_label(nullptr)
    , m_status_label_action(nullptr)
{
    m_ui->setupUi(this);

    setWindowIcon(QIcon{":/icon512.png"});

#if defined(Q_OS_WINDOWS)
    m_ui->menu_bar->hide();
#endif

    m_ui->stacked_widget->setCurrentIndex(1);

    setWindowTitle(BMFGEN_NAME);
    setUnifiedTitleAndToolBarOnMac(true);
    build_toolbar();

    connect(qApp->styleHints(),
            &QStyleHints::colorSchemeChanged,
            this,
            [](Qt::ColorScheme new_scheme) {
                if (new_scheme == Qt::ColorScheme::Dark)
                {
                    QIcon::setFallbackSearchPaths(QStringList{} << ":/ui/icons/osx-dark");
                    QIcon::setThemeName("osx-dark");
                }
                else
                {
                    QIcon::setFallbackSearchPaths(QStringList{} << ":/ui/icons/osx-light");
                    QIcon::setThemeName("osx-light");
                }
            });
}

MainWindow::~MainWindow() noexcept
{
    delete m_ui;
}

void MainWindow::do_send_feedback()
{
    QDesktopServices::openUrl(QStringLiteral("mailto:cem@dervis.de"));
}

void MainWindow::show_about_app()
{
    AboutDialog dlg{this};
    dlg.exec();
}

void MainWindow::send_feedback()
{
    do_send_feedback();
}

void MainWindow::on_new_font_action()
{
    const QString filename = QFileDialog::getSaveFileName(
        this,
        tr("New Font"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        bmfgen::constants::bmfgen_font_dialog_filter());

    if (!filename.isEmpty())
    {
        create_new_font(filename);
    }
}

void MainWindow::on_open_font_action()
{
    const QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open Font"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        bmfgen::constants::bmfgen_font_dialog_filter());

    if (!filename.isEmpty())
    {
        open_font(filename);
    }
}

void MainWindow::on_save_font_action()
{
    save_font(current_font_editor());
}

void MainWindow::on_export_font_action()
{
    export_font(current_font_editor());
}

void MainWindow::on_shared_font_config_changed()
{
    m_status_label->ShowStatus(tr("Shared font configuration has changed."), 6000);
}

void MainWindow::on_tab_close_requested(int index)
{
    const auto editor = qobject_cast<FontEditorWidget*>(m_ui->fonts_tab_widget->widget(index));

    if (editor != nullptr && handle_unsaved_changed_for_font_editor(editor))
    {
        close_font(editor);
    }
}

void MainWindow::on_current_tab_changed(int index)
{
    Q_UNUSED(index);
    update_header_buttons_enabledness();
}

void MainWindow::on_font_export_started()
{
    m_waiting_spinner->start();
    m_prg_bar_font_gen_action->setVisible(true);
}

void MainWindow::on_font_export_done(FontEditorWidget* editor)
{
    m_waiting_spinner->stop();
    m_prg_bar_font_gen_action->setVisible(false);
    m_status_label->ShowStatus(tr("Exported font '%1'").arg(editor->font_model()->name()), 3000);
    m_ui->fonts_tab_widget->setEnabled(true);
}

void MainWindow::on_font_editor_save_state_changed(const FontEditorWidget* editor)
{
    if (const std::optional<int> idx = index_of_font_editor(editor))
    {
        m_ui->fonts_tab_widget->setTabText(*idx, editor->display_name());
    }

    update_header_buttons_enabledness();
}

void MainWindow::build_toolbar()
{
    const auto add_spacer = [this](int width) {
        auto spacer = std::make_unique<QWidget>();
        spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        spacer->setFixedWidth(width);
        m_ui->tool_bar->addWidget(spacer.release());
    };

    const auto add_expander = [this] {
        auto spacer = std::make_unique<QWidget>();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_ui->tool_bar->addWidget(spacer.release());
    };

    add_spacer(10);

    m_ui->tool_bar->addAction(m_ui->action_new_font);
    m_ui->tool_bar->addAction(m_ui->action_open_font);
    m_ui->tool_bar->addAction(m_ui->action_save_font);

    m_ui->tool_bar->addAction(m_ui->action_export_font);
    add_spacer(20);

    {
        m_waiting_spinner         = new WaitingSpinnerWidget();
        m_prg_bar_font_gen_action = m_ui->tool_bar->addWidget(m_waiting_spinner);
    }

    add_expander();

    {
        m_status_label = new StatusLabel();
        m_status_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_status_label_action = m_ui->tool_bar->addWidget(m_status_label);

        connect(m_status_label, &StatusLabel::visibility_requested, this, [this](bool visible) {
            m_status_label_action->setVisible(visible);
        });
    }

    add_expander();

    m_ui->tool_bar->addAction(m_ui->action_send_feedback);
    m_ui->tool_bar->addAction(m_ui->action_about);

    add_spacer(10);

    m_prg_bar_font_gen_action->setVisible(false);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    update_header_buttons_enabledness();
}

void MainWindow::create_new_font(const QString& filename)
{
    if (const std::optional<QString> error_message = FontModel::create_new_font_file_at(filename))
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create the font: %1").arg(filename));
        return;
    }

    open_font(filename);
}

void MainWindow::open_font(const QString& filename)
{
    qDebug("Opening font '%s'", qPrintable(filename));

    // See if the font is already opened.
    {
        const QFileInfo file_info{filename};
        const int       count = m_ui->fonts_tab_widget->count();
        for (int i = 0; i < count; ++i)
        {
            const auto editor = qobject_cast<FontEditorWidget*>(m_ui->fonts_tab_widget->widget(i));

            if (QFileInfo{editor->font_model()->filename()} == file_info)
            {
                qDebug("  - Already open");
                m_ui->fonts_tab_widget->setCurrentWidget(editor);
            }
        }
    }

    gsl::owner<FontEditorWidget*> font_editor = new FontEditorWidget(this, filename);

    if (!*font_editor->font_model())
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load the font."));
        return;
    }

    const FontModel* font      = font_editor->font_model();
    const int        tab_index = m_ui->fonts_tab_widget->addTab(font_editor, font->name());

    m_ui->fonts_tab_widget->setTabToolTip(tab_index, font->filename());
    m_ui->fonts_tab_widget->setCurrentIndex(tab_index);

    connect(font_editor,
            &FontEditorWidget::save_state_changed,
            this,
            &MainWindow::on_font_editor_save_state_changed);
    connect(font_editor,
            &FontEditorWidget::font_export_started,
            this,
            &MainWindow::on_font_export_started);
    connect(font_editor,
            &FontEditorWidget::font_export_done,
            this,
            &MainWindow::on_font_export_done);

    m_ui->stacked_widget->setCurrentIndex(0);
}

void MainWindow::save_font(FontEditorWidget* editor)
{
    editor->save_font(editor->font_model()->filename());
    m_status_label->ShowStatus(tr("Saved font '%1'").arg(editor->font_model()->name()), 3000);
}

void MainWindow::export_font(FontEditorWidget* editor)
{
    try
    {
        m_status_label->hide();
        m_ui->fonts_tab_widget->setEnabled(false);
        editor->save_font(editor->font_model()->filename());
        editor->export_font();
    }
    catch (const std::exception& ex)
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Failed to export the font: %1").arg(ex.what()));
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!handle_unsaved_changes_for_all_editors())
    {
        event->ignore();
    }
}

void MainWindow::close_font(gsl::owner<FontEditorWidget*> editor)
{
    if (!handle_unsaved_changed_for_font_editor(editor))
    {
        return;
    }

    if (const int idx = m_ui->fonts_tab_widget->indexOf(editor); idx >= 0)
    {
        m_ui->fonts_tab_widget->removeTab(idx);
        delete editor;
    }

    if (m_ui->fonts_tab_widget->count() == 0)
    {
        m_ui->stacked_widget->setCurrentIndex(1);
    }
}

bool MainWindow::handle_unsaved_changes_for_all_editors()
{
    const int count = m_ui->fonts_tab_widget->count();

    for (int i = 0; i < count; ++i)
    {
        if (const auto editor = qobject_cast<FontEditorWidget*>(m_ui->fonts_tab_widget->widget(i));
            !handle_unsaved_changed_for_font_editor(editor))
        {
            return false;
        }
    }

    return true;
}

bool MainWindow::handle_unsaved_changed_for_font_editor(FontEditorWidget* editor)
{
    if (!editor->is_unsaved())
    {
        return true;
    }

    const QString msg =
        tr("Do you want to save changes made to %1?").arg(editor->font_model()->name());

    const QMessageBox::StandardButton result =
        QMessageBox::question(this,
                              BMFGEN_NAME,
                              msg,
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (result == QMessageBox::Yes)
    {
        try
        {
            save_font(editor);
        }
        catch (const std::exception& ex)
        {
            QMessageBox::critical(this, tr("Error"), ex.what());
        }
    }
    else if (result == QMessageBox::No)
    {
        // Count as handled.
        return true;
    }
    else if (result == QMessageBox::Cancel)
    {
        return false;
    }

    return true;
}

void MainWindow::update_header_buttons_enabledness()
{
    FontEditorWidget* current_editor = current_font_editor();

    m_ui->action_save_font->setEnabled(current_editor != nullptr && current_editor->is_unsaved());
    m_ui->action_export_font->setEnabled(current_editor != nullptr);
}

FontEditorWidget* MainWindow::current_font_editor() const
{
    return qobject_cast<FontEditorWidget*>(m_ui->fonts_tab_widget->currentWidget());
}

std::optional<int> MainWindow::index_of_font_editor(const FontEditorWidget* widget) const
{
    const int count = m_ui->fonts_tab_widget->count();

    for (int i = 0; i < count; ++i)
    {
        if (m_ui->fonts_tab_widget->widget(i) == widget)
        {
            return i;
        }
    }

    return {};
}
