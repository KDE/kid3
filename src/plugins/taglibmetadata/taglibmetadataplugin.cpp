/**
 * \file taglibmetadataplugin.cpp
 * TagLib metadata plugin.
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

#include "taglibmetadataplugin.h"
#include "taglibfile.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TaglibMetadataPlugin, TaglibMetadataPlugin)
#endif

static const QLatin1String TAGGEDFILE_KEY("TaglibMetadata");

/*!
 * Constructor.
 * @param parent parent object
 */
TaglibMetadataPlugin::TaglibMetadataPlugin(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("TaglibMetadata"));
}

/**
 * Destructor.
 */
TaglibMetadataPlugin::~TaglibMetadataPlugin()
{
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
  return QStringList() << TAGGEDFILE_KEY;
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
#if TAGLIB_VERSION >= 0x010700
        TaggedFile::TF_OggPictures |
#endif
#if TAGLIB_VERSION >= 0x010800
        TaggedFile::TF_ID3v23 |
#endif
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
 * @param dirName directory name
 * @param fileName filename
 * @param idx model index
 * @param features optional tagged file features (TaggedFile::Feature flags)
 * to activate at creation
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* TaglibMetadataPlugin::createTaggedFile(
    const QString& key,
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx,
    int features)
{
#if TAGLIB_VERSION >= 0x010800
  Q_UNUSED(features)
#endif
  if (key == TAGGEDFILE_KEY) {
    QString ext = fileName.right(4).toLower();
    QString ext2 = ext.right(3);
    if (((ext == QLatin1String(".mp3") || ext == QLatin1String(".mp2") || ext == QLatin1String(".aac"))
#if TAGLIB_VERSION < 0x010800
         && (TagConfig::instance().id3v2Version() == TagConfig::ID3v2_4_0 ||
             (features & TaggedFile::TF_ID3v24) != 0)
#endif
          )
        || ext == QLatin1String(".mpc") || ext == QLatin1String(".oga") || ext == QLatin1String(".ogg") || ext == QLatin1String("flac")
        || ext == QLatin1String(".spx") || ext == QLatin1String(".tta")
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
        || ext == QLatin1String(".m4a") || ext == QLatin1String(".m4b") || ext == QLatin1String(".m4p") || ext == QLatin1String(".mp4")
#endif
#ifdef TAGLIB_WITH_ASF
        || ext == QLatin1String(".wma") || ext ==  QLatin1String(".asf")
#endif
        || ext == QLatin1String(".aif") || ext ==  QLatin1String("aiff") || ext ==  QLatin1String(".wav")
#endif
#if TAGLIB_VERSION >= 0x010700
        || ext == QLatin1String(".ape")
#endif
#if TAGLIB_VERSION >= 0x010800
        || ext == QLatin1String(".mod") || ext == QLatin1String(".s3m") || ext2 == QLatin1String(".it")
#ifdef HAVE_TAGLIB_XM_SUPPORT
        || ext2 == QLatin1String(".xm")
#endif
#endif
#if TAGLIB_VERSION >= 0x010900
        || ext == QLatin1String("opus")
#endif
        || ext2 == QLatin1String(".wv"))
      return new TagLibFile(dirName, fileName, idx);
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
TaglibMetadataPlugin::supportedFileExtensions(const QString& key) const
{
  if (key == TAGGEDFILE_KEY) {
    return QStringList() << QLatin1String(".flac") << QLatin1String(".mp3") << QLatin1String(".mpc") << QLatin1String(".oga") << QLatin1String(".ogg") <<
      QLatin1String(".spx") << QLatin1String(".tta") << QLatin1String(".aac") << QLatin1String(".mp2") <<
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      QLatin1String(".m4a") << QLatin1String(".m4b") << QLatin1String(".m4p") << QLatin1String(".mp4") <<
#endif
#ifdef TAGLIB_WITH_ASF
      QLatin1String(".wma") << QLatin1String(".asf") <<
#endif
      QLatin1String(".aif") << QLatin1String(".aiff") << QLatin1String(".wav") <<
#endif
#if TAGLIB_VERSION >= 0x010700
      QLatin1String(".ape") <<
#endif
#if TAGLIB_VERSION >= 0x010800
      QLatin1String(".mod") << QLatin1String(".s3m") << QLatin1String(".it") <<
#ifdef HAVE_TAGLIB_XM_SUPPORT
      QLatin1String(".xm") <<
#endif
#endif
#if TAGLIB_VERSION >= 0x010900
      QLatin1String(".opus") <<
#endif
      QLatin1String(".wv");
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
