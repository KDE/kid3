/**
 * \file guiconfig.h
 * GUI related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2014  Urs Fleisch
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

#ifndef GUICONFIG_H
#define GUICONFIG_H

#include "generalconfig.h"
#include "kid3api.h"

/**
 * GUI related configuration.
 */
class KID3_CORE_EXPORT GuiConfig : public StoredConfig<GuiConfig>
{
public:
  /**
   * Constructor.
   */
  GuiConfig();

  /**
   * Destructor.
   */
  virtual ~GuiConfig();

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

  /** true to automatically hide unused tags */
  bool m_autoHideTags;
  /** true to hide file controls */
  bool m_hideFile;
  /** true to hide ID3v1.1 controls */
  bool m_hideV1;
  /** true to hide ID3v2.3 controls */
  bool m_hideV2;
  /** true to hide picture preview */
  bool m_hidePicture;
  /** true to play file on double click */
  bool m_playOnDoubleClick;
  /** sorted column in file list */
  int m_fileListSortColumn;
  /** sort order in file list */
  Qt::SortOrder m_fileListSortOrder;
  /** visible columns in file list */
  QList<int> m_fileListVisibleColumns;
  /** sorted column in directory list */
  int m_dirListSortColumn;
  /** sort order in directory list */
  Qt::SortOrder m_dirListSortOrder;
  /** visible columns in directory list */
  QList<int> m_dirListVisibleColumns;
  /** size of splitter in main window */
  QList<int> m_splitterSizes;
  /** size of file/dirlist splitter */
  QList<int> m_vSplitterSizes;

  /** Index in configuration storage */
  static int s_index;
};

#endif
