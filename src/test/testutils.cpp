/**
 * \file testutils.cpp
 * Utility functions for tests.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012-2013  Urs Fleisch
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
#include <QTest>
#include <QRegularExpression>
#include <cstdio>

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
  QRegularExpression testCaseRe;
  for (int i = 1; i < args.size(); ++i) {
    if (args.at(i) == QLatin1String("-help")) {
      std::printf(" -testcases : Returns a list of current testcases\n"
                  " -testcase re      : Run only testcases matching regular "
                  "expression\n");
    } else if (args.at(i) == QLatin1String("-testcases")) {
      listTestCases = true;
      args.removeAt(i);
      break;
    } else if (args.at(i) == QLatin1String("-testcase") && i + 1 < args.size()) {
      testCaseRe.setPattern(args.at(i + 1));
      args.removeAt(i + 1);
      args.removeAt(i);
    }
  }

  int testsFailed = 0;
  int testCasesPassed = 0;
  int testCasesFailed = 0;

  foreach (QObject* tc, testSuite.children()) {
    QString tcName(QString::fromLatin1(tc->metaObject()->className()));
    if (listTestCases) {
      std::printf("%s\n", qPrintable(tcName));
    } else if (testCaseRe.pattern().isEmpty() || testCaseRe.match(tcName).hasMatch()) {
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
