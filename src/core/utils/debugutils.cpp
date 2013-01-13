/**
 * \file debugutils.cpp
 * Utility functions for debugging.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef QT_NO_DEBUG

#include "debugutils.h"

/**
 * Dump an item model.
 * @param model item model to dump
 * @param parent parent model index
 * @param indent number of spaces to indent
 */
void DebugUtils::dumpModel(const QAbstractItemModel& model,
                           const QModelIndex& parent, int indent)
{
  if (indent == 0) {
    QString name(model.objectName());
    if (name.isEmpty()) {
      if (const QMetaObject* metaObject = model.metaObject()) {
        name = metaObject->className();
      }
    }
    qDebug("Dump for %s", qPrintable(name));
    QString columnStr;
    for (int i = 0; i < model.columnCount(parent); ++i) {
      if (i != 0)
        columnStr += ", ";
      columnStr += QString::number(i);
      columnStr += ": ";
      columnStr += model.headerData(i, Qt::Horizontal).toString();
    }
    qDebug("%s", qPrintable(columnStr));
  }
  if (!model.hasChildren(parent))
    return;

  for (int row = 0; row < model.rowCount(parent); ++row) {
    QString rowStr(indent, ' ');
    QString rowHeader(model.headerData(row, Qt::Vertical).toString());
    rowStr += QString::number(row);
    if (!rowHeader.isEmpty()) {
      rowStr += ' ';
      rowStr += rowHeader;
    }
    rowStr += ':';
    QModelIndexList indexesWithChildren;
    for (int column = 0; column < model.columnCount(parent); ++column) {
      QModelIndex idx(model.index(row, column, parent));
      if (column > 0)
        rowStr += ",";
      rowStr += QString("%1%2:").
          arg(model.hasChildren(idx) ? "p" : "").
          arg(column);
      rowStr += model.data(idx).toString();
      if (model.hasChildren(idx))
        indexesWithChildren.append(idx);
    }
    qDebug("%s", qPrintable(rowStr));
    foreach (QModelIndex idx, indexesWithChildren) {
      dumpModel(model, idx, indent + 2);
    }
  }
}

#endif
