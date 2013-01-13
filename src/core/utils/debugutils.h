/**
 * \file debugutils.h
 * Utility functions for debugging.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jan 2013
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

#ifndef QT_NO_DEBUG

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <QModelIndex>

class QAbstractItemModel;

namespace DebugUtils {

/**
 * Dump an item model.
 * @param model item model to dump
 * @param parent parent model index
 * @param indent number of spaces to indent
 */
void dumpModel(const QAbstractItemModel& model,
               const QModelIndex& parent = QModelIndex(), int indent = 0);


}

#endif // DEBUGUTILS_H

#endif // QT_NO_DEBUG
