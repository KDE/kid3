/**
 * \file taglibmodsupport.h
 * Support for Tracker modules.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Nov 2025
 *
 * Copyright (C) 2025  Urs Fleisch
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

#include "taglibformatsupport.h"

class TagLibModSupport : public TagLibFormatSupport {
public:
  TagLib::File* createFromExtension(TagLib::IOStream* stream,
                                    const TagLib::String& ext) const override;
  bool readFile(TagLibFile& f, TagLib::File* file) const override;
  bool readAudioProperties(TagLibFile& f,
    TagLib::AudioProperties* audioProperties) const override;
};
