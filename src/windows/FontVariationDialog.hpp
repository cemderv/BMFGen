// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class FontVariationDialog;
}
QT_END_NAMESPACE

class FontModel;
class QListWidgetItem;

class FontVariationDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit FontVariationDialog(QWidget* parent, FontModel* font);

    ~FontVariationDialog() override;

  protected:
    void accept() override;

  private slots:
    void on_item_double_clicked(QListWidgetItem* item);

  private:
    Ui::FontVariationDialog* m_ui;
    FontModel*                    m_font;
};
