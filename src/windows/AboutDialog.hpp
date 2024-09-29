// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class AboutDialog;
}
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit AboutDialog(QWidget* parent = nullptr);

    ~AboutDialog() noexcept override;

  private slots:
    void send_feedback();

    void report_issue();

  private:
    Ui::AboutDialog* m_ui;
};
