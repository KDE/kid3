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
  setObjectName(QLatin1String("OggFlacMetadata"));
}

/**
 * Destructor.
 */
OggFlacMetadataPlugin::~OggFlacMetadataPlugin()
{
}

/**
 * Get name of factory, the same as the QObject::objectName() of the plugin.
 * @return factory name.
 */
QString OggFlacMetadataPlugin::name() const
{
  return objectName();
}

/**
 * Get keys of available tagged file formats.
 * @return list of keys.
 */
QStringList OggFlacMetadataPlugin::taggedFileKeys() const
{
  return {
#ifdef HAVE_VORBIS
      OGG_KEY,
#endif
#ifdef HAVE_FLAC
      FLAC_KEY,
#endif
  };
}

/**
 * Get features supported.
 * @param key tagged file key
 * @return bit mask with TaggedFile::Feature flags set.
 */
int OggFlacMetadataPlugin::taggedFileFeatures(const QString& key) const
{
#ifdef HAVE_VORBIS
  if (key == OGG_KEY) {
    return TaggedFile::TF_OggPictures;
  }
#else
  Q_UNUSED(key)
#endif
  return 0;
}

/**
 * Initialize tagged file factory.
 *
 * @param key tagged file key
 */
void OggFlacMetadataPlugin::initialize(const QString& key)
{
  Q_UNUSED(key)
}

/**
 * Create a tagged file.
 *
 * @param key tagged file key
 * @param fileName filename
 * @param idx model index
 * @param features optional tagged file features (TaggedFile::Feature flags)
 * to activate at creation
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* OggFlacMetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& fileName,
    const QPersistentModelIndex& idx,
    int features)
{
  Q_UNUSED(features)
#ifdef HAVE_VORBIS
  if (key == OGG_KEY) {
    QString ext = fileName.right(4).toLower();
    if (ext == QLatin1String(".oga") || ext == QLatin1String(".ogg"))
      return new OggFile(idx);
  }
#endif
#ifdef HAVE_FLAC
  if (key == FLAC_KEY) {
    if (fileName.right(5).toLower() == QLatin1String(".flac"))
      return new FlacFile(idx);
  }
#endif
  return nullptr;
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
    return {QLatin1String(".oga"), QLatin1String(".ogg")};
  }
#endif
#ifdef HAVE_FLAC
  if (key == FLAC_KEY) {
    return {QLatin1String(".flac")};
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
