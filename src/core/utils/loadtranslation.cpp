/**
 * \file loadtranslation.cpp
 * Load application translation.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Mar 2013
 *
 * Copyright (C) 2013  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loadtranslation.h"
#include <QCoreApplication>
#include <QStringList>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include "config.h"

/**
 * @brief Load application translation.
 *
 * @param lang preferred language, if not set, the language is determined by
 * the system configuration
 */
void Utils::loadTranslation(const QString& lang)
{
  QLocale locale;

  QStringList languages(
#if QT_VERSION >= 0x040800 && !defined Q_OS_WIN32
        locale.uiLanguages()
#else
        locale.name()
#endif
        );
  if (!lang.isEmpty()) {
    languages.prepend(lang);
  }

  // translation file for Qt
  QTranslator* qtTr = new QTranslator(qApp);
  foreach (QString localeName, languages) {
    if (
        localeName.startsWith(QLatin1String("en")) ||
#if defined Q_OS_WIN32 || defined Q_OS_MAC
#ifdef CFG_TRANSLATIONSDIR
        qtTr->load(QLatin1String("qt_") + localeName,
                   QLatin1String(CFG_TRANSLATIONSDIR)) ||
#endif
        qtTr->load(QLatin1String("qt_") + localeName, QLatin1String("."))
#else
        qtTr->load(QLatin1String("qt_") + localeName,
                   QLibraryInfo::location(QLibraryInfo::TranslationsPath))
#endif
        ) {
      break;
    }
  }
  qApp->installTranslator(qtTr);

  // translation file for application strings
  QTranslator* kid3Tr = new QTranslator(qApp);
  foreach (QString localeName, languages) {
    if (
        localeName.startsWith(QLatin1String("en")) ||
#ifdef CFG_TRANSLATIONSDIR
        kid3Tr->load(QLatin1String("kid3_") + localeName,
                     QLatin1String(CFG_TRANSLATIONSDIR)) ||
#endif
        kid3Tr->load(QLatin1String("kid3_") + localeName, QLatin1String("."))
        ) {
      break;
    }
  }
  qApp->installTranslator(kid3Tr);
}
