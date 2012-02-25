/**
 * \file contexthelp.cpp
 * Context sensitive help.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011-2012  Urs Fleisch
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
#ifdef CONFIG_USE_KDE
#include <ktoolinvocation.h>
#else
#include "browserdialog.h"
#include "qtcompatmac.h"

BrowserDialog* ContextHelp::s_helpBrowser = 0;
#endif

#ifdef CONFIG_USE_KDE

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void ContextHelp::displayHelp(const QString& anchor)
{
  KToolInvocation::invokeHelp(anchor);
}

#else

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void ContextHelp::displayHelp(const QString& anchor)
{
  if (!s_helpBrowser) {
    QString caption(QCM_translate(I18N_NOOP("Kid3 Handbook")));
    s_helpBrowser = new BrowserDialog(0, caption);
  }
  s_helpBrowser->goToAnchor(anchor);
  s_helpBrowser->setModal(!anchor.isEmpty());
  if (s_helpBrowser->isHidden()) {
    s_helpBrowser->show();
  }
}
#endif

/**
 * Free static resources.
 */
void ContextHelp::staticCleanup()
{
#ifndef CONFIG_USE_KDE
  delete s_helpBrowser;
  s_helpBrowser = 0;
#endif
}
