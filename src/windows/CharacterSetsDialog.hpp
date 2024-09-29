// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QDialog>
#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui
{
class CharacterSetsDialog;
}
QT_END_NAMESPACE

class FontModel;
class CharacterSet;
class QListWidgetItem;

class CharacterSetsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit CharacterSetsDialog(QWidget* parent, FontModel* font);

    ~CharacterSetsDialog() override;

  private slots:
    void on_item_selection_changed();

    void on_preview_font_changed(const QFont& font);

    void on_char_set_item_changed(QListWidgetItem* item);

    void on_item_double_clicked(QListWidgetItem* item);

    void on_select_all_char_sets();

    void on_select_no_char_sets();

  private:
    void accept() override;

    void populate_char_sets_list_widget();

    void update_chars_selected_info_label();

    Ui::CharacterSetsDialog* m_ui{};
    FontModel*                    m_font{};
    QSet<QChar>              m_chars_backup;
    QList<CharacterSet>      m_char_set_list;
};
