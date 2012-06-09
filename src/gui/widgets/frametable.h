/**
 * \file frametable.h
 * Table to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Sep 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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

#ifndef FRAMETABLE_H
#define FRAMETABLE_H

#include <QTableView>

class QAction;
class QPoint;
class FrameTableModel;

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
   * @param parent parent widget
   */
  explicit FrameTable(FrameTableModel* model, QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~FrameTable();

  /**
   * Filters events if this object has been installed as an event filter
   * for the watched object.
   * This method is reimplemented to keep track of the current open editor.
   * It has to be installed on the viewport of the table.
   * @param watched watched object
   * @param event   event
   * @return true to filter event out.
   */
  virtual bool eventFilter(QObject* watched, QEvent* event);

  /**
   * Commit data from the current editor.
   * This is used to avoid losing the changes in open editors e.g. when
   * the file is changed using Alt-Up or Alt-Down.
   *
   * @return true if data was committed.
   */
  bool acceptEdit();

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

#endif // FRAMETABLE_H
