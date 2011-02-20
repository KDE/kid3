/**
 * \file attributedata.cpp
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

#include "attributedata.h"
#include <qmap.h>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param name owner of Windows media PRIV frame
 */
AttributeData::AttributeData(const QString& name)
{
	/** PRIV-owner and type of Windows media PRIV frames */
	static const struct TypeOfWmPriv {
		const char* str;
		Type type;
	} typeOfWmPriv[] = {
		{ "AverageLevel", DWord },
		{ "PeakValue", DWord },
		{ "WM/AlbumArtist", Utf16 },
		{ "WM/AuthorURL", Utf16 },
		{ "WM/BeatsPerMinute", Utf16 },
		{ "WM/Composer", Utf16 },
		{ "WM/Conductor", Utf16 },
		{ "WM/ContentDistributor", Utf16 },
		{ "WM/ContentGroupDescription", Utf16 },
		{ "WM/EncodedBy", Utf16 },
		{ "WM/EncodingSettings", Utf16 },
		{ "WM/EncodingTime", Binary },
		{ "WM/Genre", Utf16 },
		{ "WM/InitialKey", Utf16 },
		{ "WM/Language", Utf16 },
		{ "WM/Lyrics", Utf16 },
		{ "WM/Lyrics_Synchronised", Binary },
		{ "WM/MCDI", Binary },
		{ "WM/MediaClassPrimaryID", Guid },
		{ "WM/MediaClassSecondaryID", Guid },
		{ "WM/Mood", Utf16 },
		{ "WM/ParentalRating", Utf16 },
		{ "WM/PartOfSet", Utf16 },
		{ "WM/Period", Utf16 },
		{ "WM/Picture", Binary },
		{ "WM/Producer", Utf16 },
		{ "WM/PromotionURL", Utf16 },
		{ "WM/Provider", Utf16 },
		{ "WM/Publisher", Utf16 },
		{ "WM/SubTitle", Utf16 },
		{ "WM/ToolName", Utf16 },
		{ "WM/ToolVersion", Utf16 },
		{ "WM/TrackNumber", Utf16 },
		{ "WM/UniqueFileIdentifier", Utf16 },
		{ "WM/UserWebURL", Binary },
		{ "WM/WMCollectionGroupID", Guid },
		{ "WM/WMCollectionID", Guid },
		{ "WM/WMContentID", Guid },
		{ "WM/Writer", Utf16 }
	};

	static QMap<QString, int> strNumMap;
	if (strNumMap.empty()) {
		// first time initialization
		for (unsigned i = 0; i < sizeof(typeOfWmPriv) / sizeof(typeOfWmPriv[0]);
		     ++i) {
			strNumMap.insert(QString(typeOfWmPriv[i].str), typeOfWmPriv[i].type);
		}
	}
	QMap<QString, int>::const_iterator it =
		strNumMap.find(name);
	m_type = (it != strNumMap.end()) ? static_cast<Type>(*it) : Unknown;
}

/**
 * Convert attribute data to string.
 *
 * @param data byte array with data
 * @param str  result string
 *
 * @return true if ok.
 */
bool AttributeData::toString(const QByteArray& data, QString& str)
{
	switch (m_type) {
		case Utf16: {
			const ushort* unicode = reinterpret_cast<const ushort*>(data.data());
			int size = data.size() / 2;
			while (size > 0 && unicode[size - 1] == 0) {
				--size;
			}
#if QT_VERSION >= 0x040000
			str = QString::fromUtf16(unicode, size);
#else
			str.setUnicodeCodes(unicode, size);
#endif
			return true;
		}
		case Guid:
			if (data.size() == 16) {
#if QT_VERSION >= 0x040000
				str.clear();
#else
				str.truncate(0);
#endif
				for (int i = 0; i < 16; ++i) {
					if (i == 4 || i == 6 || i == 8 || i == 10) {
						str += '-';
					}
					unsigned char c = (unsigned char)data[i];
					unsigned char d = c >> 4;
					str += d >= 10 ? d - 10 + 'A' : d + '0';
					d = c & 0x0f;
					str += d >= 10 ? d - 10 + 'A' : d + '0';
				}
				return true;
			}
			break;
		case DWord:
			if (data.size() == 4) {
				ulong num = 0;
				for (int i = 3; i >= 0; --i) {
					num <<= 8;
					num |= static_cast<unsigned char>(data[i]);
				}
				str.setNum(num);
				return true;
			}
			break;
		case Binary:
		case Unknown:
		default:
			;
	}
	return false;
}

/**
 * Convert attribute data string to byte array.
 *
 * @param str  string representation of data
 * @param data result data
 *
 * @return true if ok.
 */
bool AttributeData::toByteArray(const QString& str, QByteArray& data)
{
	switch (m_type) {
		case Utf16: {
#if QT_VERSION >= 0x040000
			const ushort* unicode = str.utf16();
#else
			const ushort* unicode = str.ucs2();
#endif
			QCM_duplicate(data, reinterpret_cast<const char*>(unicode),
			              (str.length() + 1) * 2);
			return true;
		}
		case Guid: {
			QString hexStr(str.QCM_toUpper());
			hexStr.remove('-');
			if (hexStr.length() == 32) {
				unsigned char buf[16];
				unsigned char* bufPtr = buf;
				for (int i = 0; i < 32;) {
#if QT_VERSION >= 0x040000
					unsigned char h = (unsigned char)hexStr[i++].toLatin1();
					unsigned char l = (unsigned char)hexStr[i++].toLatin1();
#else
					unsigned char h = (unsigned char)hexStr[i++].latin1();
					unsigned char l = (unsigned char)hexStr[i++].latin1();
#endif
					if (!((h >= '0' && h <= '9') || (h >= 'A' && h <= 'F')) ||
							!((l >= '0' && l <= '9') || (l >= 'A' && l <= 'F'))) {
						return false;
					}
					*bufPtr++ = ((h >= 'A' ? h + 10 - 'A' : h - '0') << 4) |
					  (l >= 'A' ? l + 10 - 'A' : l - '0');
				}
				QCM_duplicate(data, reinterpret_cast<char*>(buf), 16);
				return true;
			}
			break;
		}
		case DWord: {
			bool ok;
			ulong num = str.toULong(&ok);
			if (ok) {
				data.resize(4);
				for (int i = 0; i < 4; ++i) {
					data[i] = num & 0xff;
					num >>= 8;
				}
				return true;
			}
			break;
		}
		case Binary:
		case Unknown:
		default:
			;
	}
	return false;
}

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
bool AttributeData::isHexString(const QString& str, char lastAllowedLetter,
                                const QString additionalChars)
{
	if (str.isEmpty()) {
		return false;
	}
	for (int i = 0; i < static_cast<int>(str.length()); ++i) {
#if QT_VERSION >= 0x040000
		char c = str[i].toLatin1();
#else
		char c = str[i].latin1();
#endif
		if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= lastAllowedLetter) ||
					additionalChars.QCM_indexOf(c) != -1)) {
			return false;
		}
	}
	return true;
}
