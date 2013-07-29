/**
 * \file fileconfig.cpp
 * File related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
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

#include "fileconfig.h"

int FileConfig::s_index = -1;

/** Default to filename format list */
static const char* defaultToFilenameFormats[] = {
  "%{track} %{title}",
  "%{track}. %{title}",
  "%{track} - %{artist} - %{title}",
  "%{track}. %{artist} - %{title}",
  "%{artist} - %{track} - %{title}",
  "%{artist} - %{album} - %{track} - %{title}",
  "%{artist} - [%{year}] %{album} - %{track} - %{title}",
  "%{artist} - %{title}",
  "%{artist}-%{title}",
  "(%{artist}) %{title}",
  "%{artist}-%{title}-%{album}",
  0
};

/** Default from filename format list */
static const char* defaultFromFilenameFormats[] = {
  "%{artist} - %{album}/%{track} %{title}",
  "%{artist} - %{album}/%{track}. %{title}",
  "%{artist} - [%{year}] %{album}/%{track} %{title}",
  "%{artist} - [%{year}] %{album}/%{track}. %{title}",
  "%{artist} - %{album} (%{year})/%{track} - %{title}",
  "%{artist}/%{album}/%{track} %{title}",
  "%{artist}/%{album}/%{track}. %{title}",
  "%{artist}/[%{year}] %{album}/%{track} %{title}",
  "%{artist}/[%{year}] %{album}/%{track}. %{title}",
  "%{album}/%{track} - %{artist} - %{title}",
  "%{album}/%{track}. %{artist} - %{title}",
  "%{album}/%{artist} - %{track} - %{title}",
  "[%{year}] %{album}/%{track} - %{artist} - %{title}",
  "%{artist} - %{album} - %{track} - %{title}",
  "%{artist} - [%{year}] %{album} - %{track} - %{title}",
  "%{album}/%{artist} - %{track} - %{title}",
  "[%{year}] %{album}/%{artist} - %{track} - %{title}",
  "%{album}/%{artist} - %{title}",
  "%{album}/%{artist}-%{title}",
  "%{album}/(%{artist}) %{title}",
  "%{artist}-%{title}-%{album}",
  0
};

/**
 * Constructor.
 */
FileConfig::FileConfig() :
  StoredConfig<FileConfig>(QLatin1String("Files")),
  m_preserveTime(false),
  m_markChanges(true),
  m_nameFilter(QLatin1String("")),
  m_formatText(QString::fromLatin1(defaultToFilenameFormats[0])),
  m_formatItem(0),
  m_formatFromFilenameText(QString::fromLatin1(defaultFromFilenameFormats[0])),
  m_formatFromFilenameItem(0),
  m_defaultCoverFileName(QLatin1String("folder.jpg")),
  m_loadLastOpenedFile(false)
{
}

/**
 * Destructor.
 */
FileConfig::~FileConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void FileConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("NameFilter"), QVariant(m_nameFilter));
  config->setValue(QLatin1String("FormatItem"), QVariant(m_formatItem));
  config->setValue(QLatin1String("FormatItems"), QVariant(m_formatItems));
  config->setValue(QLatin1String("FormatText"), QVariant(m_formatText));
  config->setValue(QLatin1String("FormatFromFilenameItem"), QVariant(m_formatFromFilenameItem));
  config->setValue(QLatin1String("FormatFromFilenameItems"), QVariant(m_formatFromFilenameItems));
  config->setValue(QLatin1String("FormatFromFilenameText"), QVariant(m_formatFromFilenameText));
  config->setValue(QLatin1String("PreserveTime"), QVariant(m_preserveTime));
  config->setValue(QLatin1String("MarkChanges"), QVariant(m_markChanges));
  config->setValue(QLatin1String("LoadLastOpenedFile"), QVariant(m_loadLastOpenedFile));
  config->setValue(QLatin1String("LastOpenedFile"), QVariant(m_lastOpenedFile));
  config->setValue(QLatin1String("DefaultCoverFileName"), QVariant(m_defaultCoverFileName));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void FileConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_nameFilter =
      config->value(QLatin1String("NameFilter"), QLatin1String("")).toString();
  m_formatItem =
      config->value(QLatin1String("FormatItem"), 0).toInt();
  m_formatItems =
      config->value(QLatin1String("FormatItems"),
                    m_formatItems).toStringList();
  m_formatFromFilenameItem =
      config->value(QLatin1String("FormatFromFilenameItem"), 0).toInt();
  m_formatFromFilenameItems =
      config->value(QLatin1String("FormatFromFilenameItems"),
                    m_formatFromFilenameItems).toStringList();
  m_preserveTime = config->value(QLatin1String("PreserveTime"), m_preserveTime).toBool();
  m_markChanges = config->value(QLatin1String("MarkChanges"), m_markChanges).toBool();

  m_formatText =
      config->value(QLatin1String("FormatText"), QString::fromLatin1(defaultToFilenameFormats[0])).toString();
  m_formatFromFilenameText =
      config->value(QLatin1String("FormatFromFilenameText"), QString::fromLatin1(defaultFromFilenameFormats[0])).toString();
  m_loadLastOpenedFile = config->value(QLatin1String("LoadLastOpenedFile"), m_loadLastOpenedFile).toBool();
  m_lastOpenedFile = config->value(QLatin1String("LastOpenedFile"), m_lastOpenedFile).toString();
  m_defaultCoverFileName = config->value(QLatin1String("DefaultCoverFileName"), m_defaultCoverFileName).toString();
  config->endGroup();

  if (m_formatItems.isEmpty()) {
    for (const char** sl = defaultToFilenameFormats; *sl != 0; ++sl) {
      m_formatItems += QString::fromLatin1(*sl);
    }
  }
  if (m_formatFromFilenameItems.isEmpty()) {
    for (const char** sl = defaultFromFilenameFormats; *sl != 0; ++sl) {
      m_formatFromFilenameItems += QString::fromLatin1(*sl);
    }
  }
}
