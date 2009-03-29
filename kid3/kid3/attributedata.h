/**
 * \file attributedata.h
 * String representation of attribute data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Mar 2009
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

#ifndef ATTRIBUTEDATA_H
#define ATTRIBUTEDATA_H

#include <qstring.h>
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif

/** Attribute data used e.g. by Windows Media Player. */
class AttributeData {
public:
	/** Attribute data types. */
	enum Type {
		Unknown, /**< Unknown type */
		Utf16,   /**< UTF-16 encoded, zero-terminated Unicode string */
		Guid,    /**< 128-bit GUID */
		DWord,   /**< 32-bit value little-endian */
		Binary   /**< Binary data */
	};

	/**
	 * Constructor.
	 *
	 * @param type type
	 */
	AttributeData(Type type)
	{
		m_type = type;
	}

	/**
	 * Constructor.
	 *
	 * @param name owner of Windows media PRIV frame
	 */
	AttributeData(const QString& name);

	/**
	 * Destructor.
	 */
	~AttributeData() {}

	/**
	 * Get type.
	 * @return type.
	 */
	Type getType() const { return m_type; }

	/**
	 * Convert attribute data to string.
	 *
	 * @param data byte array with data
	 * @param str  result string
	 *
	 * @return true if ok.
	 */
	bool toString(const QByteArray& data, QString& str);

	/**
	 * Convert attribute data string to byte array.
	 *
	 * @param str  string representation of data
	 * @param data result data
	 *
	 * @return true if ok.
	 */
	bool toByteArray(const QString& str, QByteArray& data);

	/**
	 * Check if a string represents a hexadecimal number, i.e.
	 * contains only characters 0..9, A..F.
	 *
	 * @param str string to check
	 * @param lastAllowedLetter last allowed character (normally 'F')
	 * @param additionalChars additional allowed characters
	 *
	 * @return true if string has hex format.
	 */
	static bool isHexString(const QString& str, char lastAllowedLetter = 'F',
	                        const QString additionalChars = QString());

private:
	Type m_type;
};

#endif // ATTRIBUTEDATA_H
