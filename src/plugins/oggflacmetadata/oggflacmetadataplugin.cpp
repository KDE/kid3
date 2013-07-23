/**
 * \file oggflacmetadataplugin.cpp
 * Ogg/Vorbis & FLAC metadata plugin.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Jul 2013
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

#include "oggflacmetadataplugin.h"
#include "oggfile.hpp"
#include "flacfile.hpp"
#include "tagconfig.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(OggFlacMetadataPlugin, OggFlacMetadataPlugin)
#endif

#ifdef HAVE_VORBIS
static const QLatin1String OGG_KEY("OggMetadata");
#endif
#ifdef HAVE_FLAC
static const QLatin1String FLAC_KEY("FlacMetadata");
#endif

/*!
 * Constructor.
 * @param parent parent object
 */
OggFlacMetadataPlugin::OggFlacMetadataPlugin(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("OggFlacMetadataPlugin"));
}

/**
 * Destructor.
 */
OggFlacMetadataPlugin::~OggFlacMetadataPlugin()
{
}

/**
 * Get keys of available tagged file formats.
 * @return list of keys.
 */
QStringList OggFlacMetadataPlugin::taggedFileKeys() const
{
  return QStringList()
#ifdef HAVE_VORBIS
      << OGG_KEY
#endif
#ifdef HAVE_FLAC
      << FLAC_KEY
#endif
         ;
}

/**
 * Initialize tagged file factory.
 *
 * @param key tagged file key
 */
void OggFlacMetadataPlugin::initialize(const QString& key)
{
#ifdef HAVE_VORBIS
  if (key == OGG_KEY) {
    TagConfig::instance().setTagFormat(TagConfig::TF_VORBIS_LIBOGG);
  }
#else
  Q_UNUSED(key)
#endif
}

/**
 * Create a tagged file.
 *
 * @param key tagged file key
 * @param dirName directory name
 * @param fileName filename
 * @param idx model index
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* OggFlacMetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx)
{
#ifdef HAVE_VORBIS
  if (key == OGG_KEY) {
    QString ext = fileName.right(4).toLower();
    if (ext == QLatin1String(".oga") || ext == QLatin1String(".ogg"))
      return new OggFile(dirName, fileName, idx);
  }
#endif
#ifdef HAVE_FLAC
  if (key == FLAC_KEY) {
    if (fileName.right(5).toLower() == QLatin1String(".flac"))
      return new FlacFile(dirName, fileName, idx);
  }
#endif
  return 0;
}

/**
 * Get a list with all extensions (e.g. ".mp3") supported by TaggedFile subclass.
 *
 * @param key tagged file key
 *
 * @return list of file extensions.
 */
QStringList
OggFlacMetadataPlugin::supportedFileExtensions(const QString& key) const
{
#ifdef HAVE_VORBIS
  if (key == OGG_KEY) {
    return QStringList() << QLatin1String(".oga") << QLatin1String(".ogg");
  }
#endif
#ifdef HAVE_FLAC
  if (key == FLAC_KEY) {
    return QStringList() << QLatin1String(".flac");
  }
#endif
  return QStringList();
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 *
 * @param key tagged file key
 */
void OggFlacMetadataPlugin::notifyConfigurationChange(const QString& key)
{
  Q_UNUSED(key)
}
