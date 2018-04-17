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
#include "frame.h"
#include "kid3api.h"

/**
 * GUI related configuration.
 */
class KID3_CORE_EXPORT GuiConfig : public StoredConfig<GuiConfig> {
  Q_OBJECT
  /** sorted column in file list */
  Q_PROPERTY(int fileListSortColumn READ fileListSortColumn WRITE setFileListSortColumn NOTIFY fileListSortColumnChanged)
  /** sort order in file list */
  Q_PROPERTY(Qt::SortOrder fileListSortOrder READ fileListSortOrder WRITE setFileListSortOrder NOTIFY fileListSortOrderChanged)
  /** visible columns in file list */
  Q_PROPERTY(QList<int> fileListVisibleColumns READ fileListVisibleColumns WRITE setFileListVisibleColumns NOTIFY fileListVisibleColumnsChanged)
  /** sorted column in directory list */
  Q_PROPERTY(int dirListSortColumn READ dirListSortColumn WRITE setDirListSortColumn NOTIFY dirListSortColumnChanged)
  /** sort order in directory list */
  Q_PROPERTY(Qt::SortOrder dirListSortOrder READ dirListSortOrder WRITE setDirListSortOrder NOTIFY dirListSortOrderChanged)
  /** visible columns in directory list */
  Q_PROPERTY(QList<int> dirListVisibleColumns READ dirListVisibleColumns WRITE setDirListVisibleColumns NOTIFY dirListVisibleColumnsChanged)
  /** size of splitter in main window */
  Q_PROPERTY(QList<int> splitterSizes READ splitterSizes WRITE setSplitterSizes NOTIFY splitterSizesChanged)
  /** size of file/dirlist splitter */
  Q_PROPERTY(QList<int> vSplitterSizes READ vSplitterSizes WRITE setVSplitterSizes NOTIFY vSplitterSizesChanged)
  /** true to automatically hide unused tags */
  Q_PROPERTY(bool autoHideTags READ autoHideTags WRITE setAutoHideTags NOTIFY autoHideTagsChanged)
  /** true to hide file controls */
  Q_PROPERTY(bool hideFile READ hideFile WRITE setHideFile NOTIFY hideFileChanged)
  /** true to hide picture preview */
  Q_PROPERTY(bool hidePicture READ hidePicture WRITE setHidePicture NOTIFY hidePictureChanged)
  /** true to play file on double click */
  Q_PROPERTY(bool playOnDoubleClick READ playOnDoubleClick WRITE setPlayOnDoubleClick NOTIFY playOnDoubleClickChanged)
  /** config window geometry */
  Q_PROPERTY(QByteArray configWindowGeometry READ configWindowGeometry WRITE setConfigWindowGeometry NOTIFY configWindowGeometryChanged)

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

  /** Get sorted column in file list. */
  int fileListSortColumn() const { return m_fileListSortColumn; }

  /** Set sorted column in file list. */
  void setFileListSortColumn(int fileListSortColumn);

  /** Get sort order in file list. */
  Qt::SortOrder fileListSortOrder() const { return m_fileListSortOrder; }

  /** Set sort order in file list. */
  void setFileListSortOrder(Qt::SortOrder fileListSortOrder);

  /** Get visible columns in file list. */
  QList<int> fileListVisibleColumns() const { return m_fileListVisibleColumns; }

  /** Set visible columns in file list. */
  void setFileListVisibleColumns(const QList<int>& fileListVisibleColumns);

  /** Get sorted column in directory list. */
  int dirListSortColumn() const { return m_dirListSortColumn; }

  /** Set sorted column in directory list. */
  void setDirListSortColumn(int dirListSortColumn);

  /** Get sort order in directory list. */
  Qt::SortOrder dirListSortOrder() const { return m_dirListSortOrder; }

  /** Set sort order in directory list. */
  void setDirListSortOrder(Qt::SortOrder dirListSortOrder);

  /** Get visible columns in directory list. */
  QList<int> dirListVisibleColumns() const { return m_dirListVisibleColumns; }

  /** Set visible columns in directory list. */
  void setDirListVisibleColumns(const QList<int>& dirListVisibleColumns);

  /** Get size of splitter in main window. */
  QList<int> splitterSizes() const { return m_splitterSizes; }

  /** Set size of splitter in main window. */
  void setSplitterSizes(const QList<int>& splitterSizes);

  /** Get size of file/dirlist splitter. */
  QList<int> vSplitterSizes() const { return m_vSplitterSizes; }

  /** Set size of file/dirlist splitter. */
  void setVSplitterSizes(const QList<int>& vSplitterSizes);

  /** Check if unused tags are automatically hidden. */
  bool autoHideTags() const { return m_autoHideTags; }

  /** Set if unused tags are automatically hidden. */
  void setAutoHideTags(bool autoHideTags);

  /** Check if file controls are hidden. */
  bool hideFile() const { return m_hideFile; }

  /** Set if file controls are hidden. */
  void setHideFile(bool hideFile);

  /** Check if tag controls are hidden. */
  bool hideTag(Frame::TagNumber tagNr) const { return m_hideTag[tagNr]; }

  /** Set if tag controls are hidden. */
  void setHideTag(Frame::TagNumber tagNr, bool hide);

  /** Check if the picture preview is hidden. */
  bool hidePicture() const { return m_hidePicture; }

  /** Set if the picture preview is hidden. */
  void setHidePicture(bool hidePicture);

  /** Check if play file on double click is enabled. */
  bool playOnDoubleClick() const { return m_playOnDoubleClick; }

  /** Set if play file on double click is enabled. */
  void setPlayOnDoubleClick(bool playOnDoubleClick);

  /** Get config window geometry. */
  QByteArray configWindowGeometry() const { return m_configWindowGeometry; }

  /** Set import window geometry. */
  void setConfigWindowGeometry(const QByteArray& configWindowGeometry);

signals:
  /** Emitted when @a fileListSortColumn changed. */
  void fileListSortColumnChanged(int fileListSortColumn);

  /** Emitted when @a fileListSortOrder changed. */
  void fileListSortOrderChanged(Qt::SortOrder fileListSortOrder);

  /** Emitted when @a fileListVisibleColumns changed. */
  void fileListVisibleColumnsChanged(const QList<int>& fileListVisibleColumns);

  /** Emitted when @a dirListSortColumn changed. */
  void dirListSortColumnChanged(int dirListSortColumn);

  /** Emitted when @a dirListSortOrder changed. */
  void dirListSortOrderChanged(Qt::SortOrder dirListSortOrder);

  /** Emitted when @a dirListVisibleColumns changed. */
  void dirListVisibleColumnsChanged(const QList<int>& dirListVisibleColumns);

  /** Emitted when @a splitterSizes changed. */
  void splitterSizesChanged(const QList<int>& splitterSizes);

  /** Emitted when @a vSplitterSizes changed. */
  void vSplitterSizesChanged(const QList<int>& vSplitterSizes);

  /** Emitted when @a autoHideTags changed. */
  void autoHideTagsChanged(bool autoHideTags);

  /** Emitted when @a hideFile changed. */
  void hideFileChanged(bool hideFile);

  /** Emitted when @a hideTag changed. */
  void hideTagChanged();

  /** Emitted when @a hidePicture changed. */
  void hidePictureChanged(bool hidePicture);

  /** Emitted when @a playOnDoubleClick changed. */
  void playOnDoubleClickChanged(bool playOnDoubleClick);

  /** Emitted when @a configWindowGeometry changed. */
  void configWindowGeometryChanged(const QByteArray& configWindowGeometry);

private:
  friend GuiConfig& StoredConfig<GuiConfig>::instance();

  int m_fileListSortColumn;
  Qt::SortOrder m_fileListSortOrder;
  QList<int> m_fileListVisibleColumns;
  int m_dirListSortColumn;
  Qt::SortOrder m_dirListSortOrder;
  QList<int> m_dirListVisibleColumns;
  QList<int> m_splitterSizes;
  QList<int> m_vSplitterSizes;
  QByteArray m_configWindowGeometry;
  bool m_autoHideTags;
  bool m_hideFile;
  bool m_hideTag[Frame::Tag_NumValues];
  bool m_hidePicture;
  bool m_playOnDoubleClick;

  /** Index in configuration storage */
  static int s_index;
};

#endif
