/**
 * \file timeeventeditor.h
 * Editor for time events (synchronized lyrics and event timing codes).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#ifndef TIMEEVENTEDITOR_H
#define TIMEEVENTEDITOR_H

#include <QWidget>
#include "frame.h"

class QLabel;
class QTableView;
class QModelIndex;
class IPlatformTools;
class Kid3Application;
class TimeEventModel;
class EventCodeDelegate;
class TaggedFile;

/**
 * Editor for time events (synchronized lyrics and event timing codes).
 */
class TimeEventEditor : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param app application context
   * @param parent parent widget
   * @param field  field containing binary data
   * @param taggedFile tagged file
   */
  TimeEventEditor(IPlatformTools* platformTools, Kid3Application* app,
                  QWidget* parent, const Frame::Field& field,
                  const TaggedFile* taggedFile);

  /**
   * Destructor.
   */
  virtual ~TimeEventEditor();

  /**
   * Set time event model.
   * @param model time event model
   */
  void setModel(TimeEventModel* model);

protected:
  /**
   * Connect to player when editor is shown.
   * @param event event
   */
  virtual void showEvent(QShowEvent* event);

  /**
   * Disconnect from player when editor is hidden.
   * @param event event
   */
  virtual void hideEvent(QHideEvent* event);

private slots:
  /**
   * Make sure that player is visible and in the edited file.
   */
  void preparePlayer();

  /**
   * Add a time event at the current player position.
   */
  void addItem();

  /**
   * Import data in LRC format.
   */
  void importData();

  /**
   * Export data in LRC format.
   */
  void exportData();

  /**
   * Insert a new row after the current row.
   */
  void insertRow();

  /**
   * Delete the selected rows.
   */
  void deleteRows();

  /**
   * Clear the selected cells.
   */
  void clearCells();

  /**
   * Add offset to time stamps.
   */
  void addOffset();

  /**
   * Seek to position of current time stamp.
   */
  void seekPosition();

  /**
   * Display custom context menu.
   *
   * @param pos position where context menu is drawn on screen
   */
  void customContextMenu(const QPoint& pos);

  /**
   * Called when the played track changed.
   * @param filePath path to file being played
   */
  void onTrackChanged(const QString& filePath);

  /**
   * Called when the player position changed.
   * @param position time in ms
   */
  void onPositionChanged(qint64 position);

private:
  QString getLrcNameFilter() const;

  IPlatformTools* m_platformTools;
  Kid3Application* m_app;
  QLabel* m_label;
  QTableView* m_tableView;
  EventCodeDelegate* m_eventCodeDelegate;
  TimeEventModel* m_model;
  const TaggedFile* m_taggedFile;
  QByteArray m_byteArray;
  bool m_fileIsPlayed;
};

#endif // TIMEEVENTEDITOR_H
