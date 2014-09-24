/**
 * \file scriptutils.cpp
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

#include "scriptutils.h"
#include <QMetaProperty>

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QModelIndex)
#endif

ScriptUtils::ScriptUtils(QObject *parent) : QObject(parent)
{
}

ScriptUtils::~ScriptUtils()
{
}

QStringList ScriptUtils::toStringList(const QList<QUrl>& urls)
{
  QStringList paths;
  foreach (const QUrl& url, urls) {
    paths.append(url.toLocalFile());
  }
  return paths;
}

QList<QPersistentModelIndex> ScriptUtils::toPersistentModelIndexList(const QVariantList& lst)
{
  QList<QPersistentModelIndex> indexes;
  foreach (const QVariant& var, lst) {
#if QT_VERSION >= 0x050000
    indexes.append(var.toModelIndex());
#else
    indexes.append(qvariant_cast<QModelIndex>(var));
#endif
  }
  return indexes;
}

TrackData::TagVersion ScriptUtils::toTagVersion(int nr)
{
  TrackData::TagVersion tagVersion = static_cast<TrackData::TagVersion>(nr);
  switch (tagVersion) {
  case TrackData::TagNone:
  case TrackData::TagV1:
  case TrackData::TagV2:
  case TrackData::TagV2V1:
    return tagVersion;
  }
  return TrackData::TagNone;
}

QVariant ScriptUtils::getRoleData(
    QObject* modelObj, int row, const QByteArray& roleName,
    QModelIndex parent)
{
  if (QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(modelObj)) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->index(row, 0, parent).data(it.key());
      }
    }
  }
  return QVariant();
}

bool ScriptUtils::setRoleData(
    QObject* modelObj, int row, const QByteArray& roleName,
    const QVariant& value, QModelIndex parent)
{
  if (QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(modelObj)) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->setData(model->index(row, 0, parent), value, it.key());
      }
    }
  }
  return false;
}

QVariant ScriptUtils::getIndexRoleData(const QModelIndex& index,
                                       const QByteArray& roleName)
{
  if (const QAbstractItemModel* model = index.model()) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return index.data(it.key());
      }
    }
  }
  return QVariant();
}

QString ScriptUtils::properties(QObject* obj)
{
  QString str;
  const QMetaObject* meta;
  if (obj && (meta = obj->metaObject()) != 0) {
    str += QLatin1String("className: ");
    str += QString::fromLatin1(meta->className());
    for (int i = 0; i < meta->propertyCount(); i++) {
      QMetaProperty property = meta->property(i);
      const char* name = property.name();
      QVariant value = obj->property(name);
      str += QLatin1Char('\n');
      str += QString::fromLatin1(name);
      str += QLatin1String(": ");
      str += value.toString();
    }
  }
  return str;
}
