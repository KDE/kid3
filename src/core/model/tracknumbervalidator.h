/**
 * \file tracknumbervalidator.h
 * Validator for track and disc numbers with optional total.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 24 May 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include <QValidator>
#include "kid3api.h"

/**
 * Validator for track and disc numbers with optional total.
 */
class KID3_CORE_EXPORT TrackNumberValidator : public QValidator {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit TrackNumberValidator(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~TrackNumberValidator() override = default;

  /**
   * Validate input string.
   * @param input input string
   * @param pos current position
   * @return current state of input (Invalid, Intermediate or Acceptable).
   */
  virtual QValidator::State validate(QString& input, int& pos) const override;

  /**
   * Attempt to change @a input to be valid.
   * @param input input string
   */
  virtual void fixup(QString& input) const override;
};
