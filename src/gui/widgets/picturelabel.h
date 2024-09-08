/**
 * \file picturelabel.h
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 *
 * Copyright (C) 2009-2018  Urs Fleisch
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

#include <QWidget>

class QByteArray;
class QLabel;
class QToolButton;
class PictureFrame;

/**
 * Label for picture preview.
 */
class PictureLabel : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit PictureLabel(QWidget* parent);

  /**
   * Destructor.
   */
  ~PictureLabel() override;

  /**
   * Set picture data.
   *
   * @param pictures picture frames, empty if no picture is available
   */
  void setData(const QList<PictureFrame>& pictures);

  /**
   * Get picture index.
   *
   * @return index of picture, -1 if not available.
   */
  int getIndex() const { return m_index; }

  /**
   * Set picture index.
   *
   * @param index index of picture to show
   */
  void setIndex(int index);

  /**
   * Set picture index to last picture.
   */
  void setLastIndex();

private slots:
  /**
   * Select previous picture.
   */
  void previous();

  /**
   * Select next picture.
   */
  void next();

private:
  /**
   * Update UI controls.
   */
  void updateControls();

  QList<PictureFrame> m_pictures;
  QLabel* m_pictureLabel;
  QLabel* m_sizeLabel;
  QLabel* m_indexLabel;
  QWidget* m_indexWidget;
  QToolButton* m_previousButton;
  QToolButton* m_nextButton;
  uint m_pixmapHash;
  int m_index;
};
