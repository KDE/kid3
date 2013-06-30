/**
 * \file tagconfig.cpp
 * Tag related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
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

#include "tagconfig.h"
#include "frame.h"

namespace {

/** Default value for comment name */
const char* const defaultCommentName = "COMMENT";

}

/**
 * Constructor.
 *
 * @param group configuration group
 */
TagConfig::TagConfig(const QString& group) :
  GeneralConfig(group),
  m_markTruncations(true),
  m_enableTotalNumberOfTracks(false),
  m_genreNotNumeric(false),
  m_commentName(QString::fromLatin1(defaultCommentName)),
  m_pictureNameItem(VP_METADATA_BLOCK_PICTURE),
  m_id3v2Version(ID3v2_3_0),
  m_textEncodingV1(QLatin1String("ISO-8859-1")),
  m_textEncoding(TE_ISO8859_1),
  m_quickAccessFrames(FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES),
  m_trackNumberDigits(1),
  m_onlyCustomGenres(false)
{
}

/**
 * Destructor.
 */
TagConfig::~TagConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void TagConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("MarkTruncations"), QVariant(m_markTruncations));
  config->setValue(QLatin1String("EnableTotalNumberOfTracks"), QVariant(m_enableTotalNumberOfTracks));
  config->setValue(QLatin1String("GenreNotNumeric"), QVariant(m_genreNotNumeric));
  config->setValue(QLatin1String("CommentName"), QVariant(m_commentName));
  config->setValue(QLatin1String("PictureNameItem"), QVariant(m_pictureNameItem));
  config->setValue(QLatin1String("CustomGenres"), QVariant(m_customGenres));
  config->setValue(QLatin1String("ID3v2Version"), QVariant(m_id3v2Version));
  config->setValue(QLatin1String("TextEncodingV1"), QVariant(m_textEncodingV1));
  config->setValue(QLatin1String("TextEncoding"), QVariant(m_textEncoding));
  config->setValue(QLatin1String("QuickAccessFrames"), QVariant(m_quickAccessFrames));
  config->setValue(QLatin1String("TrackNumberDigits"), QVariant(m_trackNumberDigits));
  config->setValue(QLatin1String("OnlyCustomGenres"), QVariant(m_onlyCustomGenres));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void TagConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_markTruncations = config->value(QLatin1String("MarkTruncations"), m_markTruncations).toBool();
  m_enableTotalNumberOfTracks = config->value(QLatin1String("EnableTotalNumberOfTracks"), m_enableTotalNumberOfTracks).toBool();
  m_genreNotNumeric = config->value(QLatin1String("GenreNotNumeric"), m_genreNotNumeric).toBool();
  m_commentName = config->value(QLatin1String("CommentName"), QString::fromLatin1(defaultCommentName)).toString();
  m_pictureNameItem = config->value(QLatin1String("PictureNameItem"), VP_METADATA_BLOCK_PICTURE).toInt();
  m_customGenres = config->value(QLatin1String("CustomGenres"),
                                 m_customGenres).toStringList();
  m_id3v2Version = config->value(QLatin1String("ID3v2Version"), ID3v2_3_0).toInt();
  m_textEncodingV1 = config->value(QLatin1String("TextEncodingV1"), QLatin1String("ISO-8859-1")).toString();
  m_textEncoding = config->value(QLatin1String("TextEncoding"), TE_ISO8859_1).toInt();
  m_quickAccessFrames = config->value(QLatin1String("QuickAccessFrames"),
                                     FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES).toUInt();
  m_trackNumberDigits = config->value(QLatin1String("TrackNumberDigits"), 1).toInt();
  m_onlyCustomGenres = config->value(QLatin1String("OnlyCustomGenres"), m_onlyCustomGenres).toBool();
  config->endGroup();
}
