/**
 * \file scriptutils.h
 * QML support functions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#ifndef SCRIPTUTILS_H
#define SCRIPTUTILS_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QPersistentModelIndex>
#include "trackdata.h"

/**
 * QML support functions.
 */
class KID3_PLUGIN_EXPORT ScriptUtils : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit ScriptUtils(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~ScriptUtils();

  /**
   * Convert a list of URLs to a list of local file paths.
   * @param urls file URLs
   * @return list with local file paths.
   */
  Q_INVOKABLE static QStringList toStringList(const QList<QUrl>& urls);

  /**
   * Convert a variant list containing model indexes to a list of persistent
   * model indexes.
   * @param lst variant list with model indexes
   * @return persistent model index list.
   */
  Q_INVOKABLE static QList<QPersistentModelIndex> toPersistentModelIndexList(
      const QVariantList& lst);

  /**
   * Convert an integer to a tag version.
   * @param nr tag mask (0=none, 1, 2, 3=1 and 2)
   */
  Q_INVOKABLE static Frame::TagVersion toTagVersion(int nr);

  /**
   * Get data for @a roleName and @a row from @a model.
   * @param modelObj model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param parent optional parent model index
   * @return model data.
   */
  Q_INVOKABLE static QVariant getRoleData(QObject* modelObj, int row, const QByteArray& roleName,
      QModelIndex parent = QModelIndex());

  /**
   * Set data for @a roleName and @a row in @a model.
   * @param modelObj model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param value model data
   * @param parent optional parent model index
   * @return true if ok.
   */
  Q_INVOKABLE static bool setRoleData(QObject* modelObj, int row,
      const QByteArray& roleName, const QVariant& value,
      QModelIndex parent = QModelIndex());

  /**
   * Get data for @a roleName and model @a index.
   * @param index model index
   * @param roleName role name as used in scripting languages
   * @return model data.
   */
  Q_INVOKABLE static QVariant getIndexRoleData(const QModelIndex& index,
                                               const QByteArray& roleName);

  /**
   * Get property values as a string.
   * @param obj object to inspect
   * @return string containing property values.
   */
  Q_INVOKABLE static QString properties(QObject* obj);

  /**
   * String list of frame field ID names.
   */
  Q_INVOKABLE static QStringList getFieldIdNames();

  /**
   * String list of text encoding names.
   */
  Q_INVOKABLE static QStringList getTextEncodingNames();

  /**
   * String list of timestamp format names.
   */
  Q_INVOKABLE static QStringList getTimestampFormatNames();

  /**
   * String list of picture type names.
   */
  Q_INVOKABLE static QStringList getPictureTypeNames();

  /**
   * String list of content type names.
   */
  Q_INVOKABLE static QStringList getContentTypeNames();

  /**
   * Write data to a file.
   * @param filePath path to file
   * @param data data to write
   * @return true if ok.
   */
  Q_INVOKABLE static bool writeFile(const QString& filePath,
                                    const QByteArray& data);

  /**
   * Read data from file
   * @param filePath path to file
   * @return data read, empty if failed.
   */
  Q_INVOKABLE static QByteArray readFile(const QString& filePath);

  /**
   * Remove file.
   * @param filePath path to file
   * @return true if ok.
   */
  Q_INVOKABLE static bool removeFile(const QString& filePath);

  /**
   * Check if file exists.
   * @param filePath path to file
   * @return true if file exists.
   */
  Q_INVOKABLE static bool fileExists(const QString& filePath);

  /**
   * Rename file.
   * @param oldName old name
   * @param newName new name
   * @return true if ok.
   */
  Q_INVOKABLE static bool renameFile(const QString& oldName,
                                     const QString& newName);

  /**
   * Get path of temporary directory.
   * @return temporary directory.
   */
  Q_INVOKABLE static QString tempPath();

  /**
   * List directory entries.
   * @param path directory path
   * @param nameFilters list of name filters, e.g. ["*.jpg", "*.png"]
   * @param classify if true, add /, @, * for directories, symlinks, executables
   * @return list of directory entries.
   */
  Q_INVOKABLE static QStringList listDir(
      const QString& path, const QStringList& nameFilters = QStringList(),
      bool classify = false);

  /**
   * Synchronously start a system command.
   * @param program executable
   * @param args arguments
   * @param msecs timeout in milliseconds, -1 for no timeout
   * @return [exit code, standard output, standard error], empty list on timeout.
   */
  Q_INVOKABLE static QVariantList system(
      const QString& program, const QStringList& args = QStringList(),
      int msecs = -1);


  /**
   * Get value of environment variable.
   * @param varName variable name
   * @return value.
   */
  Q_INVOKABLE static QByteArray getEnv(const QByteArray& varName);

  /**
   * Set value of environment variable.
   * @param varName variable name
   * @param value value to set
   * @return true if value could be set.
   */
  Q_INVOKABLE static bool setEnv(const QByteArray& varName,
                                 const QByteArray& value);

  /**
   * Get version of Qt.
   * @return Qt version string, e.g. "5.4.1".
   */
  Q_INVOKABLE static QString getQtVersion();

  /**
   * Get hex string of the MD5 hash of data.
   * This is a replacement for Qt::md5(), which does only work with strings.
   * @param data data bytes
   * @return MD5 sum.
   */
  Q_INVOKABLE static QString getDataMd5(const QByteArray& data);

  /**
   * Get size of byte array.
   * @param data data bytes
   * @return number of bytes in @a data.
   */
  Q_INVOKABLE static int getDataSize(const QByteArray& data);

  /**
   * Create an image from data bytes.
   * @param data data bytes
   * @param format image format, default is "JPG"
   * @return image variant.
   */
  Q_INVOKABLE static QVariant dataToImage(const QByteArray& data,
                                          const QByteArray& format = "JPG");

  /**
   * Get data bytes from image.
   * @param var image variant
   * @param format image format, default is "JPG"
   * @return data bytes.
   */
  Q_INVOKABLE static QByteArray dataFromImage(const QVariant& var,
                                              const QByteArray& format = "JPG");

  /**
   * Load an image from a file.
   * @param filePath path to file
   * @return image variant.
   */
  Q_INVOKABLE static QVariant loadImage(const QString& filePath);

  /**
   * Save an image to a file.
   * @param var image variant
   * @param filePath path to file
   * @param format image format, default is "JPG"
   * @return true if ok.
   */
  Q_INVOKABLE static bool saveImage(const QVariant& var,
                                    const QString& filePath,
                                    const QByteArray& format = "JPG");

  /**
   * Get properties of an image.
   * @param var image variant
   * @return map containing "width", "height", "depth" and "colorCount",
   * empty if invalid image.
   */
  Q_INVOKABLE static QVariantMap imageProperties(const QVariant& var);

  /**
   * Scale an image.
   * @param var image variant
   * @param width scaled width, -1 to keep aspect ratio
   * @param height scaled height, -1 to keep aspect ratio
   * @return scaled image variant.
   */
  Q_INVOKABLE static QVariant scaleImage(const QVariant& var,
                                         int width, int height = -1);

  /**
   * @brief Open a file select dialog to get a file name.
   * @param caption dialog caption
   * @param dir working directory
   * @param filter file type filter
   * @param saveFile true to open a save file dialog
   * @return selected file, empty if canceled.
   */
  Q_INVOKABLE static QString selectFileName(
      const QString& caption = QString(), const QString& dir = QString(),
      const QString& filter = QString(), bool saveFile = false);
};

#endif // SCRIPTUTILS_H
