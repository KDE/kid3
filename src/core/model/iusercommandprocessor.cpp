/**
 * \file iusercommandprocessor.cpp
 * Interface for user command processor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

#include "iusercommandprocessor.h"

/**
 * Destructor.
 */
IUserCommandProcessor::~IUserCommandProcessor()
{
  // Just defining "virtual ~IServerImporterFactory() {}" in the header file
  // will lead to unresolved symbols when building with shared libraries on
  // Windows and a class from another library inherits from this class.
}

/**
 * Initialize processor.
 * This method can be used to initialize the processor before it is used.
 * @param app application context
 */
void IUserCommandProcessor::initialize(Kid3Application*)
{
}

/**
 * Cleanup processor.
 * This method can be used to clean up resources for which the plugin
 * destruction time is too late, e.g. GUI widgets.
 */
void IUserCommandProcessor::cleanup()
{
}
