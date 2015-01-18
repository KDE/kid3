/**
 * \file picturelabel.h
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 *
 * Copyright (C) 2009-2012  Urs Fleisch
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

#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include <QWidget>

class QByteArray;
class QLabel;

/**
 * Label for picture preview.
 */
class PictureLabel : public QWidget {
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
  virtual ~PictureLabel();

  /**
   * Set picture data.
   *
   * @param data picture data, empty if no picture is available
   */
  void setData(const QByteArray& data);

private:
  /**
   * Clear picture.
   */
  void clearPicture();

  QLabel* m_pictureLabel;
  QLabel* m_sizeLabel;
  uint m_pixmapHash;
};

#endif // PICTURELABEL_H
