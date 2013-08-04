/**
 * \file mp4v2metadataplugin.cpp
 * Mp4v2 metadata plugin.
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

#include "mp4v2metadataplugin.h"
#include "m4afile.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Mp4v2MetadataPlugin, Mp4v2MetadataPlugin)
#endif

static const QLatin1String TAGGEDFILE_KEY("Mp4v2Metadata");

/*!
 * Constructor.
 * @param parent parent object
 */
Mp4v2MetadataPlugin::Mp4v2MetadataPlugin(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("Mp4v2Metadata"));
}

/**
 * Destructor.
 */
Mp4v2MetadataPlugin::~Mp4v2MetadataPlugin()
{
}

/**
 * Get name of factory, the same as the QObject::objectName() of the plugin.
 * @return factory name.
 */
QString Mp4v2MetadataPlugin::name() const
{
  return objectName();
}

/**
 * Get keys of available tagged file formats.
 * @return list of keys.
 */
QStringList Mp4v2MetadataPlugin::taggedFileKeys() const
{
  return QStringList() << TAGGEDFILE_KEY;
}

/**
 * Get features supported.
 * @param key tagged file key
 * @return bit mask with Features flags set.
 */
int Mp4v2MetadataPlugin::taggedFileFeatures(const QString& key) const
{
  Q_UNUSED(key)
  return 0;
}

/**
 * Initialize tagged file factory.
 *
 * @param key tagged file key
 */
void Mp4v2MetadataPlugin::initialize(const QString& key)
{
  Q_UNUSED(key)
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
TaggedFile* Mp4v2MetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx,
    int features)
{
  Q_UNUSED(features)
  if (key == TAGGEDFILE_KEY) {
    QString ext = fileName.right(4).toLower();
    if (ext == QLatin1String(".m4a") || ext == QLatin1String(".m4b") ||
        ext == QLatin1String(".m4p") || ext == QLatin1String(".mp4") ||
        ext == QLatin1String(".m4v") || ext == QLatin1String("mp4v"))
      return new M4aFile(dirName, fileName, idx);
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
Mp4v2MetadataPlugin::supportedFileExtensions(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return QStringList() << QLatin1String(".m4a") << QLatin1String(".m4b")
                         << QLatin1String(".m4p") << QLatin1String(".mp4");
  }
  return QStringList();
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 *
 * @param key tagged file key
 */
void Mp4v2MetadataPlugin::notifyConfigurationChange(const QString& key)
{
  Q_UNUSED(key)
}
