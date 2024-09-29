// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "CharacterSetSelectionWidget.hpp"

#include "CharacterSet.hpp"
#include "FontModel.hpp"
#include <QApplication>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <gsl/pointers>
#include <vector>

CharacterSetSelectionWidget::CharacterSetSelectionWidget(QWidget* parent)
    : QListWidget(parent)
{
    setViewMode(IconMode);
    setSelectionMode(NoSelection);
    setSortingEnabled(false);
    setMovement(Static);
    setSpacing(6);
    setUniformItemSizes(true);
}

void CharacterSetSelectionWidget::set_char_set(const CharacterSet& set)
{
    m_char_set = set;
    recompute_items();
}

void CharacterSetSelectionWidget::recompute_items()
{
    QSignalBlocker signal_blocker{this};
    m_ignore_char_selection_changed = true;

    clear();

    if (m_char_set.range_start == 0 && m_char_set.range_end == 0)
    {
        return;
    }

    const QSet<QChar> chars = m_char_set.chars();

    std::vector<QChar> sorted_chars;
    sorted_chars.reserve(chars.size());
    for (const QChar& ch : chars)
    {
        sorted_chars.push_back(ch);
    }

    std::ranges::sort(sorted_chars);

    std::vector<QListWidgetItem*> items;
    items.reserve(sorted_chars.size());

    QString str;

    for (const auto ch : sorted_chars)
    {
        str.clear();
        str.append(ch);
        if (str.isEmpty())
        {
            continue;
        }

        gsl::owner<QListWidgetItem*> item = new QListWidgetItem();
        item->setText(QStringLiteral("%1").arg(str));
        item->setData(Qt::UserRole, ch);
        addItem(item);

        items.push_back(item);
    }

    setFocus();
    m_ignore_char_selection_changed = false;
}

void CharacterSetSelectionWidget::paintEvent(QPaintEvent* event)
{
    if (count() == 0)
    {
        QPainter painter{viewport()};

        QFont qfont = this->font();
        qfont.setPointSize(13);
        painter.setFont(qfont);

        painter.drawText(rect(),
                         tr("Please select a character set from the left side."),
                         QTextOption{Qt::AlignCenter});
    }
    else
    {
        QListWidget::paintEvent(event);
    }
}
