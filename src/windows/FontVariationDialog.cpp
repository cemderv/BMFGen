// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "FontVariationDialog.hpp"

#include "FontModel.hpp"
#include "ui_FontVariationDialog.h"

FontVariationDialog::FontVariationDialog(QWidget* parent, FontModel* font)
    : QDialog(parent, Qt::Sheet)
    , m_ui(new Ui::FontVariationDialog)
    , m_font(font)
{
    m_ui->setupUi(this);

    const QSet<double> var = font->variations();

    m_ui->list_widget->item(1)->setCheckState(var.contains(0.5) ? Qt::Checked : Qt::Unchecked);
    m_ui->list_widget->item(2)->setCheckState(var.contains(1.5) ? Qt::Checked : Qt::Unchecked);
    m_ui->list_widget->item(3)->setCheckState(var.contains(2.0) ? Qt::Checked : Qt::Unchecked);
}

FontVariationDialog::~FontVariationDialog()
{
    delete m_ui;
}

void FontVariationDialog::accept()
{
    QSet<double> variations;
    variations.insert(1.0);

    if (m_ui->list_widget->item(1)->checkState() == Qt::Checked)
    {
        variations.insert(0.5);
    }

    if (m_ui->list_widget->item(2)->checkState() == Qt::Checked)
    {
        variations.insert(1.5);
    }

    if (m_ui->list_widget->item(3)->checkState() == Qt::Checked)
    {
        variations.insert(2.0);
    }

    m_font->set_variations(variations);

    QDialog::accept();
}

void FontVariationDialog::on_item_double_clicked(QListWidgetItem* item)
{
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}
