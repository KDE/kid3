/**
 * \file eventcodedelegate.h
 * Delegate for event timing codes.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#ifndef EVENTCODEDELEGATE_H
#define EVENTCODEDELEGATE_H

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
  virtual ~EventCodeDelegate() override;

protected:
  // Reimplemented from EnumDelegate.
  virtual QStringList getEnumStrings() const override;
  virtual QString getStringForEnum(int enumNr) const override;
  virtual int getIndexForEnum(int enumNr) const override;
  virtual int getEnumForIndex(int index) const override;
};

#endif // EVENTCODEDELEGATE_H
