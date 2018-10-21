/**
 * \file frametable.h
 * Table to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Sep 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
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

#include <QTableView>

class QAction;
class QPoint;
class FrameTableModel;
class GenreModel;

/**
 * Table to edit frames.
 */
class FrameTable : public QTableView {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model frame table model
   * @param genreModel genre model
   * @param parent parent widget
   */
  explicit FrameTable(FrameTableModel* model, GenreModel* genreModel,
                      QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FrameTable() override = default;

  /**
   * Filters events if this object has been installed as an event filter
   * for the watched object.
   * This method is reimplemented to keep track of the current open editor.
   * It has to be installed on the viewport of the table.
   * @param watched watched object
   * @param event   event
   * @return true to filter event out.
   */
  virtual bool eventFilter(QObject* watched, QEvent* event) override;

  /**
   * Commit data from the current editor.
   * This is used to avoid losing the changes in open editors e.g. when
   * the file is changed using Alt-Up or Alt-Down.
   *
   * @return true if data was committed.
   */
  bool acceptEdit();

  /**
   * Get current editor widget if the table is currently in edit state.
   * @return current editor widget, 0 if not in edit state.
   */
  const QWidget* getCurrentEditor() const;

  /**
   * Select in the editor of a value row.
   * @param row row number
   * @param start start position
   * @param length number of characters to select
   */
  void setValueSelection(int row, int start, int length);

private slots:
  /**
   * Display context menu.
   *
   * @param row row at which context menu is displayed
   * @param col column at which context menu is displayed
   * @param pos position where context menu is drawn on screen
   */
  void contextMenu(int row, int col, const QPoint& pos);

  /**
   * Display custom context menu.
   *
   * @param pos position where context menu is drawn on screen
   */
  void customContextMenu(const QPoint& pos);

private:
  QWidget* m_currentEditor;
};
