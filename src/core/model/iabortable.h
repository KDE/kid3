/**
 * \file iabortable.h
 * Interface for abortable operations.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Jan 2013
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

#pragma once

#include "kid3api.h"

/**
 * Interface for abortable operations.
 */
class KID3_CORE_EXPORT IAbortable {
public:
  /**
   * Destructor.
   */
  virtual ~IAbortable();

  /**
   * Abort operation.
   */
  virtual void abort() = 0;

  /**
   * Check if operation is aborted.
   *
   * @return true if aborted.
   */
  virtual bool isAborted() const = 0;

  /**
   * Clear state which is reported by isAborted().
   */
  virtual void clearAborted() = 0;
};
