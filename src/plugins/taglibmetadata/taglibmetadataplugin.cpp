/**
 * \file taglibmetadataplugin.cpp
 * TagLib metadata plugin.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Jul 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "taglibmetadataplugin.h"
#include "taglibfile.h"

namespace {

const QLatin1String TAGGEDFILE_KEY("TaglibMetadata");

}

/*!
 * Constructor.
 * @param parent parent object
 */
TaglibMetadataPlugin::TaglibMetadataPlugin(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("TaglibMetadata"));
}

/**
 * Get name of factory, the same as the QObject::objectName() of the plugin.
 * @return factory name.
 */
QString TaglibMetadataPlugin::name() const
{
  return objectName();
}

/**
 * Get keys of available tagged file formats.
 * @return list of keys.
 */
QStringList TaglibMetadataPlugin::taggedFileKeys() const
{
  return {TAGGEDFILE_KEY};
}

/**
 * Get features supported.
 * @param key tagged file key
 * @return bit mask with TaggedFile::Feature flags set.
 */
int TaglibMetadataPlugin::taggedFileFeatures(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return TaggedFile::TF_ID3v11 | TaggedFile::TF_ID3v22 |
        TaggedFile::TF_OggFlac |
        TaggedFile::TF_OggPictures |
        TaggedFile::TF_ID3v23 |
        TaggedFile::TF_ID3v24;
  }
  return 0;
}

/**
 * Initialize tagged file factory.
 *
 * @param key tagged file key
 */
void TaglibMetadataPlugin::initialize(const QString& key)
{
  if (key == TAGGEDFILE_KEY) {
    TagLibFile::staticInit();
  }
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
TaggedFile* TaglibMetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& fileName,
    const QPersistentModelIndex& idx,
    int features)
{
  Q_UNUSED(features)
  if (key == TAGGEDFILE_KEY) {
    QString ext = fileName.right(4).toLower();
    QString ext2 = ext.right(3);
    if (   ext == QLatin1String(".mp3") || ext == QLatin1String(".mp2")
        || ext == QLatin1String(".aac")
        || ext == QLatin1String(".mpc") || ext == QLatin1String(".oga")
        || ext == QLatin1String(".ogg") || ext == QLatin1String("flac")
        || ext == QLatin1String(".spx") || ext == QLatin1String(".tta")
        || ext == QLatin1String(".m4a") || ext == QLatin1String(".m4b")
        || ext == QLatin1String(".m4p") || ext == QLatin1String(".m4r")
        || ext == QLatin1String(".mp4") || ext == QLatin1String(".m4v")
        || ext == QLatin1String("mp4v")
        || ext == QLatin1String(".wma") || ext == QLatin1String(".asf")
        || ext == QLatin1String(".wmv")
        || ext == QLatin1String(".aif") || ext == QLatin1String("aiff")
        || ext == QLatin1String(".wav") || ext == QLatin1String(".ape")
        || ext == QLatin1String(".mod") || ext == QLatin1String(".s3m")
        || ext2 == QLatin1String(".it")
        || ext2 == QLatin1String(".xm")
        || ext == QLatin1String("opus")
        || ext == QLatin1String(".dsf")
        || ext2 == QLatin1String(".wv"))
      return new TagLibFile(idx);
  }
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
TaglibMetadataPlugin::supportedFileExtensions(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return {
      QLatin1String(".flac"), QLatin1String(".mp3"), QLatin1String(".mpc"),
      QLatin1String(".oga"), QLatin1String(".ogg"), QLatin1String(".spx"),
      QLatin1String(".tta"), QLatin1String(".aac"), QLatin1String(".mp2"),
      QLatin1String(".m4a"), QLatin1String(".m4b"), QLatin1String(".m4p"),
      QLatin1String(".m4r"), QLatin1String(".mp4"), QLatin1String(".m4v"),
      QLatin1String(".mp4v"),
      QLatin1String(".wma"), QLatin1String(".asf"), QLatin1String(".wmv"),
      QLatin1String(".aif"), QLatin1String(".aiff"), QLatin1String(".wav"),
      QLatin1String(".ape"),
      QLatin1String(".mod"), QLatin1String(".s3m"), QLatin1String(".it"),
      QLatin1String(".xm"),
      QLatin1String(".opus"),
      QLatin1String(".dsf"),
      QLatin1String(".wv")
    };
  }
  return QStringList();
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 *
 * @param key tagged file key
 */
void TaglibMetadataPlugin::notifyConfigurationChange(const QString& key)
{
  if (key == TAGGEDFILE_KEY) {
    TagLibFile::notifyConfigurationChange();
  }
}
