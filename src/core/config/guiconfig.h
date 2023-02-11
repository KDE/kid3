/**
 * \file guiconfig.h
 * GUI related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
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

#pragma once

#include "generalconfig.h"
#include "frame.h"
#include "kid3api.h"

/**
 * GUI related configuration.
 */
class KID3_CORE_EXPORT GuiConfig : public StoredConfig<GuiConfig> {
  Q_OBJECT
  /** sorted column in file list */
  Q_PROPERTY(int fileListSortColumn READ fileListSortColumn
             WRITE setFileListSortColumn NOTIFY fileListSortColumnChanged)
  /** sort order in file list */
  Q_PROPERTY(Qt::SortOrder fileListSortOrder READ fileListSortOrder
             WRITE setFileListSortOrder NOTIFY fileListSortOrderChanged)
  /** visible columns in file list */
  Q_PROPERTY(QList<int> fileListVisibleColumns READ fileListVisibleColumns
             WRITE setFileListVisibleColumns NOTIFY fileListVisibleColumnsChanged)
  /** true to enable custom column widths in file list */
  Q_PROPERTY(bool fileListCustomColumnWidthsEnabled
             READ fileListCustomColumnWidthsEnabled
             WRITE setFileListCustomColumnWidthsEnabled
             NOTIFY fileListCustomColumnWidthsEnabledChanged)
  /** column widths in file list */
  Q_PROPERTY(QList<int> fileListColumnWidths READ fileListColumnWidths
             WRITE setFileListColumnWidths NOTIFY fileListColumnWidthsChanged)
  /** sorted column in directory list */
  Q_PROPERTY(int dirListSortColumn READ dirListSortColumn
             WRITE setDirListSortColumn NOTIFY dirListSortColumnChanged)
  /** sort order in directory list */
  Q_PROPERTY(Qt::SortOrder dirListSortOrder READ dirListSortOrder
             WRITE setDirListSortOrder NOTIFY dirListSortOrderChanged)
  /** visible columns in directory list */
  Q_PROPERTY(QList<int> dirListVisibleColumns READ dirListVisibleColumns
             WRITE setDirListVisibleColumns NOTIFY dirListVisibleColumnsChanged)
  /** true to enable custom column widths in directory list */
  Q_PROPERTY(bool dirListCustomColumnWidthsEnabled
             READ dirListCustomColumnWidthsEnabled
             WRITE setDirListCustomColumnWidthsEnabled
             NOTIFY dirListCustomColumnWidthsEnabledChanged)
  /** column widths in directory list */
  Q_PROPERTY(QList<int> dirListColumnWidths READ dirListColumnWidths
             WRITE setDirListColumnWidths NOTIFY dirListColumnWidthsChanged)
  /** size of splitter in main window */
  Q_PROPERTY(QList<int> splitterSizes READ splitterSizes
             WRITE setSplitterSizes NOTIFY splitterSizesChanged)
  /** size of file/dirlist splitter */
  Q_PROPERTY(QList<int> vSplitterSizes READ vSplitterSizes
             WRITE setVSplitterSizes NOTIFY vSplitterSizesChanged)
  /** true to automatically hide unused tags */
  Q_PROPERTY(bool autoHideTags READ autoHideTags WRITE setAutoHideTags
             NOTIFY autoHideTagsChanged)
  /** true to hide file controls */
  Q_PROPERTY(bool hideFile READ hideFile WRITE setHideFile
             NOTIFY hideFileChanged)
  /** true to hide picture preview */
  Q_PROPERTY(bool hidePicture READ hidePicture WRITE setHidePicture
             NOTIFY hidePictureChanged)
  /** true to play file on double click */
  Q_PROPERTY(bool playOnDoubleClick READ playOnDoubleClick
             WRITE setPlayOnDoubleClick NOTIFY playOnDoubleClickChanged)
  /** true to select file on play */
  Q_PROPERTY(bool selectFileOnPlayEnabled READ selectFileOnPlayEnabled
             WRITE setSelectFileOnPlayEnabled NOTIFY selectFileOnPlayEnabledChanged)
  /** true if play tool bar is visible */
  Q_PROPERTY(bool playToolBarVisible READ playToolBarVisible
             WRITE setPlayToolBarVisible NOTIFY playToolBarVisibleChanged)
  /** play tool bar docked area */
  Q_PROPERTY(int playToolBarArea READ playToolBarArea
             WRITE setPlayToolBarArea NOTIFY playToolBarAreaChanged)
  /** config window geometry */
  Q_PROPERTY(QByteArray configWindowGeometry READ configWindowGeometry
             WRITE setConfigWindowGeometry NOTIFY configWindowGeometryChanged)

public:
  /**
   * Constructor.
   */
  GuiConfig();

  /**
   * Destructor.
   */
  virtual ~GuiConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config) override;

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

  /** Check if custom column widths are enabled in file list. */
  bool fileListCustomColumnWidthsEnabled() const {
    return m_fileListCustomColumnWidthsEnabled;
  }

  /** Set if custom column widths are enabled in file list. */
  void setFileListCustomColumnWidthsEnabled(bool enable);

  /** Get column widths in file list. */
  QList<int> fileListColumnWidths() const { return m_fileListColumnWidths; }

  /** Set column widths in file list. */
  void setFileListColumnWidths(const QList<int>& fileListColumnWidths);

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

  /** Check if custom column widths are enabled in directory list. */
  bool dirListCustomColumnWidthsEnabled() const {
    return m_dirListCustomColumnWidthsEnabled;
  }

  /** Set if custom column widths are enabled in directory list. */
  void setDirListCustomColumnWidthsEnabled(bool enable);

  /** Get column widths in directory list. */
  QList<int> dirListColumnWidths() const { return m_dirListColumnWidths; }

  /** Set column widths in directory list. */
  void setDirListColumnWidths(const QList<int>& dirListColumnWidths);

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

  /** Check if select file on play is enabled. */
  bool selectFileOnPlayEnabled() const { return m_selectFileOnPlayEnabled; }

  /** Enable select file on play. */
  void setSelectFileOnPlayEnabled(bool selectFileOnPlayEnabled);

  /** Check if play tool bar is visible. */
  bool playToolBarVisible() const { return m_playToolBarVisible; }

  /** Set if play tool bar is visible. */
  void setPlayToolBarVisible(bool playToolBarVisible);

  /** Get play tool bar docked area. */
  int playToolBarArea() const { return m_playToolBarArea; }

  /** Set play tool bar docked area. */
  void setPlayToolBarArea(int playToolBarArea);

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

  /** Emitted when @a fileListCustomColumnWidthsEnabled changed. */
  void fileListCustomColumnWidthsEnabledChanged(bool fileListCustomColumnWidthsEnabled);

  /** Emitted when @a fileListColumnWidths changed. */
  void fileListColumnWidthsChanged(const QList<int>& fileListColumnWidths);

  /** Emitted when @a dirListSortColumn changed. */
  void dirListSortColumnChanged(int dirListSortColumn);

  /** Emitted when @a dirListSortOrder changed. */
  void dirListSortOrderChanged(Qt::SortOrder dirListSortOrder);

  /** Emitted when @a dirListVisibleColumns changed. */
  void dirListVisibleColumnsChanged(const QList<int>& dirListVisibleColumns);

  /** Emitted when @a dirListCustomColumnWidthsEnabled changed. */
  void dirListCustomColumnWidthsEnabledChanged(bool dirListCustomColumnWidthsEnabled);

  /** Emitted when @a dirListColumnWidths changed. */
  void dirListColumnWidthsChanged(const QList<int>& dirListColumnWidths);

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

  /** Emitted when @a selectFileOnPlayEnabled changed. */
  void selectFileOnPlayEnabledChanged(bool selectFileOnPlayEnabled);

  /** Emitted when @a playToolBarVisible changed. */
  void playToolBarVisibleChanged(bool playToolBarVisible);

  /** Emitted when @a playToolBarArea changed. */
  void playToolBarAreaChanged(int playToolBarArea);

  /** Emitted when @a configWindowGeometry changed. */
  void configWindowGeometryChanged(const QByteArray& configWindowGeometry);

private:
  friend GuiConfig& StoredConfig<GuiConfig>::instance();

  int m_fileListSortColumn;
  Qt::SortOrder m_fileListSortOrder;
  QList<int> m_fileListVisibleColumns;
  QList<int> m_fileListColumnWidths;
  int m_dirListSortColumn;
  Qt::SortOrder m_dirListSortOrder;
  QList<int> m_dirListVisibleColumns;
  QList<int> m_dirListColumnWidths;
  QList<int> m_splitterSizes;
  QList<int> m_vSplitterSizes;
  QByteArray m_configWindowGeometry;
  int m_playToolBarArea;
  bool m_autoHideTags;
  bool m_hideFile;
  bool m_hideTag[Frame::Tag_NumValues];
  bool m_hidePicture;
  bool m_playOnDoubleClick;
  bool m_selectFileOnPlayEnabled;
  bool m_playToolBarVisible;
  bool m_fileListCustomColumnWidthsEnabled;
  bool m_dirListCustomColumnWidthsEnabled;

  /** Index in configuration storage */
  static int s_index;
};
