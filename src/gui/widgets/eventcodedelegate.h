/**
 * \file eventcodedelegate.h
 * Delegate for event timing codes.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include "enumdelegate.h"

/**
 * Delegate for event timing codes.
 */
class EventCodeDelegate : public EnumDelegate {
public:
  /**
   * @brief Constructor.
   * @param parent parent object
   */
  explicit EventCodeDelegate(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~EventCodeDelegate() override = default;

protected:
  // Reimplemented from EnumDelegate.
  QStringList getEnumStrings() const override;
  QString getStringForEnum(int enumNr) const override;
  int getIndexForEnum(int enumNr) const override;
  int getEnumForIndex(int index) const override;
};
