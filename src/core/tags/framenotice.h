/**
 * \file framenotice.h
 * Warning about tag frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 May 2017
 *
 * Copyright (C) 2017  Urs Fleisch
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

#ifndef FRAMENOTICE_H
#define FRAMENOTICE_H

#include <QObject>
#include "kid3api.h"

/**
 * Notice about a frame in a tag.
 */
class KID3_CORE_EXPORT FrameNotice {
  Q_GADGET
  Q_ENUMS(Warning)
public:
  /** Warning type. */
  enum Warning {
    None,         /**< No warning */
    Truncated,    /**< Truncated */
    TooLarge,     /**< Size is too large */
    TooLong,      /**< Value is too long */
    Unique,       /**< Must be unique */
    NlForbidden,  /**< New line is forbidden */
    CrForbidden,  /**< Carriage return is forbidden */
    FieldEmpty,   /**< Field must be non-empty */
    Numeric,      /**< Must be numeric */
    NrTotal,      /**< Must be numeric or number/total */
    DayMonth,     /**< Format is DDMM */
    HourMinute,   /**< Format is HHMM */
    Year,         /**< Format is YYYY */
    IsoDate,      /**< Must be ISO 8601 date/time */
    MusicalKey,   /**< Must be musical key, 3 characters, A-G, b, #, m, o */
    LanguageCode, /**< Must be ISO 639-2 language code, 3 lowercase characters */
    NumWarnings
  };

  /**
   * Constructor.
   * @param warning warning type
   */
  FrameNotice(Warning warning = None) { m_warning = warning; }

  /**
   * Equality operator.
   * @param rhs right hand side to compare
   * @return true if this == rhs.
   */
  bool operator==(const FrameNotice& rhs) const {
    return m_warning == rhs.m_warning;
  }

  /**
   * Bool operator, true if not none.
   */
  operator bool() const { return m_warning != None; }

  /**
   * Get translated description of notice.
   * @return description, empty if none.
   */
  QString getDescription() const;

private:
  Warning m_warning;
};

#endif // FRAMENOTICE_H
