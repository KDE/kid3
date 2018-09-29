/**
 * \file tracknumbervalidator.cpp
 * Validator for track and disc numbers with optional total.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 24 May 2014
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

#include "tracknumbervalidator.h"

/**
 * Constructor.
 * @param parent parent object
 */
TrackNumberValidator::TrackNumberValidator(QObject *parent) : QValidator(parent)
{
}

/**
 * Destructor.
 */
TrackNumberValidator::~TrackNumberValidator()
{
}

/**
 * Validate input string.
 * @param input input string
 * @param pos current position
 * @return current state of input (Invalid, Intermediate or Acceptable).
 */
QValidator::State TrackNumberValidator::validate(QString& input, int&) const
{
  for (auto it = input.constBegin(); it != input.constEnd(); ++it) {
    if (!it->isDigit() && *it != QLatin1Char('/')) {
      return Invalid;
    }
  }

  int len = input.length();
  if (len == 0)
    return Acceptable;

  bool ok;
  int slashPos = input.indexOf(QLatin1Char('/'));
  if (slashPos == -1) {
    input.toULongLong(&ok);
    return ok ? Acceptable : Invalid;
  } else {
    if (slashPos == len - 1) {
      return Intermediate;
    }
    if (input.indexOf(QLatin1Char('/'), slashPos + 1) != -1) {
      return Invalid;
    }
    if (slashPos == 0) {
      return Intermediate;
    }

    input.left(slashPos).toULongLong(&ok);
    if (ok) {
      input.mid(slashPos + 1).toULongLong(&ok);
    }
  }
  return ok ? Acceptable : Invalid;
}

/**
 * Attempt to change @a input to be valid.
 * @param input input string
 */
void TrackNumberValidator::fixup(QString& input) const
{
  int len = input.length();
  if (len > 0) {
    if (input.at(0) == QLatin1Char('/')) {
      input = input.mid(1);
    } else if (input.at(len - 1) == QLatin1Char('/')) {
      input.truncate(len - 1);
    }
  }
}
