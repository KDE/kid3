/**
 * \file amazonconfig.h
 * Amazon configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Dec 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#ifndef AMAZONCONFIG_H
#define AMAZONCONFIG_H

#include "serverimporterconfig.h"

/**
 * Amazon configuration.
 */
class AmazonConfig : public StoredConfig<AmazonConfig, ServerImporterConfig> {
public:
  /**
   * Constructor.
   */
  AmazonConfig();

  /**
   * Destructor.
   */
  virtual ~AmazonConfig();

  static int s_index;
};

#endif // AMAZONCONFIG_H
