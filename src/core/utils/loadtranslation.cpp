/**
 * \file loadtranslation.cpp
 * Load application translation.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Mar 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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
#include <QFileInfo>
#include "config.h"

namespace {

const char* const QT_TRANSLATION_PREFIX = "qtbase_";

}

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
#ifndef Q_OS_WIN32
        locale.uiLanguages()
#else
        locale.name()
#endif
        );
  if (!lang.isEmpty()) {
    languages.prepend(lang);
  }

  QString translationsDir;
#ifdef CFG_TRANSLATIONSDIR
  translationsDir = QLatin1String(CFG_TRANSLATIONSDIR);
  prependApplicationDirPathIfRelative(translationsDir);
#endif

  // '-' is added to default delimiters because it is used on Mac OS X instead
  // of '_'.
  const QString searchDelimiters(QLatin1String("_.-"));

  // translation file for Qt
  auto qtTr = new QTranslator(qApp);
  const auto localeNames = languages;
  for (const QString& localeName : localeNames) {
    if (
        localeName.startsWith(QLatin1String("en")) ||
        (!translationsDir.isNull() &&
         qtTr->load(QLatin1String(QT_TRANSLATION_PREFIX) + localeName,
                    translationsDir, searchDelimiters)) ||
        qtTr->load(QLatin1String(QT_TRANSLATION_PREFIX) + localeName,
                   QLatin1String("."), searchDelimiters)
#if !(defined Q_OS_WIN32 || defined Q_OS_MAC || defined Q_OS_ANDROID)
        || qtTr->load(QLatin1String(QT_TRANSLATION_PREFIX) + localeName,
                   QLibraryInfo::location(QLibraryInfo::TranslationsPath),
                   searchDelimiters)
#endif
        ) {
      break;
    }
  }
  qApp->installTranslator(qtTr);

  // translation file for application strings
  auto kid3Tr = new QTranslator(qApp);
  for (const QString& localeName : localeNames) {
    if (
        (!translationsDir.isNull() &&
         kid3Tr->load(QLatin1String("kid3_") + localeName, translationsDir,
                      searchDelimiters)) ||
        kid3Tr->load(QLatin1String("kid3_") + localeName, QLatin1String("."),
                     searchDelimiters) ||
        localeName.startsWith(QLatin1String("en"))
        ) {
      break;
    }
  }
  qApp->installTranslator(kid3Tr);
}

/**
 * Prepend the application directory path to a path if it is relative.
 *
 * @param path file or directory path, will be modified if relative
 */
void Utils::prependApplicationDirPathIfRelative(QString& path)
{
  if (QFileInfo(path).isRelative()) {
    QString appDir = QCoreApplication::applicationDirPath();
    if (!appDir.isEmpty()) {
      if (!appDir.endsWith(QLatin1Char('/'))) {
        appDir.append(QLatin1Char('/'));
      }
      path.prepend(appDir);
    }
  }
}
