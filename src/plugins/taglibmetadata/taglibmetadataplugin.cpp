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
#include "taglibutils.h"

namespace {

const QLatin1String TAGGEDFILE_KEY("TaglibMetadata");

}

QSet<QString> TaglibMetadataPlugin::s_supportedFileExtensions;

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
    const TagLib::StringList extensions = TagLib::FileRef::defaultFileExtensions();
    for (const auto& extension : extensions) {
      s_supportedFileExtensions.insert(
        TagLibUtils::toQString(extension).prepend(QLatin1Char('.')));
    }
    // Add missing file extensions. The last four are only missing in TagLib 1.
    s_supportedFileExtensions.unite(QSet<QString>({
      QLatin1String(".mp4v"), QLatin1String(".wmv"), QLatin1String(".mp2"),
      QLatin1String(".aac"), QLatin1String(".dsf"), QLatin1String(".dff")
    }));
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
    if (auto dotPos = fileName.lastIndexOf(QLatin1Char('.'));
        dotPos != -1) {
      QString ext = fileName.mid(dotPos);
      if (s_supportedFileExtensions.contains(ext)) {
        return new TagLibFile(idx);
      }
    }
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
#if QT_VERSION >= 0x050e00
    return {s_supportedFileExtensions.constBegin(),
            s_supportedFileExtensions.constEnd()};
#else
    return s_supportedFileExtensions.toList();
#endif
  }
  return {};
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
