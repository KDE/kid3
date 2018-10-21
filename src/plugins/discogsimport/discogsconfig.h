/**
 * \file discogsconfig.h
 * Discogs configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2018  Urs Fleisch
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

#include "serverimporterconfig.h"

/**
 * Discogs configuration.
 */
class DiscogsConfig : public StoredConfig<DiscogsConfig, ServerImporterConfig> {
public:
  /**
   * Constructor.
   */
  DiscogsConfig();

  /**
   * Destructor.
   */
  virtual ~DiscogsConfig() override;

private:
  friend DiscogsConfig&
  StoredConfig<DiscogsConfig, ServerImporterConfig>::instance();

  /** Index in configuration storage */
  static int s_index;
};
