/**
 * \file test/main.cpp
 * Main program for executing unit test cases.
 * Besides the standard QtTest options, this test runner also allows to select
 * the testcase with "-testcase" and list the testcases with "-testcases".
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

#include <QCoreApplication>
#include <QStringList>
#include "testutils.h"
#include "testjsonparser.h"
#include "testmusicbrainzreleaseimportparser.h"
#include "testmusicbrainzreleaseimporter.h"
#include "testdiscogsimportparser.h"
#include "testdiscogsimporter.h"

/**
 * Main routine for test runner.
 *
 * @param argc number of arguments (including the command)
 * @param argv command and arguments
 *
 * @return 0 if OK, the number of failed tests if failed.
 */
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QStringList args = app.arguments();

  static QObject* const testCases[] = {
    new TestJsonParser,
    new TestMusicBrainzReleaseImportParser,
    new TestMusicBrainzReleaseImporter,
    new TestDiscogsImportParser,
    new TestDiscogsImporter,
    0
  };

  QObject ts;
  for (QObject* const* tcPtr = testCases; *tcPtr; ++tcPtr) {
    (*tcPtr)->setParent(&ts);
  }
  return TestUtils::runTestSuite(ts, args);
}
