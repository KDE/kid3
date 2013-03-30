/**
 * \file kdeplatformtools.h
 * KDE platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
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

#ifndef KDEPLATFORMTOOLS_H
#define KDEPLATFORMTOOLS_H

#include "iplatformtools.h"

/**
 * KDE platform specific tools.
 */
class KdePlatformTools : public IPlatformTools {
public:
  /**
   * Constructor.
   */
  KdePlatformTools();

  /**
   * Destructor.
   */
  virtual ~KdePlatformTools();

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  virtual bool moveToTrash(const QString& path) const;

  /**
   * Display help for a topic.
   *
   * @param anchor anchor in help document
   */
  virtual void displayHelp(const QString& anchor);
};

#endif // KDEPLATFORMTOOLS_H
