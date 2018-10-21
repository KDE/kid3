/**
 * \file genremodel.h
 * Model with genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Jun 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include <QStandardItemModel>
#include "kid3api.h"

/**
 * Genre model.
 */
class KID3_CORE_EXPORT GenreModel : public QStandardItemModel {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param id3v1 true to create genres for ID3v1
   * @param parent parent widget
   */
  explicit GenreModel(bool id3v1, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~GenreModel() override = default;

  /**
   * Initialize module with genres.
   * This method is called by the constructor. It shall be called after
   * construction if genre settings are changed.
   */
  void init();

  /**
   * Get the row for a genre.
   * If the genre is not found, it is added at the returned row.
   * @param genreStr genre string
   * @return row number.
   */
  Q_INVOKABLE int getRowForGenre(const QString& genreStr);

private:
  bool m_id3v1;
};
