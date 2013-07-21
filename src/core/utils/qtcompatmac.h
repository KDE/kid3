/**
 * \file qtcompatmac.h
 * Qt compatibility macros.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Oct 2006
 *
 * Copyright (C) 2006-2013  Urs Fleisch
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

#ifndef QTCOMPATMAC_H
#define QTCOMPATMAC_H

#include <QtGlobal>

#if QT_VERSION >= 0x040600

/** Get icon from theme. */
#define QCM_QIcon_fromTheme(n) QIcon::fromTheme(QLatin1String(n), QIcon(QLatin1String(":/images/" n ".png")))

#else

/** Get icon from theme. */
#define QCM_QIcon_fromTheme(n) QIcon(QLatin1String(":/images/" n ".png"))

#endif

#endif // QTCOMPATMAC_H
