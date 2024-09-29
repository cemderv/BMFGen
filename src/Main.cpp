// Copyright (C) 2021-2024 Cemalettin Dervis
// This file is part of BMFGen.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "windows/MainWindow.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommonStyle>
#include <QFile>
#include <QStyleHints>

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName(BMFGEN_AUTHOR);
    QCoreApplication::setOrganizationDomain(BMFGEN_WEBSITE);
    QCoreApplication::setApplicationName(BMFGEN_NAME);
    QCoreApplication::setApplicationVersion(
        QStringLiteral("%1.%2").arg(BMFGEN_VERSION_MAJOR).arg(BMFGEN_VERSION_MAJOR));

    QApplication app{argc, argv};

    QApplication::setWindowIcon(QIcon{":/icon512.png"});

    if (app.styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        QIcon::setFallbackSearchPaths(QStringList{} << ":/ui/icons/osx-dark");
        QIcon::setThemeName("osx-dark");
    }
    else
    {
        QIcon::setFallbackSearchPaths(QStringList{} << ":/ui/icons/osx-light");
        QIcon::setThemeName("osx-light");
    }

#ifdef _MSC_VER
    QApplication::setStyle("Fusion");
#endif

    MainWindow main_window;
    main_window.show();

    return QApplication::exec();
}
