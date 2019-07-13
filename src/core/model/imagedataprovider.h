/**
 * \file imagedataprovider.h
 * Image provider to store image data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jul 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#include <QByteArray>
#include "kid3api.h"

/**
 * Image provider to store image data.
 */
class KID3_CORE_EXPORT ImageDataProvider {
public:
  /**
   * Get image data for the pixmap available via the "data" icon ID.
   * @return image data.
   */
  QByteArray getImageData() const { return m_data; }

  /**
   * Set image data for the pixmap available via the "data" icon ID.
   * @param data image data
   */
  void setImageData(const QByteArray& data) {
    m_data = data;
  }

private:
  QByteArray m_data;
};
