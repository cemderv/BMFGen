// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "CharacterSetsDialog.hpp"

#include "FontModel.hpp"
#include "ui_CharacterSetsDialog.h"
#include "widgets/CharacterSetSelectionWidget.hpp"

const CharacterSet& get_character_set(const QListWidgetItem* item)
{
    const auto variant = item->data(Qt::UserRole);
    return *static_cast<const CharacterSet*>(variant.value<const void*>());
}

CharacterSetsDialog::CharacterSetsDialog(QWidget* parent, FontModel* font)
    : QDialog(parent, Qt::Sheet)
    , m_ui(new Ui::CharacterSetsDialog)
    , m_font(font)
{
    m_char_set_list = {
        {.name = QStringLiteral("Basic Latin"), .range_start = 32U, .range_end = 127U},
        {.name = QStringLiteral("Latin-1 Supplement"), .range_start = 160U, .range_end = 255U},
        {.name = QStringLiteral("Latin Extended A"), .range_start = 256U, .range_end = 383U},
        {.name = QStringLiteral("Latin Extended B"), .range_start = 384U, .range_end = 591U},
        {.name = QStringLiteral("IPA Extensions"), .range_start = 592U, .range_end = 687U},
        {.name        = QStringLiteral("Spacing Modifier Letters"),
         .range_start = 688U,
         .range_end   = 767U},
        {.name        = QStringLiteral("Combining Diacritical Marks"),
         .range_start = 768U,
         .range_end   = 879U},
        {.name = QStringLiteral("Greek / Coptic"), .range_start = 880U, .range_end = 1023U},
        {.name = QStringLiteral("Cyrillic"), .range_start = 1024U, .range_end = 1279U},
        {.name = QStringLiteral("Cyrillic Supplement"), .range_start = 1280U, .range_end = 1327U},
        {.name = QStringLiteral("Armenian"), .range_start = 1328U, .range_end = 1423U},
        {.name = QStringLiteral("Hebrew"), .range_start = 1424U, .range_end = 1535U},
        {.name = QStringLiteral("Arabic"), .range_start = 1536U, .range_end = 1791U},
        {.name = QStringLiteral("Syriac"), .range_start = 1792U, .range_end = 1871U},
    };

    m_ui->setupUi(this);

    {
        QFont qfont = this->font();
        qfont.setFamily("SF Pro");
        m_ui->cmb_preview_font->setCurrentFont(qfont);

        qfont.setPointSize(16);
        m_ui->char_set_list_widget->setFont(qfont);
    }

    m_chars_backup = font->characters();

    populate_char_sets_list_widget();
}

CharacterSetsDialog::~CharacterSetsDialog()
{
    delete m_ui;
}

void CharacterSetsDialog::on_item_selection_changed()
{
    const QList<QListWidgetItem*> selected_items = m_ui->char_set_list_widget->selectedItems();

    if (!selected_items.isEmpty())
    {
        QListWidgetItem* item = selected_items.front();
        m_ui->chars_list_widget->set_char_set(get_character_set(item));
    }
}

void CharacterSetsDialog::on_preview_font_changed(const QFont& font)
{
    m_ui->chars_list_widget->setFont(font);
}

void CharacterSetsDialog::on_char_set_item_changed(QListWidgetItem* item)
{
    const CharacterSet character_set = get_character_set(item);

    if (item->checkState() == Qt::Checked)
    {
        for (const auto ch : character_set.chars())
        {
            m_chars_backup.insert(ch);
        }
    }
    else
    {
        for (const auto ch : character_set.chars())
        {
            m_chars_backup.remove(ch);
        }
    }

    update_chars_selected_info_label();
}

void CharacterSetsDialog::on_item_double_clicked(QListWidgetItem* item)
{
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}

void CharacterSetsDialog::on_select_all_char_sets()
{
    const int count = m_ui->char_set_list_widget->count();
    for (int i = 0; i < count; ++i)
    {
        m_ui->char_set_list_widget->item(i)->setCheckState(Qt::Checked);
    }
}

void CharacterSetsDialog::on_select_no_char_sets()
{
    const int count = m_ui->char_set_list_widget->count();
    for (int i = 0; i < count; ++i)
    {
        m_ui->char_set_list_widget->item(i)->setCheckState(Qt::Unchecked);
    }
}

void CharacterSetsDialog::accept()
{
    m_font->set_characters(m_chars_backup);
    QDialog::accept();
}

void CharacterSetsDialog::populate_char_sets_list_widget()
{
    for (const CharacterSet& char_set : m_char_set_list)
    {
        auto item = std::make_unique<QListWidgetItem>();
        item->setText(char_set.name);
        item->setToolTip(tr("Range %1 - %2").arg(char_set.range_start).arg(char_set.range_end));
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<const void*>(&char_set)));
        item->setFlags(item->flags() | Qt::ItemFlag::ItemIsUserCheckable);

        const QSet<QChar> char_set_chars = char_set.chars();

        const auto check_state = [&] {
            bool has_at_least_one_char = false;
            for (const QChar ch : char_set_chars)
            {
                if (!m_chars_backup.contains(ch))
                {
                    return has_at_least_one_char ? Qt::PartiallyChecked : Qt::Unchecked;
                }

                has_at_least_one_char = true;
            }
            return Qt::Checked;
        }();

        item->setCheckState(check_state);

        m_ui->char_set_list_widget->addItem(item.release());
    }

    update_chars_selected_info_label();
}

void CharacterSetsDialog::update_chars_selected_info_label()
{
    m_ui->lbl_chars_selected_info->setText(tr("%1 characters selected").arg(m_chars_backup.size()));
}
