/**
 * \file genres.cpp
 * Alphabetical list of genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include <qstring.h>
#include <qmap.h>
#include "qtcompatmac.h"
#include "genres.h"

/**
 * Alphabetic list of genres, starts with unknown (empty) entry.
 *
 * 125: Last ID3v1, 142: WinAmp 1.91, 145: WinAmp 1.92, 255: unknown
 */
const char* Genres::s_genre[Genres::count + 3] = {
	"",                       // 255,
	"A Cappella",             // 123,
	"Acid",                   // 34,
	"Acid Jazz",              // 74, 
	"Acid Punk",              // 73, 
	"Acoustic",               // 99, 
	"Alternative",            // 20,
	"Alternative Rock",       // 40,
	"Ambient",                // 26,
	"Anime",                  // 145, 
	"Avantgarde",             // 90, 
	"Ballad",                 // 116,
	"Bass",                   // 41, 
	"Beat",                   // 135,
	"Bebob",                  // 85, 
	"Big Band",               // 96, 
	"Black Metal",            // 138,
	"Bluegrass",              // 89, 
	"Blues",                  // 0,
	"Booty Bass",             // 107,
	"BritPop",                // 132,
	"Cabaret",                // 65, 
	"Celtic",                 // 88, 
	"Chamber Music",          // 104,
	"Chanson",                // 102,
	"Chorus",                 // 97, 
	"Christian Gangsta Rap",  // 136,
	"Christian Rap",          // 61, 
	"Christian Rock",         // 141,
	"Classic Rock",           // 1,
	"Classical",              // 32,
	"Club",                   // 112,
	"Club-House",             // 128,
	"Comedy",                 // 57, 
	"Contemporary Christian", // 140,
	"Country",                // 2,
	"Crossover",              // 139,
	"Cult",                   // 58, 
	"Dance Hall",             // 125,
	"Dance",                  // 3,
	"Darkwave",               // 50, 
	"Death Metal",            // 22,
	"Disco",                  // 4,
	"Dream",                  // 55, 
	"Drum & Bass",            // 127,
	"Drum Solo",              // 122,
	"Duet",                   // 120,
	"Easy Listening",         // 98, 
	"Electronic",             // 52, 
	"Ethnic",                 // 48, 
	"Euro-House",             // 124,
	"Euro-Techno",            // 25,
	"Eurodance",              // 54, 
	"Folk",                   // 80,
	"Folk/Rock",              // 81, 
	"Folklore",               // 115,
	"Freestyle",              // 119,
	"Funk",                   // 5,
	"Fusion",                 // 30,
	"Fusion",                 // 84, 
	"Game",                   // 36,
	"Gangsta",                // 59, 
	"Goa",                    // 126,
	"Gospel",                 // 38,
	"Gothic",                 // 49, 
	"Gothic Rock",            // 91, 
	"Grunge",                 // 6,
	"Hard Rock",              // 79, 
	"Hardcore",               // 129,
	"Heavy Metal",            // 137,
	"Hip-Hop",                // 7,
	"House",                  // 35,
	"Humour",                 // 100,
	"Indie",                  // 131,
	"Industrial",             // 19,
	"Instrumental",           // 33,
	"Instrumental Pop",       // 46, 
	"Instrumental Rock",      // 47, 
	"Jazz",                   // 8,
	"Jazz+Funk",              // 29,
	"Jpop",                   // 146,
	"Jungle",                 // 63, 
	"Latin",                  // 86, 
	"Lo-Fi",                  // 71, 
	"Meditative",             // 45, 
	"Merengue",               // 142,
	"Metal",                  // 9,
	"Musical",                // 77, 
	"National Folk",          // 82, 
	"Native American",        // 64, 
	"Negerpunk",              // 133,
	"New Age",                // 10,
	"New Wave",               // 66, 
	"Noise",                  // 39,
	"Oldies",                 // 11,
	"Opera",                  // 103,
	"Other",                  // 12,
	"Polka",                  // 75, 
	"Polsk Punk",             // 134,
	"Pop",                    // 13,
	"Pop-Folk",               // 53, 
	"Pop/Funk",               // 62, 
	"Porn Groove",            // 109,
	"Power Ballad",           // 117,
	"Pranks",                 // 23,
	"Primus",                 // 108,
	"Progressive Rock",       // 92, 
	"Psychedelic",            // 67, 
	"Psychedelic Rock",       // 93, 
	"Punk",                   // 43, 
	"Punk Rock",              // 121,
	"R&B",                    // 14,
	"Rap",                    // 15,
	"Rave",                   // 68, 
	"Reggae",                 // 16,
	"Retro",                  // 76, 
	"Revival",                // 87, 
	"Rhythmic Soul",          // 118,
	"Rock",                   // 17,
	"Rock & Roll",            // 78, 
	"Salsa",                  // 143,
	"Samba",                  // 114,
	"Satire",                 // 110,
	"Showtunes",              // 69, 
	"Ska",                    // 21,
	"Slow Jam",               // 111,
	"Slow Rock",              // 95, 
	"Sonata",                 // 105,
	"Soul",                   // 42, 
	"Sound Clip",             // 37,
	"Soundtrack",             // 24,
	"Southern Rock",          // 56, 
	"Space",                  // 44, 
	"Speech",                 // 101,
	"Swing",                  // 83, 
	"Symphonic Rock",         // 94, 
	"Symphony",               // 106,
	"Synthpop",               // 147,
	"Tango",                  // 113,
	"Techno",                 // 18,
	"Techno-Industrial",      // 51, 
	"Terror",                 // 130,
	"Thrash Metal",           // 144,
	"Top 40",                 // 60,
	"Trailer",                // 70, 
	"Trance",                 // 31,
	"Tribal",                 // 72, 
	"Trip-Hop",               // 27,
	"Vocal",                  // 28,
	"Custom",                 // place for temporary custom genres
	0                         // end of StrList
};

/**
 * s_genreNum[n] gives the number of the n-th genre
 * in the alphabetically sorted list.
 */
const unsigned char Genres::s_genreNum[Genres::count + 1] = {
	255,
	123,
	34,
	74, 
	73, 
	99, 
	20,
	40,
	26,
	145, 
	90, 
	116,
	41, 
	135,
	85, 
	96, 
	138,
	89, 
	0,
	107,
	132,
	65, 
	88, 
	104,
	102,
	97, 
	136,
	61, 
	141,
	1,
	32,
	112,
	128,
	57, 
	140,
	2,
	139,
	58, 
	125,
	3,
	50, 
	22,
	4,
	55, 
	127,
	122,
	120,
	98, 
	52, 
	48, 
	124,
	25,
	54, 
	80,
	81, 
	115,
	119,
	5,
	30,
	84, 
	36,
	59, 
	126,
	38,
	49, 
	91, 
	6,
	79, 
	129,
	137,
	7,
	35,
	100,
	131,
	19,
	33,
	46, 
	47, 
	8,
	29,
	146,
	63, 
	86, 
	71, 
	45, 
	142,
	9,
	77, 
	82, 
	64, 
	133,
	10,
	66, 
	39,
	11,
	103,
	12,
	75, 
	134,
	13,
	53, 
	62, 
	109,
	117,
	23,
	108,
	92, 
	67, 
	93, 
	43, 
	121,
	14,
	15,
	68, 
	16,
	76, 
	87, 
	118,
	17,
	78, 
	143,
	114,
	110,
	69, 
	21,
	111,
	95, 
	105,
	42, 
	37,
	24,
	56, 
	44, 
	101,
	83, 
	94, 
	106,
	147,
	113,
	18,
	51, 
	130,
	144,
	60,
	70, 
	31,
	72, 
	27,
	28
};

const char** Genres::s_strList = &s_genre[0];

/**
 * Get name assigned to genre number.
 *
 * @param num genre number
 *
 * @return name, empty string for unknown number.
 */
const char* Genres::getName(int num)
{
	return s_genre[getIndex(num)];
}

/**
 * Get the index in the alphabethically sorted list from the genre number.
 *
 * @param num genre number
 *
 * @return index, 0 for unknown number.
 */
int Genres::getIndex(int num)
{
	int i;
	for (i = 0; i < Genres::count + 1; i++) {
		if (s_genreNum[i] == num) {
			return i;
		}
	}
	return 0; // 0 for unknown entry
}

/**
 * Get the genre number from a string containing a genre text.
 *
 * @param str string with genre
 *
 * @return genre number, 255 for unknown index.
 */
int Genres::getNumber(const QString& str)
{
	static QMap<QString, int> strNumMap;
	if (strNumMap.empty()) {
		// first time initialization
		for (int i = 0; i < Genres::count + 1; i++) {
			strNumMap.insert(s_genre[i], s_genreNum[i]);
		}
	}
	QMap<QString, int>::const_iterator it = strNumMap.find(str);
	if (it != strNumMap.end()) {
		return *it;
	}
	return 255; // 255 for unknown
}

/**
 * Get a name string from a string with a number or a name.
 * ID3v2 genres can be stored as "9", "(9)", "(9)Metal" or "Metal".
 *
 * @param str genre string.
 */
QString Genres::getNameString(const QString& str)
{
	if (!str.isEmpty()) {
		int cpPos, n;
		bool ok;
		if ((str[0] == '(') && ((cpPos = str.QCM_indexOf(')', 2)) > 1)) {
			n = str.mid(1, cpPos - 1).toInt(&ok);
			if (ok && n <= 0xff) {
				return getName(n);
			}
		} else if ((n = str.toInt(&ok)) >= 0 && n <= 0xff && ok) {
			return getName(n);
		}
	}
	return str;
}

/**
 * Get a number representation of a genre name if possible.
 *
 * @param str         string with genre name
 * @param parentheses true to put the number in parentheses
 *
 * @return genre string.
 */
QString Genres::getNumberString(const QString& str, bool parentheses)
{
	int n = getNumber(str);
	if (n < 0xff) {
		if (parentheses) {
			QString s("(");
			s += QString::number(n);
			s += ')';
			return s;
		} else {
			return QString::number(n);
		}
	}
	return str;
}
