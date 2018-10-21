/**
 * \file kid3api.h
 * Macros for import and export of shared library symbols.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Feb 2012
 *
 * Copyright (C) 2012-2018  Urs Fleisch
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

#pragma once

#ifdef KID3_SHARED

#include <QtGlobal>

#ifdef KID3_BUILD_CORE_LIB
#define KID3_CORE_EXPORT Q_DECL_EXPORT
#else
#define KID3_CORE_EXPORT Q_DECL_IMPORT
#endif

#ifdef KID3_BUILD_GUI_LIB
#define KID3_GUI_EXPORT Q_DECL_EXPORT
#else
#define KID3_GUI_EXPORT Q_DECL_IMPORT
#endif

#ifdef KID3_BUILD_PLUGIN_LIB
#define KID3_PLUGIN_EXPORT Q_DECL_EXPORT
#else
#define KID3_PLUGIN_EXPORT Q_DECL_IMPORT
#endif

#else

#define KID3_CORE_EXPORT
#define KID3_GUI_EXPORT
#define KID3_PLUGIN_EXPORT

#endif
