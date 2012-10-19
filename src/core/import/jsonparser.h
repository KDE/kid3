/**
 * \file jsonparser.h
 * JSON serializer and deserializer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "kid3api.h"

class QString;
class QVariant;

/**
 * Serialize and deserialize simple string-variant maps with JSON.
 *
 * The following types are serialized: null (represented by invalid QVariant),
 * bool, int, qlonglong, double, QString, QDateTime (ISO 8601 extended format
 * YYYY-MM-DDTHH:MM:SS).
 */
namespace JsonParser {

/**
 * Deserialize a JSON string to a string-variant map.
 * @param str string to deserialize
 * @param ok if not null, true is returned here on success
 * @return deserialized string-variant map
 */
QVariant KID3_CORE_EXPORT deserialize(const QString& str, bool* ok = 0);

/**
 * Serialize a variant as a JSON string.
 * @param var variant
 * @return JSON representation of @a var.
 */
QString KID3_CORE_EXPORT serialize(const QVariant& var);

}

#endif
