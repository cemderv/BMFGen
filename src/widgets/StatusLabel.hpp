// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QWidget>

class QTimer;

class StatusLabel final : public QWidget
{
    Q_OBJECT

  public:
    explicit StatusLabel(QWidget* parent = nullptr);

    void ShowStatus(const QString& text, int durationMs = 2000);

    void paintEvent(QPaintEvent* event) override;

  signals:
    void visibility_requested(bool visible);

  private:
    float remaining_time_in_percent() const;

    QTimer* m_timer;
    QString m_currentText;
    int     m_currentDurationMs;
};
