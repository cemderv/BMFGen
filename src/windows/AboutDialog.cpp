// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "AboutDialog.hpp"

#include "MainWindow.hpp"
#include "ui_AboutDialog.h"
#include <QDesktopServices>
#include <QFile>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent, Qt::Sheet)
    , m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);

    {
        QString text = windowTitle();
        text.replace(QStringLiteral("${APP_NAME}"), BMFGEN_NAME);

        setWindowTitle(text);
    }

    {
        QString text = m_ui->lbl_info->text();
        text.replace(QStringLiteral("${APP_NAME}"), QStringLiteral(BMFGEN_NAME));
        text.replace(QStringLiteral("${APP_DISPLAY_NAME}"), QStringLiteral(BMFGEN_DISPLAY_NAME));
        text.replace(QStringLiteral("${VERSION_STRING}"), qApp->applicationVersion());
        text.replace(QStringLiteral("${APP_WEBSITE}"), QStringLiteral(BMFGEN_WEBSITE));

        m_ui->lbl_info->setText(text);
    }

    {
        QString text = m_ui->lbl_license_info->text();
        text.replace(QStringLiteral("${APP_NAME}"), QStringLiteral(BMFGEN_NAME));

        m_ui->lbl_license_info->setText(text);
    }

    // Build information
    {
        m_ui->lbl_compiler_name_value->setText(BMFGEN_COMPILER_NAME);
        m_ui->lbl_compiler_version_value->setText(BMFGEN_COMPILER_VERSION);
        m_ui->lbl_build_date_value->setText(BMFGEN_BUILD_DATE);
        m_ui->lbl_built_for_value->setText(BMFGEN_BUILT_FOR);
        m_ui->lbl_built_on_value->setText(BMFGEN_BUILT_ON);
        m_ui->lbl_qt_version_value->setText(BMFGEN_LINKED_QT_VERSION);
    }
}

AboutDialog::~AboutDialog() noexcept
{
    delete m_ui;
}

void AboutDialog::send_feedback()
{
    MainWindow::do_send_feedback();
}

void AboutDialog::report_issue()
{
    QDesktopServices::openUrl(QUrl{QStringLiteral("https://www.github.com/cemderv/BMFGen/issues")});
}
