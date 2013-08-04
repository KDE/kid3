/**
 * \file id3libmetadataplugin.cpp
 * id3lib metadata plugin.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jul 2013
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

#include "id3libmetadataplugin.h"
#include "mp3file.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Id3libMetadataPlugin, Id3libMetadataPlugin)
#endif

static const QLatin1String TAGGEDFILE_KEY("Id3libMetadata");

/*!
 * Constructor.
 * @param parent parent object
 */
Id3libMetadataPlugin::Id3libMetadataPlugin(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("Id3libMetadata"));
}

/**
 * Destructor.
 */
Id3libMetadataPlugin::~Id3libMetadataPlugin()
{
}

/**
 * Get name of factory, the same as the QObject::objectName() of the plugin.
 * @return factory name.
 */
QString Id3libMetadataPlugin::name() const
{
  return objectName();
}

/**
 * Get keys of available tagged file formats.
 * @return list of keys.
 */
QStringList Id3libMetadataPlugin::taggedFileKeys() const
{
  return QStringList() << TAGGEDFILE_KEY;
}

/**
 * Get features supported.
 * @param key tagged file key
 * @return bit mask with TaggedFile::Feature flags set.
 */
int Id3libMetadataPlugin::taggedFileFeatures(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return TaggedFile::TF_ID3v11 | TaggedFile::TF_ID3v23;
  }
  return 0;
}

/**
 * Initialize tagged file factory.
 *
 * @param key tagged file key
 */
void Id3libMetadataPlugin::initialize(const QString& key)
{
  Q_UNUSED(key);
}

/**
 * Create a tagged file.
 *
 * @param key tagged file key
 * @param dirName directory name
 * @param fileName filename
 * @param idx model index
 * @param features optional tagged file features (TaggedFile::Feature flags)
 * to activate at creation
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* Id3libMetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx,
    int features)
{
  if (key == TAGGEDFILE_KEY) {
    QString ext = fileName.right(4).toLower();
    if ((ext == QLatin1String(".mp3") || ext == QLatin1String(".mp2") ||
         ext == QLatin1String(".aac")) &&
        (TagConfig::instance().id3v2Version() == TagConfig::ID3v2_3_0 ||
         (features & TaggedFile::TF_ID3v23) != 0)) {
      return new Mp3File(dirName, fileName, idx);
    }
  }
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
Id3libMetadataPlugin::supportedFileExtensions(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return QStringList() << QLatin1String(".mp3") << QLatin1String(".mp2")
                         << QLatin1String(".aac");
  }
  return QStringList();
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 *
 * @param key tagged file key
 */
void Id3libMetadataPlugin::notifyConfigurationChange(const QString& key)
{
  if (key == TAGGEDFILE_KEY) {
    Mp3File::notifyConfigurationChange();
  }
}
