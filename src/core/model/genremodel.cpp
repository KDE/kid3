/**
 * \file genremodel.cpp
 * Model with genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Jun 2014
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

#include "genremodel.h"
#include "genres.h"
#include "tagconfig.h"

namespace {

QList<QStandardItem*> createGenreItems()
{
  QList<QStandardItem*> items;
  for (const char** sl = Genres::s_strList; *sl != 0; ++sl) {
    items.append(new QStandardItem(QString::fromLatin1(*sl)));
  }
  return items;
}

}

/**
 * Constructor.
 * @param parent parent widget
 */
GenreModel::GenreModel(bool id3v1, QObject* parent) :
  QStandardItemModel(parent), m_id3v1(id3v1)
{
  setObjectName(QLatin1String("GenreModel"));
  init();
}

/**
 * Destructor.
 */
GenreModel::~GenreModel()
{
}

/**
 * Initialize module with genres.
 * This method is called by the constructor. It shall be called after
 * construction if genre settings are changed.
 */
void GenreModel::init()
{
  QList<QStandardItem*> items;
  if (TagConfig::instance().onlyCustomGenres()) {
    items.append(new QStandardItem(QLatin1String("")));
  } else {
    items = createGenreItems();
  }
  QStringList customGenres = TagConfig::instance().customGenres();
  if (m_id3v1) {
    for (QStringList::const_iterator it = customGenres.constBegin();
         it != customGenres.constEnd();
         ++it) {
      if (Genres::getNumber(*it) != 255) {
        items.append(new QStandardItem(*it));
      }
    }
    if (items.count() <= 1) {
      // No custom genres for ID3v1 => Show standard genres
      items = createGenreItems();
    }
  } else {
    for (QStringList::const_iterator it = customGenres.constBegin();
         it != customGenres.constEnd();
         ++it) {
      items.append(new QStandardItem(*it));
    }
  }
  clear();
  appendColumn(items);
}

/**
 * Get the row for a genre.
 * If the genre is not found, it is added at the returned row.
 * @param genreStr genre string
 * @return row number.
 */
int GenreModel::getRowForGenre(const QString& genreStr)
{
  int genreIndex, customIndex;
  if (TagConfig::instance().onlyCustomGenres()) {
    genreIndex = 0;
    customIndex = 0;
  } else {
    genreIndex = genreStr.isNull()
        ? 0 : Genres::getIndex(Genres::getNumber(genreStr));
    customIndex = Genres::count + 1;
  }
  if (genreIndex <= 0) {
    QModelIndexList indexes = match(index(0, 0), Qt::DisplayRole, genreStr, 1,
                                    Qt::MatchExactly | Qt::MatchCaseSensitive);
    genreIndex = indexes.isEmpty() ? -1 : indexes.first().row();
    if (genreIndex < 0) {
      genreIndex = customIndex;
      setData(index(genreIndex, 0), genreStr, Qt::EditRole);
    }
  }
  return genreIndex;
}
