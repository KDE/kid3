/**
 * \file fileconfig.cpp
 * File related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2017  Urs Fleisch
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
#include <QCoreApplication>

int FileConfig::s_index = -1;

namespace {

/** Default to filename format list */
const char* defaultToFilenameFormats[] = {
  "%{track} %{title}",
  "%{track}. %{title}",
  "%{track} - %{artist} - %{title}",
  "%{track}. %{artist} - %{title}",
  "%{artist} - %{track} - %{title}",
  "%{artist} - %{album} - %{track} - %{title}",
 R"(%{artist} - %{"["year"] "}%{album} - %{track} - %{title})",
  "%{artist} - %{title}",
  "%{artist}-%{title}",
  "(%{artist}) %{title}",
  "%{artist}-%{title}-%{album}",
  nullptr
};

/** Default from filename format list */
const char* defaultFromFilenameFormats[] = {
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
  nullptr
};

}

/**
 * Constructor.
 */
FileConfig::FileConfig() :
  StoredConfig<FileConfig>(QLatin1String("Files")),
  m_nameFilter(QLatin1String("")),
  m_formatText(QString::fromLatin1(defaultToFilenameFormats[0])),
  m_formatItem(0),
  m_formatFromFilenameText(QString::fromLatin1(defaultFromFilenameFormats[0])),
  m_formatFromFilenameItem(0),
  m_defaultCoverFileName(QLatin1String("folder.jpg")),
  m_textEncoding(QLatin1String("System")),
  m_preserveTime(false),
  m_markChanges(true),
  m_loadLastOpenedFile(true),
  m_showHiddenFiles(false)
{
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void FileConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("NameFilter"), QVariant(m_nameFilter));
  config->setValue(QLatin1String("IncludeFolders"), QVariant(m_includeFolders));
  config->setValue(QLatin1String("ExcludeFolders"), QVariant(m_excludeFolders));
  config->setValue(QLatin1String("ShowHiddenFiles"), QVariant(m_showHiddenFiles));
  config->setValue(QLatin1String("FormatItem"), QVariant(m_formatItem));
  config->setValue(QLatin1String("FormatItems"), QVariant(m_formatItems));
  config->setValue(QLatin1String("FormatText"), QVariant(m_formatText));
  config->setValue(QLatin1String("FormatFromFilenameItem"), QVariant(m_formatFromFilenameItem));
  config->setValue(QLatin1String("FormatFromFilenameItems"), QVariant(m_formatFromFilenameItems));
  config->setValue(QLatin1String("FormatFromFilenameText"), QVariant(m_formatFromFilenameText));
  config->setValue(QLatin1String("PreserveTime"), QVariant(m_preserveTime));
  config->setValue(QLatin1String("MarkChanges"), QVariant(m_markChanges));
  config->setValue(QLatin1String("LoadLastOpenedFile"), QVariant(m_loadLastOpenedFile));
  config->setValue(QLatin1String("TextEncoding"), QVariant(m_textEncoding));
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
  m_includeFolders =
      config->value(QLatin1String("IncludeFolders"), m_includeFolders).toStringList();
  m_excludeFolders =
      config->value(QLatin1String("ExcludeFolders"), m_excludeFolders).toStringList();
  m_showHiddenFiles = config->value(QLatin1String("ShowHiddenFiles"), m_showHiddenFiles).toBool();
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
  m_textEncoding = config->value(QLatin1String("TextEncoding"), QLatin1String("System")).toString();
  m_lastOpenedFile = config->value(QLatin1String("LastOpenedFile"), m_lastOpenedFile).toString();
  m_defaultCoverFileName = config->value(QLatin1String("DefaultCoverFileName"), m_defaultCoverFileName).toString();
  config->endGroup();

  if (m_formatItems.isEmpty()) {
    for (const char** sl = defaultToFilenameFormats; *sl != nullptr; ++sl) {
      m_formatItems += QString::fromLatin1(*sl);
    }
  }
  if (m_formatFromFilenameItems.isEmpty()) {
    for (const char** sl = defaultFromFilenameFormats; *sl != nullptr; ++sl) {
      m_formatFromFilenameItems += QString::fromLatin1(*sl);
    }
  }
  if (ConfigStore::getConfigVersion() < 4) {
    // Reset file name filter if it is set to "All Supported Files" in order
    // to introduce newly supported file formats (e.g. *.dsf) when the
    // configuration version is increased.
    if (m_nameFilter.startsWith(QCoreApplication::translate(
                                  "Kid3Application", "All Supported Files"))) {
      m_nameFilter.clear();
    }
  }
}

void FileConfig::setNameFilter(const QString& nameFilter)
{
  if (m_nameFilter != nameFilter) {
    m_nameFilter = nameFilter;
    emit nameFilterChanged(m_nameFilter);
  }
}

void FileConfig::setIncludeFolders(const QStringList& includeFolders)
{
  if (m_includeFolders != includeFolders) {
    m_includeFolders = includeFolders;
    emit includeFoldersChanged(m_includeFolders);
  }
}

void FileConfig::setExcludeFolders(const QStringList& excludeFolders)
{
  if (m_excludeFolders != excludeFolders) {
    m_excludeFolders = excludeFolders;
    emit excludeFoldersChanged(m_excludeFolders);
  }
}

void FileConfig::setShowHiddenFiles(bool showHiddenFiles)
{
  if (m_showHiddenFiles != showHiddenFiles) {
    m_showHiddenFiles = showHiddenFiles;
    emit showHiddenFilesChanged(m_showHiddenFiles);
  }
}

void FileConfig::setToFilenameFormat(const QString& formatText)
{
  if (m_formatText != formatText) {
    m_formatText = formatText;
    emit toFilenameFormatChanged(m_formatText);
  }
}

void FileConfig::setToFilenameFormatIndex(int formatItem)
{
  if (m_formatItem != formatItem) {
    m_formatItem = formatItem;
    emit toFilenameFormatIndexChanged(m_formatItem);
  }
}

void FileConfig::setToFilenameFormats(const QStringList& formatItems)
{
  if (m_formatItems != formatItems) {
    m_formatItems = formatItems;
    emit toFilenameFormatsChanged(m_formatItems);
  }
}

void FileConfig::setFromFilenameFormat(const QString& formatFromFilenameText)
{
  if (m_formatFromFilenameText != formatFromFilenameText) {
    m_formatFromFilenameText = formatFromFilenameText;
    emit fromFilenameFormatChanged(m_formatFromFilenameText);
  }
}

void FileConfig::setFromFilenameFormatIndex(int formatFromFilenameItem)
{
  if (m_formatFromFilenameItem != formatFromFilenameItem) {
    m_formatFromFilenameItem = formatFromFilenameItem;
    emit fromFilenameFormatIndexChanged(m_formatFromFilenameItem);
  }
}

void FileConfig::setFromFilenameFormats(const QStringList& formatFromFilenameItems)
{
  if (m_formatFromFilenameItems != formatFromFilenameItems) {
    m_formatFromFilenameItems = formatFromFilenameItems;
    emit fromFilenameFormatsChanged(m_formatFromFilenameItems);
  }
}

void FileConfig::setDefaultCoverFileName(const QString& defaultCoverFileName)
{
  if (m_defaultCoverFileName != defaultCoverFileName) {
    m_defaultCoverFileName = defaultCoverFileName;
    emit defaultCoverFileNameChanged(m_defaultCoverFileName);
  }
}

void FileConfig::setLastOpenedFile(const QString& lastOpenedFile)
{
  if (m_lastOpenedFile != lastOpenedFile) {
    m_lastOpenedFile = lastOpenedFile;
    emit lastOpenedFileChanged(m_lastOpenedFile);
  }
}

void FileConfig::setTextEncoding(const QString& textEncoding)
{
  if (m_textEncoding != textEncoding) {
    m_textEncoding = textEncoding;
    emit textEncodingChanged(m_textEncoding);
  }
}

int FileConfig::textEncodingIndex() const
{
  return indexFromTextCodecName(m_textEncoding);
}

void FileConfig::setTextEncodingIndex(int index)
{
  QString encoding = indexToTextCodecName(index);
  if (!encoding.isNull()) {
    setTextEncoding(encoding);
  }
}

void FileConfig::setPreserveTime(bool preserveTime)
{
  if (m_preserveTime != preserveTime) {
    m_preserveTime = preserveTime;
    emit preserveTimeChanged(m_preserveTime);
  }
}

void FileConfig::setMarkChanges(bool markChanges)
{
  if (m_markChanges != markChanges) {
    m_markChanges = markChanges;
    emit markChangesChanged(m_markChanges);
  }
}

void FileConfig::setLoadLastOpenedFile(bool loadLastOpenedFile)
{
  if (m_loadLastOpenedFile != loadLastOpenedFile) {
    m_loadLastOpenedFile = loadLastOpenedFile;
    emit loadLastOpenedFileChanged(m_loadLastOpenedFile);
  }
}
