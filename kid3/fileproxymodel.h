/**
 * \file fileproxymodel.h
 * Proxy for filesystem model which filters files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22-Mar-2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef FILEPROXYMODEL_H
#define FILEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QHash>
#include <QSet>
#include <QFileInfo>
#include <QStringList>

class QFileSystemModel;
class TaggedFile;
class TaggedFileIconProvider;

/**
 * Proxy for filesystem model which filters files.
 */
class FileProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	/** Custom role, extending QFileSystemModel::Roles. */
	enum Roles {
		TaggedFileRole = Qt::UserRole + 4
	};

	/**
	 * Constructor.
	 *
	 * @param parent parent object
	 */
	explicit FileProxyModel(QObject* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~FileProxyModel();

	/**
	 * Get number of columns.
	 * @param parent parent model index, invalid for table models
	 * @return number of columns,
	 * if parent is valid number of children (0 for table models)
	 */
	virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;

	/**
	 * Get data for header section.
	 * @param section column or row
	 * @param orientation horizontal or vertical
	 * @param role item data role
	 * @return header data for role
	 */
	virtual QVariant headerData(int section, Qt::Orientation orientation,
															int role=Qt::DisplayRole) const;

	/**
	 * Get data for a given role.
	 * @param index model index
	 * @param role item data role
	 * @return data for role
	 */
	virtual QVariant data(const QModelIndex& index,
												int role=Qt::DisplayRole) const;

	/**
	 * Set data for a given role.
	 * @param index model index
	 * @param value data value
	 * @param role item data role
	 * @return true if successful
	 */
	virtual bool setData(const QModelIndex& index, const QVariant& value,
											 int role=Qt::EditRole);

	/**
	 * Set source model.
	 * @param sourceModel source model, must be QFileSystemModel
	 */
	virtual void setSourceModel(QAbstractItemModel* sourceModel);

	/**
	 * Sets the name filters to apply against the existing files.
	 * @param filters list of strings containing wildcards like "*.mp3"
	 */
	void setNameFilters(const QStringList& filters);

	/**
	 * Filter out a model index.
	 * @param index model index which has to be filtered out
	 */
	void filterOutIndex(const QPersistentModelIndex& index);

	/**
	 * Stop filtering out indexes.
	 */
	void disableFilteringOutIndexes();

	/**
	 * Check if index filter is active.
	 * @return true if indexes are filtered out
	 */
	bool isFilteringOutIndexes() const;

	/**
	 * Make filter changes active after adding indexes to be filtered out.
	 */
	void applyFilteringOutIndexes();

	/**
	 * Emit dataChanged() to the model to force an update of the connected views,
	 * e.g. when the modification state changes.
	 * @param topLeft top left item changed
	 * @param bottomRight bottom right item changed
	 */
	void emitDataChanged(const QModelIndex& topLeft,
											 const QModelIndex& bottomRight);

	/**
	 * Get file information of model index.
	 * @return file information
	 */
	QFileInfo fileInfo(const QModelIndex& index) const;

	/**
	 * Get file path of model index.
	 * @return path to file or directory
	 */
	QString filePath(const QModelIndex& index) const;

	/**
	 * Check if model index represents directory.
	 * @return true if directory
	 */
	bool isDir(const QModelIndex& index) const;

	/**
	 * Delete file of index.
	 * @return true if ok
	 */
	bool remove(const QModelIndex& index) const;

	/**
	 * Delete directory of index.
	 * @return true if ok
	 */
	bool rmdir(const QModelIndex& index) const;

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

	/**
	 * Get directory path if model index is of directory.
	 *
	 * @param index model index
	 *
	 * @return directory path, null if not directory
	 */
	static QString getPathIfIndexOfDir(const QModelIndex& index);

	/**
	 * Release a tagged file or directory index from an index.
	 * If the index has a TaggedFile, it will be deleted.
	 *
	 * @param index model index
	 */
	static void releaseTaggedFileOfIndex(const QModelIndex& index);

	/**
	 * Read tagged file with TagLib.
	 *
	 * @param taggedFile tagged file
	 *
	 * @return tagged file (can be new TagLibFile).
	 */
	static TaggedFile* readWithTagLib(TaggedFile* taggedFile);

	/**
	 * Read tagged file with id3lib.
	 *
	 * @param taggedFile tagged file
	 *
	 * @return tagged file (can be new Mp3File).
	 */
	static TaggedFile* readWithId3Lib(TaggedFile* taggedFile);

	/**
	 * Read file with TagLib if it has an ID3v2.4 tag.
	 *
	 * @param taggedFile tagged file
	 *
	 * @return tagged file (can be new TagLibFile).
	 */
	static TaggedFile* readWithTagLibIfId3V24(TaggedFile* taggedFile);

private slots:
	/**
	 * Update the TaggedFile contents for rows inserted into the model.
	 * @param parent parent model index
	 * @param start starting row
	 * @param end ending row
	 */
	void updateInsertedRows(const QModelIndex& parent, int start, int end);

protected:
	/**
	 * Check if row should be included in model.
	 *
	 * @param srcRow source row
	 * @param srcParent source parent
	 *
	 * @return true to include row.
	 */
	virtual bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const;

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
															QVariant value);

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
	QSet<QPersistentModelIndex> m_filteredOut;
	TaggedFileIconProvider* m_iconProvider;
	QFileSystemModel* m_fsModel;
	QStringList m_extensions;
};

Q_DECLARE_METATYPE(TaggedFile*)

#endif // FILEPROXYMODEL_H
