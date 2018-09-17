/**
 * \file scriptutils.cpp
 * QML support functions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2014
 *
 * Copyright (C) 2014-2017  Urs Fleisch
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

#include "scriptutils.h"
#include <QMetaProperty>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QImage>
#include <QFileDialog>
#include <QBuffer>
#include <QCryptographicHash>
#include "pictureframe.h"
#include "saferename.h"
#include "mainwindowconfig.h"
#include "config.h"

namespace {

/**
 * Create a string list from a NULL terminated array of C strings.
 */
QStringList cstringArrayToStringList(const char* const* strs)
{
  QStringList result;
  while (*strs) {
    result.append(QCoreApplication::translate("@default", *strs++));
  }
  return result;
}

}


ScriptUtils::ScriptUtils(QObject *parent) : QObject(parent)
{
}

ScriptUtils::~ScriptUtils()
{
}

QStringList ScriptUtils::toStringList(const QList<QUrl>& urls)
{
  QStringList paths;
  for (const QUrl& url : urls) {
    paths.append(url.toLocalFile());
  }
  return paths;
}

QList<QPersistentModelIndex> ScriptUtils::toPersistentModelIndexList(const QVariantList& lst)
{
  QList<QPersistentModelIndex> indexes;
  for (const QVariant& var : lst) {
    indexes.append(var.toModelIndex());
  }
  return indexes;
}

QVariant ScriptUtils::getRoleData(
    QObject* modelObj, int row, const QByteArray& roleName,
    QModelIndex parent)
{
  if (QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(modelObj)) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->index(row, 0, parent).data(it.key());
      }
    }
  }
  return QVariant();
}

bool ScriptUtils::setRoleData(
    QObject* modelObj, int row, const QByteArray& roleName,
    const QVariant& value, QModelIndex parent)
{
  if (QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>(modelObj)) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->setData(model->index(row, 0, parent), value, it.key());
      }
    }
  }
  return false;
}

QVariant ScriptUtils::getIndexRoleData(const QModelIndex& index,
                                       const QByteArray& roleName)
{
  if (const QAbstractItemModel* model = index.model()) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return index.data(it.key());
      }
    }
  }
  return QVariant();
}

QString ScriptUtils::properties(QObject* obj)
{
  QString str;
  const QMetaObject* meta;
  if (obj && (meta = obj->metaObject()) != 0) {
    str += QLatin1String("className: ");
    str += QString::fromLatin1(meta->className());
    for (int i = 0; i < meta->propertyCount(); i++) {
      QMetaProperty property = meta->property(i);
      const char* name = property.name();
      QVariant value = obj->property(name);
      str += QLatin1Char('\n');
      str += QString::fromLatin1(name);
      str += QLatin1String(": ");
      str += value.toString();
    }
  }
  return str;
}

/**
 * String list of frame field ID names.
 */
QStringList ScriptUtils::getFieldIdNames()
{
  return cstringArrayToStringList(Frame::Field::getFieldIdNames());
}

/**
 * String list of text encoding names.
 */
QStringList ScriptUtils::getTextEncodingNames()
{
  return cstringArrayToStringList(Frame::Field::getTextEncodingNames());
}

/**
 * String list of timestamp format names.
 */
QStringList ScriptUtils::getTimestampFormatNames()
{
  return cstringArrayToStringList(Frame::Field::getTimestampFormatNames());
}

/**
 * String list of picture type names.
 */
QStringList ScriptUtils::getPictureTypeNames()
{
  return cstringArrayToStringList(PictureFrame::getPictureTypeNames());
}

/**
 * String list of content type names.
 */
QStringList ScriptUtils::getContentTypeNames()
{
  return cstringArrayToStringList(Frame::Field::getContentTypeNames());
}

/**
 * Write data to a file.
 * @param filePath path to file
 * @param data data to write
 * @return true if ok.
 */
bool ScriptUtils::writeFile(const QString& filePath, const QByteArray& data)
{
  bool ok = false;
  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    ok = file.write(data) > 0;
    file.close();
  }
  return ok;
}

/**
 * Read data from file
 * @param filePath path to file
 * @return data read, empty if failed.
 */
QByteArray ScriptUtils::readFile(const QString& filePath)
{
  QByteArray data;
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly)) {
    data = file.readAll();
    file.close();
  }
  return data;
}

/**
 * Remove file.
 * @param filePath path to file
 * @return true if ok.
 */
bool ScriptUtils::removeFile(const QString& filePath)
{
  return QFile::remove(filePath);
}

/**
 * Check if file exists.
 * @param filePath path to file
 * @return true if file exists.
 */
bool ScriptUtils::fileExists(const QString& filePath)
{
  return QFile::exists(filePath);
}

/**
 * Rename file.
 * @param oldName old name
 * @param newName new name
 * @return true if ok.
 */
bool ScriptUtils::renameFile(const QString& oldName, const QString& newName)
{
  return Utils::safeRename(oldName, newName);
}

/**
 * Copy file.
 * @param source path to source file
 * @param dest path to destination file
 * @return true if ok.
 */
bool ScriptUtils::copyFile(const QString& source, const QString& dest)
{
  return QFile::copy(source, dest);
}

/**
 * Create directory.
 * @param path path to new directory
 * @return true if ok.
 */
bool ScriptUtils::makeDir(const QString& path)
{
  return QDir().mkpath(path);
}

/**
 * Remove directory.
 * @param path path to directory to remove
 * @return true if ok.
 */
bool ScriptUtils::removeDir(const QString& path)
{
  return QDir().rmpath(path);
}

/**
 * Get path of temporary directory.
 * @return temporary directory.
 */
QString ScriptUtils::tempPath()
{
  return QDir::tempPath();
}

/**
 * List directory entries.
 * @param path directory path
 * @param nameFilters list of name filters, e.g. ["*.jpg", "*.png"]
 * @param classify if true, add /, @, * for directories, symlinks, executables
 * @return list of directory entries.
 */
QStringList ScriptUtils::listDir(
    const QString& path, const QStringList& nameFilters, bool classify)
{
  QStringList dirList;
  const QFileInfoList entries = QDir(path).entryInfoList(nameFilters);
  for (const QFileInfo& fi : entries) {
    QString fileName = fi.fileName();
    if (classify) {
      if (fi.isDir()) fileName += QLatin1Char('/');
      else if (fi.isSymLink()) fileName += QLatin1Char('@');
      else if (fi.isExecutable()) fileName += QLatin1Char('*');
    }
    dirList.append(fileName);
  }
  return dirList;
}

/**
 * Synchronously start a system command.
 * @param program executable
 * @param args arguments
 * @param msecs timeout in milliseconds, -1 for no timeout
 * @return [exit code, standard output, standard error], empty list on timeout.
 */
QVariantList ScriptUtils::system(
    const QString& program, const QStringList& args, int msecs)
{
  QProcess proc;
  proc.start(program, args);
  if (proc.waitForFinished(msecs)) {
    return QVariantList()
        << proc.exitCode()
        << QString::fromLocal8Bit(proc.readAllStandardOutput())
        << QString::fromLocal8Bit(proc.readAllStandardError());
  }
  return QVariantList();
}

/**
 * Get value of environment variable.
 * @param varName variable name
 * @return value.
 */
QByteArray ScriptUtils::getEnv(const QByteArray& varName)
{
  return qgetenv(varName.constData());
}

/**
 * Set value of environment variable.
 * @param varName variable name
 * @param value value to set
 * @return true if value could be set.
 */
bool ScriptUtils::setEnv(const QByteArray& varName, const QByteArray& value)
{
  return qputenv(varName, value);
}

/**
 * Get version of Kid3.
 * @return Kid3 version string, e.g. "3.3.0".
 */
QString ScriptUtils::getKid3Version()
{
  return QLatin1String(VERSION);
}

/**
 * Get release year of Kid3.
 * @return Kid3 year string, e.g. "2015".
 */
QString ScriptUtils::getKid3ReleaseYear()
{
  return QLatin1String(RELEASE_YEAR);
}

/**
 * Get version of Qt.
 * @return Qt version string, e.g. "5.4.1".
 */
QString ScriptUtils::getQtVersion()
{
  return QString::fromLatin1(qVersion());
}

/**
 * Get hex string of the MD5 hash of data.
 * This is a replacement for Qt::md5(), which does only work with strings.
 * @param data data bytes
 * @return MD5 sum.
 */
QString ScriptUtils::getDataMd5(const QByteArray& data)
{
  QByteArray result = QCryptographicHash::hash(data, QCryptographicHash::Md5);
  return QLatin1String(result.toHex());
}

/**
 * Get size of byte array.
 * @param data data bytes
 * @return number of bytes in @a data.
 */
int ScriptUtils::getDataSize(const QByteArray& data)
{
  return data.size();
}

/**
 * Create an image from data bytes.
 * @param data data bytes
 * @param format image format, default is "JPG"
 * @return image variant.
 */
QVariant ScriptUtils::dataToImage(const QByteArray& data,
                                  const QByteArray& format)
{
  QImage img(QImage::fromData(data, format.constData()));
  return QVariant::fromValue(img);
}

/**
 * Get data bytes from image.
 * @param var image variant
 * @param format image format, default is "JPG"
 * @return data bytes.
 */
QByteArray ScriptUtils::dataFromImage(const QVariant& var,
                                      const QByteArray& format)
{
  QByteArray data;
  QImage img(var.value<QImage>());
  if (!img.isNull()) {
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, format.constData());
  }
  return data;
}

/**
 * Load an image from a file.
 * @param filePath path to file
 * @return image variant.
 */
QVariant ScriptUtils::loadImage(const QString& filePath)
{
  QImage img(filePath);
  return QVariant::fromValue(img);
}

/**
 * Save an image to a file.
 * @param var image variant
 * @param filePath path to file
 * @param format image format, default is "JPG"
 * @return true if ok.
 */
bool ScriptUtils::saveImage(const QVariant& var, const QString& filePath,
                            const QByteArray& format)
{
  QImage img(var.value<QImage>());
  if (!img.isNull()) {
    return img.save(filePath, format.constData());
  }
  return false;
}

/**
 * Get properties of an image.
 * @param var image variant
 * @return map containing "width", "height", "depth" and "colorCount",
 * empty if invalid image.
 */
QVariantMap ScriptUtils::imageProperties(const QVariant& var)
{
  QVariantMap map;
  QImage img(var.value<QImage>());
  if (!img.isNull()) {
    map.insert(QLatin1String("width"), img.width());
    map.insert(QLatin1String("height"), img.height());
    map.insert(QLatin1String("depth"), img.depth());
    map.insert(QLatin1String("colorCount"), img.colorCount());
  }
  return map;
}

/**
 * Scale an image.
 * @param var image variant
 * @param width scaled width, -1 to keep aspect ratio
 * @param height scaled height, -1 to keep aspect ratio
 * @return scaled image variant.
 */
QVariant ScriptUtils::scaleImage(const QVariant& var, int width, int height)
{
  QImage img(var.value<QImage>());
  if (!img.isNull()) {
    if (width > 0 && height > 0) {
      return QVariant::fromValue(img.scaled(width, height,
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else if (width > 0) {
      return QVariant::fromValue(img.scaledToWidth(width,
                                                   Qt::SmoothTransformation));
    } else if (height > 0) {
      return QVariant::fromValue(img.scaledToHeight(height,
                                                    Qt::SmoothTransformation));
    }
  }
  return QVariant();
}

/**
 * @brief Open a file select dialog to get a file name.
 * @param caption dialog caption
 * @param dir working directory
 * @param filter file type filter
 * @param saveFile true to open a save file dialog
 * @return selected file, empty if canceled.
 */
QString ScriptUtils::selectFileName(const QString& caption, const QString& dir,
                                    const QString& filter, bool saveFile)
{
  QFileDialog::Options options =
      MainWindowConfig::instance().dontUseNativeDialogs()
      ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0);
  return saveFile ? QFileDialog::getSaveFileName(0, caption, dir, filter,
                                                 0, options)
                  : QFileDialog::getOpenFileName(0, caption, dir, filter,
                                                 0, options);
}

/**
 * @brief Open a file select dialog to get a directory name.
 * @param caption dialog caption
 * @param dir working directory
 * @return selected directory, empty if canceled.
 */
QString ScriptUtils::selectDirName(const QString& caption, const QString& dir)
{
  QFileDialog::Options options =
      MainWindowConfig::instance().dontUseNativeDialogs()
      ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0);
  return QFileDialog::getExistingDirectory(0, caption, dir, options);
}
