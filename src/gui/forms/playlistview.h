/**
 * \file playlistview.h
 * List view for playlist items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QListView>
#include "kid3api.h"

/**
 * List view with support for internal and external drag'n'drop operations.
 */
class KID3_GUI_EXPORT PlaylistView : public QListView {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit PlaylistView(QWidget* parent = nullptr);
  virtual ~PlaylistView() override;

  /**
   * Get role which is used when setting dropped items.
   * @return role, default is QFileSystemModel::FilePathRole.
   */
  int dropRole() const { return m_dropRole; }

  /**
   * Set role which is used when setting dropped items.
   * @param role model role
   */
  void setDropRole(int role) { m_dropRole = role; }

protected:
  /**
   * Accept drag.
   *
   * @param event drag event.
   */
  virtual void dragEnterEvent(QDragEnterEvent* event) override;

  /**
   * Handle event when mouse is moved while dragging.
   *
   * @param event drag event.
   */
  virtual void dragMoveEvent(QDragMoveEvent* event) override;

  /**
   * Handle event when mouse leaves widget while dragging.
   *
   * @param event drag event.
   */
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;

  /**
   * Handle drop event.
   *
   * @param event drop event.
   */
  virtual void dropEvent(QDropEvent* event) override;

private slots:
  void deleteCurrentRow();
  void moveUpCurrentRow();
  void moveDownCurrentRow();

private:
  bool droppingOnItself(QDropEvent* event, const QModelIndex& index);
  bool dropOn(QDropEvent* event, int* dropRow, int* dropCol,
              QModelIndex* dropIndex);
  DropIndicatorPosition position(const QPoint& pos, const QRect& rect,
                                 const QModelIndex& idx) const;
  QList<int> getSelectedRows() const;
  void swapRows(int offset1, int offset2);

  int m_dropRole;
};

#endif // PLAYLISTVIEW_H
