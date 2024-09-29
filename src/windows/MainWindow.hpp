// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include "widgets/WaitingSpinner.hpp"
#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include <gsl/pointers>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class FontEditorWidget;
class QProgressBar;
class QPushButton;
class StatusLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() noexcept override;

    static void do_send_feedback();

  private slots:
    void show_about_app();

    void send_feedback();

    void on_new_font_action();

    void on_open_font_action();

    void on_save_font_action();

    void on_export_font_action();

    void on_shared_font_config_changed();

    void on_tab_close_requested(int index);

    void on_current_tab_changed(int index);

    void on_font_export_started();

    void on_font_export_done(FontEditorWidget* editor);

    void on_font_editor_save_state_changed(const FontEditorWidget* editor);

  private:
    void build_toolbar();

    void showEvent(QShowEvent* event) override;

    void create_new_font(const QString& filename);

    void open_font(const QString& filename);

    void save_font(FontEditorWidget* editor);

    void export_font(FontEditorWidget* editor);

    void closeEvent(QCloseEvent* event) override;

    void close_font(gsl::owner<FontEditorWidget*> editor);

    bool handle_unsaved_changes_for_all_editors();

    bool handle_unsaved_changed_for_font_editor(FontEditorWidget* editor);

    void update_header_buttons_enabledness();

    FontEditorWidget* current_font_editor() const;

    std::optional<int> index_of_font_editor(const FontEditorWidget* widget) const;

    Ui::MainWindow*                   m_ui;
    gsl::owner<WaitingSpinnerWidget*> m_waiting_spinner;
    QAction*                          m_prg_bar_font_gen_action;
    gsl::owner<StatusLabel*>          m_status_label;
    QAction*                          m_status_label_action;
};
