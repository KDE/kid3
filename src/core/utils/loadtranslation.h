/**
 * \file loadtranslation.h
 * Load application translation.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Mar 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef LOADTRANSLATION_H
#define LOADTRANSLATION_H

#include <QString>
#include "kid3api.h"

namespace Utils {

/**
 * @brief Load application translation.
 *
 * @param lang preferred language, if not set, the language is determined by
 * the system configuration
 */
void KID3_CORE_EXPORT loadTranslation(const QString& lang = QString());

}

#endif // LOADTRANSLATION_H
