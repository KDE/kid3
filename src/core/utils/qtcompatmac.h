/**
 * \file qtcompatmac.h
 * Qt compatibility macros.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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
#include <QObject>
#include "config.h"

#ifdef CONFIG_USE_KDE
#include <klocale.h>

#define QCM_translate(s) i18n(s)

#else
#include <QCoreApplication>

#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TRANSLATE_NOOP("@default", s)

#define QCM_translate(s) QCoreApplication::translate("@default", s)

#endif

#endif // QTCOMPATMAC_H
