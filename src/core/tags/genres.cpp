/**
 * \file genres.cpp
 * Alphabetical list of genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2024  Urs Fleisch
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

#include "genres.h"
#include "frame.h"
#include <QString>
#include <QVector>
#include <QMap>

/**
 * Alphabetic list of genres, starts with unknown (empty) entry.
 *
 * 125: Last ID3v1, 142: WinAmp 1.91, 145: WinAmp 1.92, 148: WinAmp 5.6, 255: unknown
 */
const char* Genres::s_genre[Genres::count + 3] = {
  "",                       // 255,
  "A Cappella",             // 123,
  "Abstract",               // 148,
  "Acid",                   // 34,
  "Acid Jazz",              // 74,
  "Acid Punk",              // 73,
  "Acoustic",               // 99,
  "Alternative",            // 20,
  "Alternative Rock",       // 40,
  "Ambient",                // 26,
  "Anime",                  // 145,
  "Art Rock",               // 149,
  "Audio Theatre",          // 184,
  "Audiobook",              // 183,
  "Avant-Garde",            // 90,
  "Ballad",                 // 116,
  "Baroque",                // 150,
  "Bass",                   // 41,
  "Beat Music",             // 135,
  "Bebop",                  // 85,
  "Bhangra",                // 151,
  "Big Band",               // 96,
  "Big Beat",               // 152,
  "Black Metal",            // 138,
  "Bluegrass",              // 89,
  "Blues",                  // 0,
  "Booty Bass",             // 107,
  "Breakbeat",              // 153,
  "Britpop",                // 132,
  "Cabaret",                // 65,
  "Celtic",                 // 88,
  "Chamber Music",          // 104,
  "Chanson",                // 102,
  "Chillout",               // 154,
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
  "Dance",                  // 3,
  "Dancehall",              // 125,
  "Dark Wave",              // 50,
  "Death Metal",            // 22,
  "Disco",                  // 4,
  "Downtempo",              // 155,
  "Dream",                  // 55,
  "Drum & Bass",            // 127,
  "Drum Solo",              // 122,
  "Dub",                    // 156,
  "Dubstep",                // 189,
  "Duet",                   // 120,
  "EBM",                    // 157,
  "Easy Listening",         // 98,
  "Eclectic",               // 158,
  "Electro",                // 159,
  "Electroclash",           // 160,
  "Electronic",             // 52,
  "Emo",                    // 161,
  "Ethnic",                 // 48,
  "Euro House",             // 124,
  "Eurodance",              // 54,
  "Eurotechno",             // 25,
  "Experimental",           // 162,
  "Fast Fusion",            // 84,
  "Folk",                   // 80,
  "Folk Rock",              // 81,
  "Folklore",               // 115,
  "Freestyle",              // 119,
  "Funk",                   // 5,
  "Fusion",                 // 30,
  "G-Funk",                 // 188,
  "Game",                   // 36,
  "Gangsta",                // 59,
  "Garage",                 // 163,
  "Garage Rock",            // 190,
  "Global",                 // 164,
  "Goa",                    // 126,
  "Gospel",                 // 38,
  "Gothic",                 // 49,
  "Gothic Rock",            // 91,
  "Grunge",                 // 6,
  "Hard Rock",              // 79,
  "Hardcore",               // 129,
  "Heavy Metal",            // 137,
  "Hip Hop",                // 7,
  "House",                  // 35,
  "Humour",                 // 100,
  "IDM",                    // 165,
  "Illbient",               // 166,
  "Indie",                  // 131,
  "Indie Rock",             // 187,
  "Industrial",             // 19,
  "Industro-Goth",          // 167,
  "Instrumental",           // 33,
  "Instrumental Pop",       // 46,
  "Instrumental Rock",      // 47,
  "Jam Band",               // 168,
  "Jazz",                   // 8,
  "Jazz-Funk",              // 29,
  "Jpop",                   // 146,
  "Jungle",                 // 63,
  "Krautrock",              // 169,
  "Latin",                  // 86,
  "Leftfield",              // 170,
  "Lo-Fi",                  // 71,
  "Lounge",                 // 171,
  "Math Rock",              // 172,
  "Meditative",             // 45,
  "Merengue",               // 142,
  "Metal",                  // 9,
  "Musical",                // 77,
  "National Folk",          // 82,
  "Native American",        // 64,
  "Neoclassical",           // 182,
  "Neue Deutsche Welle",    // 185,
  "New Age",                // 10,
  "New Romantic",           // 173,
  "New Wave",               // 66,
  "Noise",                  // 39,
  "Nu-Breakz",              // 174,
  "Oldies",                 // 11,
  "Opera",                  // 103,
  "Other",                  // 12,
  "Podcast",                // 186,
  "Polka",                  // 75,
  "Polsk Punk",             // 134,
  "Pop",                    // 13,
  "Pop-Folk",               // 53,
  "Pop-Funk",               // 62,
  "Porn Groove",            // 109,
  "Post-Punk",              // 175,
  "Post-Rock",              // 176,
  "Power Ballad",           // 117,
  "Pranks",                 // 23,
  "Primus",                 // 108,
  "Progressive Rock",       // 92,
  "Psybient",               // 191,
  "Psychedelic",            // 67,
  "Psychedelic Rock",       // 93,
  "Psytrance",              // 177,
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
  "Shoegaze",               // 178,
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
  "Space Rock",             // 179,
  "Speech",                 // 101,
  "Swing",                  // 83,
  "Symphonic Rock",         // 94,
  "Symphony",               // 106,
  "Synth-Pop",              // 147,
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
  "Trop Rock",              // 180,
  "Vocal",                  // 28,
  "World Music",            // 181,
  "Worldbeat",              // 133,
  "Custom",                 // place for temporary custom genres
  nullptr                   // end of StrList
};

/**
 * s_genreNum[n] gives the number of the n-th genre
 * in the alphabetically sorted list.
 */
const unsigned char Genres::s_genreNum[Genres::count + 1] = {
  255,
  123,
  148,
  34,
  74,
  73,
  99,
  20,
  40,
  26,
  145,
  149,
  184,
  183,
  90,
  116,
  150,
  41,
  135,
  85,
  151,
  96,
  152,
  138,
  89,
  0,
  107,
  153,
  132,
  65,
  88,
  104,
  102,
  154,
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
  3,
  125,
  50,
  22,
  4,
  155,
  55,
  127,
  122,
  156,
  189,
  120,
  157,
  98,
  158,
  159,
  160,
  52,
  161,
  48,
  124,
  54,
  25,
  162,
  84,
  80,
  81,
  115,
  119,
  5,
  30,
  188,
  36,
  59,
  163,
  190,
  164,
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
  165,
  166,
  131,
  187,
  19,
  167,
  33,
  46,
  47,
  168,
  8,
  29,
  146,
  63,
  169,
  86,
  170,
  71,
  171,
  172,
  45,
  142,
  9,
  77,
  82,
  64,
  182,
  185,
  10,
  173,
  66,
  39,
  174,
  11,
  103,
  12,
  186,
  75,
  134,
  13,
  53,
  62,
  109,
  175,
  176,
  117,
  23,
  108,
  92,
  191,
  67,
  93,
  177,
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
  178,
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
  179,
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
  180,
  28,
  181,
  133
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
 * Get the index in the alphabetically sorted list from the genre number.
 *
 * @param num genre number
 *
 * @return index, 0 for unknown number.
 */
int Genres::getIndex(int num)
{
  for (int i = 0; i < Genres::count + 1; i++) {
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
      strNumMap.insert(QString::fromLatin1(s_genre[i]), s_genreNum[i]);
    }
  }
  if (auto it = strNumMap.constFind(str); it != strNumMap.constEnd()) {
    return *it;
  }
  return 255; // 255 for unknown
}

/**
 * Get a name string from a string with a number or a name.
 * ID3v2 genres can be stored as "9", "(9)", "(9)Metal" or "Metal".
 *
 * @param str genre string, it can also reference multiple ID3v1 genres
 * and have a refinement such as "(9)(138)Viking Metal".
 * Multiple genres can be separated by Frame::stringListSeparator().
 *
 * @return genre name or multiple genre names separated by
 * Frame::stringListSeparator().
 */
QString Genres::getNameString(const QString& str)
{
  if (!str.isEmpty()) {
    QStringList genres;
    const auto parts = Frame::splitStringList(str);
    for (const auto& part : parts) {
      auto s = part.trimmed();
      // First extract all genre codes which are in parentheses
      int offset = 0;
      int end = 0;
      while (s.length() > offset && s.at(offset) == QLatin1Char('(') &&
            (end = s.indexOf(QLatin1Char(')'), offset + 1)) > offset) {
        const auto genreCode = s.mid(offset + 1, end - 1);
        s = s.mid(end + 1);
        bool ok;
        int n = genreCode.toInt(&ok);
        if (genreCode == QLatin1String("RX") ||
            genreCode == QLatin1String("CR")) {
          genres.append(genreCode);
        } else if (ok && n >= 0 && n <= 0xff) {
          if (QString genreText = QString::fromLatin1(getName(n));
              !genreText.isEmpty()) {
            genres.append(genreText);
          }
        }
      }
      // Process the rest as a genre code or text
      s = s.trimmed();
      if (!s.isEmpty()) {
        bool ok;
        if (int n = s.toInt(&ok); ok && n >= 0 && n <= 0xff) {
          if (QString genreText = QString::fromLatin1(getName(n));
              !genreText.isEmpty()) {
            genres.append(genreText);
          }
        } else {
          genres.append(s);
        }
      }
    }
    genres.removeDuplicates();
    return Frame::joinStringList(genres);
  }
  return str;
}

/**
 * Get a number representation of a genre name if possible.
 *
 * @param str string with genre name, can also contain multiple genres
 * separated by Frame::stringListSeparator()
 * @param parentheses true to put the numbers in parentheses, this will
 * result in an ID3v2.3.0 genre string, which can containing multiple
 * references to ID3v1 genres and optionally a refinement as a genre text
 *
 * @return genre string using numbers where possible. If @a parentheses
 * is true, an ID3v2.3.0 genre string such as "(9)(138)Viking Metal" is
 * returned, else if @a str contains multiple genres, they are returned
 * as numbers (where possible) separated by Frame::stringListSeparator().
 */
QString Genres::getNumberString(const QString& str, bool parentheses)
{
  QStringList genres;
  QString genreText;

  const auto parts = Frame::splitStringList(str);
  for (const auto& part : parts) {
    auto s = part.trimmed();
    bool ok;
    int n = s.toInt(&ok);
    if (s == QLatin1String("RX") || s == QLatin1String("CR")) {
      genres.append(s);
    } else if ((ok && n >= 0 && n <= 255) ||
               (n = getNumber(s)) < 0xff) {
      genres.append(QString::number(n));
    } else if (!parentheses) {
      genres.append(s);
    } else if (genreText.isEmpty()) {
      // For ID3v2.3.0, we can append only one genre text as a refinement
      genreText = s;
    }
  }
  if (!parentheses) {
    return Frame::joinStringList(genres);
  }
  if (!genres.isEmpty()) {
    genreText.prepend(QLatin1Char('(') + genres.join(QLatin1String(")(")) +
      QLatin1Char(')'));
  }
  return genreText;
}
