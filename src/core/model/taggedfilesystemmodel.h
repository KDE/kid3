/**
 * \file taggedfilesystemmodel.h
 * Filesystem model which additional columns.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08-Aug-2021
 *
 * Copyright (C) 2021  Urs Fleisch
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

#pragma once

#include "filesystemmodel.h"
#include "taggedfile.h"
#include "kid3api.h"

class CoreTaggedFileIconProvider;
class ITaggedFileFactory;

class KID3_CORE_EXPORT TaggedFileSystemModel : public FileSystemModel {
  Q_OBJECT
public:
  /** Number of columns in FileSystemModel. */
  static const int NUM_FILESYSTEM_COLUMNS = 4;

  /** Custom role, extending FileSystemModel::Roles. */
  enum Roles {
    TaggedFileRole = Qt::UserRole + 4,
    IconIdRole = Qt::UserRole + 5,
    TruncatedRole =  Qt::UserRole + 6,
    IsDirRole = Qt::UserRole + 7
  };

  /**
   * Constructor.
   * @param iconProvider icon provider
   * @param parent parent object
   */
  explicit TaggedFileSystemModel(CoreTaggedFileIconProvider* iconProvider,
                                 QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~TaggedFileSystemModel() override;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role = Qt::DisplayRole) const override;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole) override;

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;

  virtual QModelIndex sibling(int row, int column,
                              const QModelIndex& idx) const override;
  virtual int columnCount(
      const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Rename file or directory of @a index to @a newName.
   * @return true if ok
   */
  bool rename(const QModelIndex& index, const QString& newName);

  /**
   * Get icon provider.
   * @return icon provider.
   */
  CoreTaggedFileIconProvider* getIconProvider() const { return m_iconProvider; }

  /**
   * Called from tagged file to notify modification state changes.
   * @param index model index
   * @param modified true if file is modified
   */
  void notifyModificationChanged(const QModelIndex& index, bool modified);

  /**
   * Called from tagged file to notify changes in extra model data, e.g. the
   * information on which the CoreTaggedFileIconProvider depends.
   * @param index model index
   */
  void notifyModelDataChanged(const QModelIndex& index);

  /**
   * Access to tagged file factories.
   * @return reference to tagged file factories.
   */
  static QList<ITaggedFileFactory*>& taggedFileFactories() {
    return s_taggedFileFactories;
  }

  /**
   * Create a tagged file with a given feature.
   *
   * @param feature tagged file feature
   * @param fileName filename
   * @param idx model index
   *
   * @return tagged file, 0 if feature not found or type not supported.
   */
  static TaggedFile* createTaggedFile(
      TaggedFile::Feature feature,
      const QString& fileName,
      const QPersistentModelIndex& idx);

  /**
   * Create a tagged file.
   *
   * @param fileName filename
   * @param idx model index
   *
   * @return tagged file, 0 if not found or type not supported.
   */
  static TaggedFile* createTaggedFile(
      const QString& fileName,
      const QPersistentModelIndex& idx);

  /**
   * Get tagged file data of model index.
   *
   * @param index model index
   * @param taggedFile a TaggedFile pointer is returned here
   *
   * @return true if index has a tagged file, *taggedFile is set to the pointer.
   */
  static bool getTaggedFileOfIndex(const QModelIndex& index,
                                   TaggedFile** taggedFile);

  /**
   * Get tagged file of model index.
   *
   * @param index model index
   *
   * @return tagged file, 0 is returned if the index does not contain a
   * TaggedFile or if has a TaggedFile which is null.
   */
  static TaggedFile* getTaggedFileOfIndex(const QModelIndex& index);

signals:
  /**
   * Emitted when the modification state of a file changes.
   * @param index model index
   * @param modified true if file is modified
   */
  void fileModificationChanged(const QModelIndex& index, bool modified);

protected slots:
  /**
   * Reset internal data of the model.
   * Is called from endResetModel().
   */
#if QT_VERSION >= 0x060000
  virtual void resetInternalData() override;
#else
  void resetInternalData();
#endif

private slots:
  /**
   * Update the TaggedFile contents for rows inserted into the model.
   * @param parent parent model index
   * @param start starting row
   * @param end ending row
   */
  void updateInsertedRows(const QModelIndex& parent, int start, int end);

private:
  /**
   * Retrieve tagged file for an index.
   * @param index model index
   * @return QVariant with tagged file, invalid QVariant if not found.
   */
  QVariant retrieveTaggedFileVariant(const QPersistentModelIndex& index) const;

  /**
   * Store tagged file from variant with index.
   * @param index model index
   * @param value QVariant containing tagged file
   * @return true if index and value valid
   */
  bool storeTaggedFileVariant(const QPersistentModelIndex& index,
                              const QVariant& value);

  /**
   * Clear store with tagged files.
   */
  void clearTaggedFileStore();

  /**
   * Initialize tagged file for model index.
   * @param index model index
   */
  void initTaggedFileData(const QModelIndex& index);

  QHash<QPersistentModelIndex, TaggedFile*> m_taggedFiles;
  QList<Frame::Type> m_tagFrameColumnTypes;
  CoreTaggedFileIconProvider* m_iconProvider;

  static QList<ITaggedFileFactory*> s_taggedFileFactories;
};

Q_DECLARE_METATYPE(TaggedFile*)
