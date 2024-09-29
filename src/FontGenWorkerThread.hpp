// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <QThread>

class FontModel;
class FontGenContext;
class GeneratedFont;

class FontGenWorkerThread : public QThread
{
    Q_OBJECT

  public:
    explicit FontGenWorkerThread(const FontModel* model, FontGenContext* context);

    void run() override;

    std::shared_ptr<GeneratedFont> generated_font() const;

  private:
    const FontModel*               m_model;
    FontGenContext*                m_context;
    std::shared_ptr<GeneratedFont> m_generated_font;
};
