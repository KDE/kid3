/**
 * \file testutils.cpp
 * Utility functions for tests.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#include "testutils.h"
#include <QtTest>
#include <QRegExp>
#include <cstdio>
#include "serverimporter.h"

/**
 * Run the tests of a test suite.
 *
 * Besides the standard QtTest options, this test runner also allows to select
 * the testcase with "-testcase" and list the testcases with "-testcases".
 * This function will exit the application for certain \a args parsed by QtTest.
 *
 * @param testSuite an object containing QtTest test cases as children
 * @param args command line arguments from QCoreApplication::arguments()
 *
 * @return 0 if OK, the number of failed tests if failed.
 */
int TestUtils::runTestSuite(const QObject& testSuite, QStringList& args)
{
  bool listTestCases = false;
  QRegExp testCaseRe;
  for (int i = 1; i < args.size(); ++i) {
    if (args.at(i) == "-help") {
      std::printf(" -testcases : Returns a list of current testcases\n"
                  " -testcase re      : Run only testcases matching regular "
                  "expression\n");
    } else if (args.at(i) == "-testcases") {
      listTestCases = true;
      args.removeAt(i);
      break;
    } else if (args.at(i) == "-testcase" && i + 1 < args.size()) {
      testCaseRe.setPattern(args.at(i + 1));
      args.removeAt(i + 1);
      args.removeAt(i);
    }
  }

  int testsFailed = 0;
  int testCasesPassed = 0;
  int testCasesFailed = 0;

  foreach (QObject* tc, testSuite.children()) {
    QString tcName(tc->metaObject()->className());
    if (listTestCases) {
      std::printf("%s\n", qPrintable(tcName));
    } else if (testCaseRe.isEmpty() || testCaseRe.exactMatch(tcName)) {
      int rc = QTest::qExec(tc, args);
      testsFailed += rc;
      if (rc == 0) {
        ++testCasesPassed;
      } else {
        ++testCasesFailed;
      }
    }
  }

  std::printf("Test cases: %d passed, %d failed\n",
              testCasesPassed, testCasesFailed);
  return testsFailed;
}

/**
 * Dump an item model.
 * @param model item model to dump
 * @param parent parent model index
 * @param indent number of spaces to indent
 */
void TestUtils::dumpModel(const QAbstractItemModel& model,
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

/**
 * Dump an album list.
 * @param albumModel album list model
 */
void TestUtils::dumpAlbumList(const QStandardItemModel* albumModel)
{
  for (int row = 0; row < albumModel->rowCount(); ++row) {
    AlbumListItem* item = dynamic_cast<AlbumListItem*>(albumModel->item(row, 0));
    if (item) {
      qDebug("%s (%s, %s)", qPrintable(item->text()),
             qPrintable(item->getCategory()), qPrintable(item->getId()));
    }
  }
}
