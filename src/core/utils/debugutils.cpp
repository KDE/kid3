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

#include "debugutils.h"
#include <QMetaObject>
#include <QMetaMethod>

/**
 * Constructor.
 * @param parent parent object
 */
DebugUtils::SignalEmissionDumper::SignalEmissionDumper(QObject* parent) :
  QObject(parent)
{
}

#ifndef QT_NO_DEBUG

/**
 * Monitor signal emissions of object
 * @param obj object to monitor
 */
void DebugUtils::SignalEmissionDumper::connectObject(QObject* obj)
{
  const QMetaObject* metaObject = obj->metaObject();
  for (int i = 0; i < metaObject->methodCount(); ++i) {
    QByteArray sig = metaObject->method(i).methodSignature();
    if (metaObject->indexOfSignal(sig) != -1) {
      sig.prepend(QSIGNAL_CODE + '0');
      connect(obj, sig, this, SLOT(printSignal()));
    }
  }
}

/**
 * @brief Print emitted signal to debug output.
 */
void DebugUtils::SignalEmissionDumper::printSignal()
{
  if (QObject* obj = sender()) {
    int idx = senderSignalIndex();
    if (idx != -1) {
      QByteArray sig = obj->metaObject()->method(idx).methodSignature();
      if (!sig.isEmpty()) {
        qDebug("SIGNAL OUT %s::%s %s",
               obj->metaObject()->className(),
               qPrintable(obj->objectName().isEmpty()
                          ? QLatin1String("unnamed") : obj->objectName()),
               sig.constData());
      }
    }
  }
}


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
        name = QString::fromLatin1(metaObject->className());
      }
    }
    qDebug("Dump for %s", qPrintable(name));
    QString columnStr;
    for (int i = 0; i < model.columnCount(parent); ++i) {
      if (i != 0)
        columnStr += QLatin1String(", ");
      columnStr += QString::number(i);
      columnStr += QLatin1String(": ");
      columnStr += model.headerData(i, Qt::Horizontal).toString();
    }
    qDebug("%s", qPrintable(columnStr));
  }
  if (!model.hasChildren(parent))
    return;

  for (int row = 0; row < model.rowCount(parent); ++row) {
    QString rowStr(indent, QLatin1Char(' '));
    QString rowHeader(model.headerData(row, Qt::Vertical).toString());
    rowStr += QString::number(row);
    if (!rowHeader.isEmpty()) {
      rowStr += QLatin1Char(' ');
      rowStr += rowHeader;
    }
    rowStr += QLatin1Char(':');
    QModelIndexList indexesWithChildren;
    for (int column = 0; column < model.columnCount(parent); ++column) {
      QModelIndex idx(model.index(row, column, parent));
      if (column > 0)
        rowStr += QLatin1String(",");
      rowStr += QString(QLatin1String("%1%2:")).
          arg(model.hasChildren(idx) ? QLatin1String("p") : QLatin1String("")).
          arg(column);
      rowStr += model.data(idx).toString();
      if (model.hasChildren(idx))
        indexesWithChildren.append(idx);
    }
    qDebug("%s", qPrintable(rowStr));
    const auto idxs = indexesWithChildren;
    for (const QModelIndex& idx : idxs) {
      dumpModel(model, idx, indent + 2);
    }
  }
}

#else

void DebugUtils::SignalEmissionDumper::connectObject(QObject*) {}
void DebugUtils::SignalEmissionDumper::printSignal() {}

#endif
