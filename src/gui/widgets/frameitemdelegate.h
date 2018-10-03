/**
 * \file frameitemdelegate.h
 * Delegate for table widget items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#ifndef FRAMEITEMDELEGATE_H
#define FRAMEITEMDELEGATE_H

#include <QItemDelegate>

class GenreModel;

/**
 * Delegate for table widget items.
 */
class FrameItemDelegate : public QItemDelegate {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param genreModel genre model
   * @param parent parent QTableView
   */
  explicit FrameItemDelegate(GenreModel* genreModel, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FrameItemDelegate() override = default;

  /**
   * Render delegate.
   * @param painter painter to be used
   * @param option style
   * @param index index of item
   */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const override;

  /**
   * Get size needed by delegate.
   * @param option style
   * @param index  index of item
   * @return size needed by delegate.
   */
  virtual QSize sizeHint(const QStyleOptionViewItem& option,
                         const QModelIndex& index) const override;

  /**
   * Create an editor to edit the cells contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return combo box editor widget.
   */
  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  /**
   * Set data to be edited by the editor.
   * @param editor editor widget
   * @param index  index of item
   */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  /**
   * Set model data supplied by editor.
   * @param editor editor widget
   * @param model  model
   * @param index  index of item
   */
  virtual void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private slots:
  /**
   * Format text if enabled.
   * @param txt text to format and set in line edit
   */
  void formatTextIfEnabled(const QString& txt);

  /**
   * Commit data and close the editor.
   */
  void commitAndCloseEditor();

private:
  GenreModel* m_genreModel;
  QValidator* m_trackNumberValidator;
  QValidator* m_dateTimeValidator;
};

/**
 * Editor for star rating.
 */
class StarEditor : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit StarEditor(QWidget* parent = nullptr);

  /**
   * Get size needed by editor.
   * @return size needed by editor.
   */
  virtual QSize sizeHint() const override;

  /**
   * Set star rating.
   * @param starCount number of stars
   */
  void setStarCount(int starCount);

  /**
   * Get star rating.
   * @return number of stars.
   */
  int starCount() const { return m_starCount; }

  /**
   * Check if the star rating has been modified in the editor since it has been
   * set using setStarCount().
   * @return true if rating modified in editor.
   */
  bool isStarCountEdited() const { return m_starCountEdited; }

signals:
  /**
   * Emitted when editing is finished.
   */
  void editingFinished();

protected:
  /**
   * Called when widget is painted.
   * @param event paint event
   */
  virtual void paintEvent(QPaintEvent* event) override;

  /**
   * Called when the mouse is moved inside the widget.
   * @param event mouse event
   */
  virtual void mouseMoveEvent(QMouseEvent* event) override;

  /**
   * Called when the mouse is released inside the widget.
   * @param event mouse event
   */
  virtual void mouseReleaseEvent(QMouseEvent* event) override;

  /**
   * Called when a key is pressed while the widget has focus.
   * @param event key event
   */
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  int starAtPosition(int x);
  void modifyStarCount(int starCount);

  int m_starCount;
  int m_paintedStarCount;
  bool m_starCountEdited;
};

#endif // FRAMEITEMDELEGATE_H
