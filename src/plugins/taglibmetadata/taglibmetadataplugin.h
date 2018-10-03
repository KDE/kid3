/**
 * \file taglibmetadataplugin.h
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

#ifndef TAGLIBMETADATAPLUGIN_H
#define TAGLIBMETADATAPLUGIN_H

#include <QObject>
#include "itaggedfilefactory.h"

/**
 * TagLib metadata plugin.
 */
class KID3_PLUGIN_EXPORT TaglibMetadataPlugin :
    public QObject, public ITaggedFileFactory {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "net.sourceforge.kid3.ITaggedFileFactory")
  Q_INTERFACES(ITaggedFileFactory)
public:
  /*!
   * Constructor.
   * @param parent parent object
   */
  explicit TaglibMetadataPlugin(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~TaglibMetadataPlugin() override = default;

  /**
   * Get name of factory, the same as the QObject::objectName() of the plugin.
   * @return factory name.
   */
  virtual QString name() const override;

  /**
   * Get keys of available tagged file formats.
   * @return list of keys.
   */
  virtual QStringList taggedFileKeys() const override;

  /**
   * Get features supported.
   * @param key tagged file key
   * @return bit mask with TaggedFile::Feature flags set.
   */
  virtual int taggedFileFeatures(const QString& key) const override;

  /**
   * Initialize tagged file factory.
   *
   * @param key tagged file key
   */
  virtual void initialize(const QString& key) override;

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
  virtual TaggedFile* createTaggedFile(
      const QString& key,
      const QString& fileName,
      const QPersistentModelIndex& idx,
      int features = 0) override;

  /**
   * Get a list with all extensions (e.g. ".mp3") supported by TaggedFile subclass.
   *
   * @param key tagged file key
   *
   * @return list of file extensions.
   */
  virtual QStringList supportedFileExtensions(const QString& key) const override;

  /**
   * Notify about configuration change.
   * This method shall be called when the configuration changes.
   *
   * @param key tagged file key
   */
  virtual void notifyConfigurationChange(const QString& key) override;
};

#endif // TAGLIBMETADATAPLUGIN_H
