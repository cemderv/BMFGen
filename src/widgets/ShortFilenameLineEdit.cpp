// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "ShortFilenameLineEdit.hpp"

#include <QRegularExpressionValidator>

class InputValidator : public QRegularExpressionValidator
{
  public:
    InputValidator()
    {
        setRegularExpression(QRegularExpression{R"(^[^.\\\\/:*?\"<>|]?[^\\\\/:*?\"<>|]*)"});
    }

    State validate(QString& text, int& pos) const override
    {
        return QRegularExpressionValidator::validate(text, pos);
    }

    void fixup(QString& text) const override
    {
        Q_UNUSED(text);
    }
};

ShortFilenameLineEdit::ShortFilenameLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    static const InputValidator validator;

    setValidator(&validator);
}
