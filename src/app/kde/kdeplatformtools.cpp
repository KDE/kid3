/**
 * \file kdeplatformtools.cpp
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

#include "kdeplatformtools.h"
#include <kurl.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <ktoolinvocation.h>

/**
 * Constructor.
 */
KdePlatformTools::KdePlatformTools()
{
}

/**
 * Destructor.
 */
KdePlatformTools::~KdePlatformTools()
{
}

/**
 * Move file or directory to trash.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool KdePlatformTools::moveToTrash(const QString& path) const
{
  KUrl src;
  src.setPath(path);
  KIO::Job* job = KIO::trash(src);
  return KIO::NetAccess::synchronousRun(job, 0);
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void KdePlatformTools::displayHelp(const QString& anchor)
{
  KToolInvocation::invokeHelp(anchor);
}
