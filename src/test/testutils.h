/**
 * \file testutils.h
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

#ifndef TESTUTILS_H
#define TESTUTILS_H

class QObject;
class QStringList;

namespace TestUtils {

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
int runTestSuite(const QObject& testSuite, QStringList& args);

}

#endif
