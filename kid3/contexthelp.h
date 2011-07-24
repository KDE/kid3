/**
 * \file contexthelp.h
 * Context sensitive help.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef CONTEXTHELP_H
#define CONTEXTHELP_H

#include <QString>
#include "config.h"

class BrowserDialog;

/**
 * Context sensitive help.
 */
class ContextHelp {
public:
  /**
   * Display help for a topic.
   *
   * @param anchor anchor in help document
   */
  static void displayHelp(const QString& anchor = QString());

  /**
   * Free static resources.
   */
  static void staticCleanup();

private:
#ifndef CONFIG_USE_KDE
  static BrowserDialog* s_helpBrowser;
#endif
};

#endif // CONTEXTHELP_H
