/**
 * \file fileconfig.h
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

#ifndef FILECONFIG_H
#define FILECONFIG_H

#include <QStringList>
#include "generalconfig.h"
#include "kid3api.h"

/**
 * File related configuration.
 */
class KID3_CORE_EXPORT FileConfig : public StoredConfig<FileConfig>
{
public:
  /**
   * Constructor.
   */
  FileConfig();

  /**
   * Destructor.
   */
  virtual ~FileConfig();

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config);

  /** filter of file names to be opened */
  QString m_nameFilter;
  /** filename format */
  QString m_formatText;
  /** index of filename format selected */
  int m_formatItem;
  /** filename formats */
  QStringList m_formatItems;
  /** from filename format */
  QString m_formatFromFilenameText;
  /** index of from filename format selected */
  int m_formatFromFilenameItem;
  /** from filename formats */
  QStringList m_formatFromFilenameItems;
  /** default file name to save cover art */
  QString m_defaultCoverFileName;
  /** path to last opened file */
  QString m_lastOpenedFile;
  /** true to preserve file time stamps */
  bool m_preserveTime;
  /** true to mark changed fields */
  bool m_markChanges;
  /** true to open last opened file on startup */
  bool m_loadLastOpenedFile;

  /** Index in configuration storage */
  static int s_index;
};

#endif
