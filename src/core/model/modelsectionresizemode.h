/**
 * \file modelsectionresizemode.h
 * Model section resize mode.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Nov 2018
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

#pragma once

/**
 * Mirrors values of QHeaderView::ResizeMode to avoid QWidgets dependency.
 */
enum class ModelSectionResizeMode : int {
  Interactive,     /**< User can resize the section */
  Stretch,         /**< Automatically resizes to fill available space */
  Fixed,           /**< User cannot resize the section */
  ResizeToContents /**< Automatically resizes based on contents */
};
