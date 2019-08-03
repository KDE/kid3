/**
 * \file abstractcliformatter.cpp
 * Abstract base class for CLI formatter.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Jul 2019
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

#include "abstractcliformatter.h"
#include "abstractcli.h"

AbstractCliFormatter::AbstractCliFormatter(AbstractCliIO* io)
  : QObject(io), m_io(io)
{
}

AbstractCliFormatter::~AbstractCliFormatter()
{
}
