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
#include <QCoreApplication>
#include "pictureframe.h"

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QModelIndex)
#endif

namespace {

/**
 * Create a string list from a NULL terminated array of C strings.
 */
QStringList cstringArrayToStringList(const char* const* strs)
{
  QStringList result;
  while (*strs) {
    result.append(QCoreApplication::translate("@default", *strs++));
  }
  return result;
}

}


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

Frame::TagVersion ScriptUtils::toTagVersion(int nr)
{
  Frame::TagVersion tagVersion = static_cast<Frame::TagVersion>(nr);
  switch (tagVersion) {
  case Frame::TagNone:
  case Frame::TagV1:
  case Frame::TagV2:
  case Frame::TagV2V1:
    return tagVersion;
  }
  return Frame::TagNone;
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

/**
 * String list of frame field ID names.
 */
QStringList ScriptUtils::getFieldIdNames()
{
  return cstringArrayToStringList(Frame::Field::getFieldIdNames());
}

/**
 * String list of text encoding names.
 */
QStringList ScriptUtils::getTextEncodingNames()
{
  return cstringArrayToStringList(Frame::Field::getTextEncodingNames());
}

/**
 * String list of timestamp format names.
 */
QStringList ScriptUtils::getTimestampFormatNames()
{
  return cstringArrayToStringList(Frame::Field::getTimestampFormatNames());
}

/**
 * String list of picture type names.
 */
QStringList ScriptUtils::getPictureTypeNames()
{
  return cstringArrayToStringList(PictureFrame::getPictureTypeNames());
}

/**
 * String list of content type names.
 */
QStringList ScriptUtils::getContentTypeNames()
{
  return cstringArrayToStringList(Frame::Field::getContentTypeNames());
}
