/**
 * \file filelist.h
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#ifndef FILELIST_H
#define FILELIST_H

#include <QTreeView>
#include <QList>
#include "config.h"

class BaseMainWindowImpl;
class ExternalProcess;

/**
 * List of files to operate on.
 */
class FileList : public QTreeView {
Q_OBJECT

public:
  /**
   * Constructor.
   * @param parent parent widget
   * @param mainWin main window
   */
  FileList(QWidget* parent, BaseMainWindowImpl* mainWin);

  /**
   * Destructor.
   */
  virtual ~FileList();

  /**
   * Returns the recommended size for the widget.
   * @return recommended size.
   */
  virtual QSize sizeHint() const;

  /**
   * Fill the filelist with the files found in a directory.
   *
   * @param dirIndex index of directory in filesystem model
   * @param fileIndex index of file to select in filesystem model (optional,
   * else invalid)
   *
   * @return false if name is not directory path, else true.
   */
  bool readDir(const QModelIndex& dirIndex,
               const QModelIndex& fileIndex=QModelIndex());

  /**
   * Get the stored current selection.
   * @return stored selection.
   */
  const QList<QPersistentModelIndex>& getCurrentSelection() const {
    return m_currentSelection;
  }

  /**
   * Clear the stored current selection.
   */
  void clearCurrentSelection() { m_currentSelection.clear(); }

  /**
   * Update the stored current selection with the list of all selected items.
   */
  void updateCurrentSelection();

  /**
   * Get current index or root index if current index is invalid.
   * @return current index, root index if not valid.
   */
  QModelIndex currentOrRootIndex() const {
    return currentIndex().isValid() ? currentIndex() : rootIndex();
  }

private slots:
  /**
   * Display a context menu with operations for selected files.
   *
   * @param index index of item
   * @param pos   position where context menu is drawn on screen
   */
  void contextMenu(const QModelIndex& index, const QPoint& pos);

  /**
   * Execute a context menu command.
   *
   * @param id command ID
   */
  void executeContextCommand(int id);

  /**
   * Execute a context menu action.
   *
   * @param action action of selected menu
   */
  void executeAction(QAction* action);

  /**
   * Display a custom context menu with operations for selected files.
   *
   * @param pos  position where context menu is drawn on screen
   */
  void customContextMenu(const QPoint& pos);

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /**
   * Play item if it is a tagged file.
   *
   * @param index model index of item
   */
  void playIfTaggedFile(const QModelIndex& index);
#endif

private:
  Q_DISABLE_COPY(FileList)

  /**
   * Format a string list from the selected files.
   * Supported format fields:
   * Those supported by FrameFormatReplacer::getReplacement(),
   * when prefixed with u, encoded as URL
   * %f filename
   * %F list of files
   * %uf URL of single file
   * %uF list of URLs
   * %d directory name
   * %b the web browser set in the configuration
   *
   * @todo %f and %F are full paths, which is inconsistent with the
   * export format strings but compatible with .desktop files.
   * %d is duration in export format.
   * The export codes should be changed.
   *
   * @param format format specification
   *
   * @return formatted string list.
   */
  QStringList formatStringList(const QStringList& format);

  /** Process for context menu commands */
  ExternalProcess* m_process;
  QList<QPersistentModelIndex> m_currentSelection;
  BaseMainWindowImpl* m_mainWin;
};

#endif // FILELIST_H
