/**
 * \file contexthelp.cpp
 * Context sensitive help.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "contexthelp.h"
#include "iplatformtools.h"

IPlatformTools* ContextHelp::s_platformTools = nullptr;

/**
 * Initialize context help.
 *
 * @param platformTools platform tools to use
 */
void ContextHelp::init(IPlatformTools* platformTools)
{
  s_platformTools = platformTools;
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void ContextHelp::displayHelp(const QString& anchor)
{
  if (s_platformTools) {
    s_platformTools->displayHelp(anchor);
  }
}
