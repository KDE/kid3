/**
 * \file scriptutils.h
 * QML support functions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2014
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

#ifndef SCRIPTUTILS_H
#define SCRIPTUTILS_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QPersistentModelIndex>
#include "trackdata.h"

/**
 * QML support functions.
 */
class KID3_PLUGIN_EXPORT ScriptUtils : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit ScriptUtils(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~ScriptUtils();

  /**
   * Convert a list of URLs to a list of local file paths.
   * @param urls file URLs
   * @return list with local file paths.
   */
  Q_INVOKABLE static QStringList toStringList(const QList<QUrl>& urls);

  /**
   * Convert a variant list containing model indexes to a list of persistent
   * model indexes.
   * @param lst variant list with model indexes
   * @return persistent model index list.
   */
  Q_INVOKABLE static QList<QPersistentModelIndex> toPersistentModelIndexList(
      const QVariantList& lst);

  /**
   * Convert an integer to a tag version.
   * @param nr tag mask (0=none, 1, 2, 3=1 and 2)
   */
  Q_INVOKABLE static TrackData::TagVersion toTagVersion(int nr);

  /**
   * Get data for @a roleName and @a row from @a model.
   * @param modelObj model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param parent optional parent model index
   * @return model data.
   */
  Q_INVOKABLE static QVariant getRoleData(QObject* modelObj, int row, const QByteArray& roleName,
      QModelIndex parent = QModelIndex());

  /**
   * Set data for @a roleName and @a row in @a model.
   * @param modelObj model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param value model data
   * @param parent optional parent model index
   * @return true if ok.
   */
  Q_INVOKABLE static bool setRoleData(QObject* modelObj, int row,
      const QByteArray& roleName, const QVariant& value,
      QModelIndex parent = QModelIndex());

  /**
   * Get data for @a roleName and model @a index.
   * @param index model index
   * @param roleName role name as used in scripting languages
   * @return model data.
   */
  Q_INVOKABLE static QVariant getIndexRoleData(const QModelIndex& index,
                                               const QByteArray& roleName);

  /**
   * Get property values as a string.
   * @param obj object to inspect
   * @return string containing property values.
   */
  Q_INVOKABLE static QString properties(QObject* obj);
};

#endif // SCRIPTUTILS_H
