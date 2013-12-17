/**
 * \file scriptinterface.cpp
 * D-Bus script adaptor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Dec 2007
 *
 * Copyright (C) 2007-2013  Urs Fleisch
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

#include "scriptinterface.h"
#ifdef HAVE_QTDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QFileInfo>
#include <QCoreApplication>
#include <QItemSelectionModel>
#include "kid3application.h"
#include "taggedfile.h"
#include "frametablemodel.h"
#include "filefilter.h"
#include "pictureframe.h"
#include "fileproxymodel.h"
#include "modeliterator.h"
#include "batchimportconfig.h"
#include "batchimportprofile.h"

/**
 * Constructor.
 *
 * @param app parent application
 */
ScriptInterface::ScriptInterface(Kid3Application* app) :
  QDBusAbstractAdaptor(app), m_app(app)
{
  setObjectName(QLatin1String("ScriptInterface"));
  setAutoRelaySignals(true);
}

/**
 * Destructor.
 */
ScriptInterface::~ScriptInterface()
{
}

/**
 * Open file or directory.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool ScriptInterface::openDirectory(const QString& path)
{
  return m_app->openDirectory(QStringList() << path, true);
}

/**
 * Save all modified files.
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::save()
{
  QStringList errorFiles = m_app->saveDirectory();
  if (errorFiles.isEmpty()) {
    m_errorMsg.clear();
    return true;
  } else {
    m_errorMsg = QLatin1String("Error while writing file:\n") + errorFiles.join(QLatin1String("\n"));
    return false;
  }
}

/**
 * Get a detailed error message provided by some methods.
 *
 * @return detailed error message.
 */
QString ScriptInterface::getErrorMessage() const
{
  return m_errorMsg;
}

/**
 * Revert changes in the selected files.
 */
void ScriptInterface::revert()
{
  m_app->revertFileModifications();
}

/**
 * Import tags from a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file, "clipboard" for import from clipboard
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::importFromFile(int tagMask, const QString& path,
                                     int fmtIdx)
{
  return m_app->importTags(TrackData::tagVersionCast(tagMask), path, fmtIdx);
}

/**
 * Start an automatic batch import.
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param profileName name of batch import profile to use
 *
 * @return true if profile found.
 */
bool ScriptInterface::batchImport(int tagMask, const QString& profileName)
{
  BatchImportProfile profile;
  if (BatchImportConfig::instance().getProfileByName(profileName, profile)) {
    m_app->batchImport(profile, TrackData::tagVersionCast(tagMask));
    return true;
  }
  return false;
}

/**
 * Download album cover art into the picture frame of the selected files.
 *
 * @param url           URL of picture file or album art resource
 * @param allFilesInDir true to add the image to all files in the directory
 */
void ScriptInterface::downloadAlbumArt(const QString& url, bool allFilesInDir)
{
  m_app->downloadImage(url, allFilesInDir
    ? Kid3Application::ImageForAllFilesInDirectory
    : Kid3Application::ImageForSelectedFiles);
}

/**
 * Export tags to a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file, "clipboard" for export to clipboard
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::exportToFile(int tagMask, const QString& path, int fmtIdx)
{
  return m_app->exportTags(TrackData::tagVersionCast(tagMask), path, fmtIdx);
}

/**
 * Create a playlist.
 *
 * @return true if ok.
 */
bool ScriptInterface::createPlaylist()
{
  return m_app->writePlaylist();
}

/**
 * Quit the application.
 */
void ScriptInterface::quit()
{
  selectAll();
  revert();
  QCoreApplication::quit();
}

/**
 * Select all files.
 */
void ScriptInterface::selectAll()
{
  m_app->selectAllFiles();
}

/**
 * Deselect all files.
 */
void ScriptInterface::deselectAll()
{
  m_app->deselectAllFiles();
}

/**
 * Set the first file as the current file.
 *
 * @return true if there is a first file.
 */
bool ScriptInterface::firstFile()
{
  return m_app->firstFile(false);
}

/**
 * Set the previous file as the current file.
 *
 * @return true if there is a previous file.
 */
bool ScriptInterface::previousFile()
{
  return m_app->previousFile(false);
}

/**
 * Set the next file as the current file.
 *
 * @return true if there is a next file.
 */
bool ScriptInterface::nextFile()
{
  return m_app->nextFile(false);
}

/**
 * Select the first file.
 *
 * @return true if there is a first file.
 */
bool ScriptInterface::selectFirstFile()
{
  return m_app->firstFile(true);
}

/**
 * Select the previous file.
 *
 * @return true if there is a previous file.
 */
bool ScriptInterface::selectPreviousFile()
{
  return m_app->previousFile(true);
}

/**
 * Select the next file.
 *
 * @return true if there is a next file.
 */
bool ScriptInterface::selectNextFile()
{
  return m_app->nextFile(true);
}

/**
 * Select the current file.
 *
 * @return true if there is a current file.
 */
bool ScriptInterface::selectCurrentFile()
{
 return m_app->selectCurrentFile(true);
}

/**
 * Expand the current file item if it is a directory.
 * A file list item is a directory if getFileName() returns a name with
 * '/' as the last character.
 * The directory is fetched but not expanded in the GUI. To expand it in the
 * GUI, call nextFile() or selectNextFile() after expandDirectory().
 *
 * @return true if current file item is a directory.
 */
bool ScriptInterface::expandDirectory()
{
  QModelIndex index(m_app->getFileSelectionModel()->currentIndex());
  if (!FileProxyModel::getPathIfIndexOfDir(index).isNull()) {
    m_app->expandDirectory(index);
    return true;
  }
  return false;
}

/**
 * Apply the file name format.
 */
void ScriptInterface::applyFilenameFormat()
{
  m_app->applyFilenameFormat();
}

/**
 * Apply the tag format.
 */
void ScriptInterface::applyTagFormat()
{
  m_app->applyId3Format();
}

/**
 * Apply text encoding.
 */
void ScriptInterface::applyTextEncoding()
{
  m_app->applyTextEncoding();
}

/**
 * Set the directory name from the tags.
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param format  directory name format
 * @param create  true to create, false to rename
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::setDirNameFromTag(int tagMask, const QString& format,
                                        bool create)
{
  connect(m_app, SIGNAL(renameActionsScheduled()),
          this, SLOT(onRenameActionsScheduled()));
  if (m_app->renameDirectory(TrackData::tagVersionCast(tagMask), format,
                             create)) {
    return true;
  } else {
    disconnect(m_app, SIGNAL(renameActionsScheduled()),
               this, SLOT(onRenameActionsScheduled()));
    return false;
  }
}

void ScriptInterface::onRenameActionsScheduled()
{
  disconnect(m_app, SIGNAL(renameActionsScheduled()),
             this, SLOT(onRenameActionsScheduled()));
  m_errorMsg = m_app->performRenameActions();
  if (!m_errorMsg.isEmpty()) {
    m_errorMsg = QLatin1String("Error while renaming:\n") + m_errorMsg;
  }
}

/**
 * Set subsequent track numbers in the selected files.
 *
 * @param tagMask      tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param firstTrackNr number to use for first file
 */
void ScriptInterface::numberTracks(int tagMask, int firstTrackNr)
{
  m_app->numberTracks(firstTrackNr, 0, TrackData::tagVersionCast(tagMask));
}

/**
 * Filter the files.
 *
 * @param expression filter expression
 */
void ScriptInterface::filter(const QString& expression)
{
  m_app->applyFilter(expression);
}

/**
 * Convert ID3v2.3 tags to ID3v2.4.
 */
void ScriptInterface::convertToId3v24()
{
  m_app->convertToId3v24();
}

/**
 * Convert ID3v2.4 tags to ID3v2.3.
 */
void ScriptInterface::convertToId3v23()
{
  m_app->convertToId3v23();
}

/**
 * Get path of directory.
 *
 * @return absolute path of directory.
 */
QString ScriptInterface::getDirectoryName()
{
  return m_app->getDirPath();
}

/**
 * Get name of current file.
 *
 * @return absolute file name, ends with "/" if it is a directory.
 */
QString ScriptInterface::getFileName()
{
  return m_app->getFileNameOfSelectedFile();
}

/**
 * Set name of selected file.
 * The file will be renamed when the directory is saved.
 *
 * @param name file name.
 */
void ScriptInterface::setFileName(const QString& name)
{
  m_app->setFileNameOfSelectedFile(name);
}

/**
 * Set format to use when setting the filename from the tags.
 *
 * @param format file name format
 * @see setFileNameFromTag()
 */
void ScriptInterface::setFileNameFormat(const QString& format)
{
  m_app->setTagsToFilenameFormat(format);
}

/**
 * Set the file names of the selected files from the tags.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @see setFileNameFormat()
 */
void ScriptInterface::setFileNameFromTag(int tagMask)
{
  m_app->getFilenameFromTags(TrackData::tagVersionCast(tagMask));
}

/**
 * Get value of frame.
 * To get binary data like a picture, the name of a file to write can be
 * added after the @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 */
QString ScriptInterface::getFrame(int tagMask, const QString& name)
{
  return m_app->getFrame(TrackData::tagVersionCast(tagMask), name);
}

/**
 * Set value of frame.
 * For tag 2 (@a tagMask 2), if no frame with @a name exists, a new frame
 * is added, if @a value is empty, the frame is deleted.
 * To add binary data like a picture, a file can be added after the
 * @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 * @param value   value of frame
 */
bool ScriptInterface::setFrame(int tagMask, const QString& name,
                           const QString& value)
{
  return m_app->setFrame(TrackData::tagVersionCast(tagMask), name, value);
}

/**
 * Get all frames of a tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 *
 * @return list with alternating frame names and values.
 */
QStringList ScriptInterface::getTag(int tagMask)
{
  QStringList lst;
  FrameTableModel* ft = (tagMask & 2) ? m_app->frameModelV2() :
    m_app->frameModelV1();
  for (FrameCollection::const_iterator it = ft->frames().begin();
       it != ft->frames().end();
       ++it) {
    lst << it->getName();
    lst << it->getValue();
  }
  return lst;
}

/**
 * Get technical information about file.
 * Properties are Format, Bitrate, Samplerate, Channels, Duration,
 * Channel Mode, VBR, Tag 1, Tag 2.
 * Properties which are not available are omitted.
 *
 * @return list with alternating property names and values.
 */
QStringList ScriptInterface::getInformation()
{
  QStringList lst;
  QModelIndex index = m_app->getFileSelectionModel()->currentIndex();
  if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
    TaggedFile::DetailInfo info;
    taggedFile->getDetailInfo(info);
    if (info.valid) {
      lst << QLatin1String("Format") << info.format;
      if (info.bitrate > 0 && info.bitrate < 999) {
        lst << QLatin1String("Bitrate") << QString::number(info.bitrate);
      }
      if (info.sampleRate > 0) {
        lst << QLatin1String("Samplerate") << QString::number(info.sampleRate);
      }
      if (info.channels > 0) {
        lst << QLatin1String("Channels") << QString::number(info.channels);
      }
      if (info.duration > 0) {
        lst << QLatin1String("Duration") << QString::number(info.duration);
      }
      if (info.channelMode == TaggedFile::DetailInfo::CM_Stereo ||
          info.channelMode == TaggedFile::DetailInfo::CM_JointStereo) {
        lst << QLatin1String("Channel Mode") <<
          (info.channelMode == TaggedFile::DetailInfo::CM_Stereo ?
           QLatin1String("Stereo") : QLatin1String("Joint Stereo"));
      }
      if (info.vbr) {
        lst << QLatin1String("VBR") << QLatin1String("1");
      }
    }
    QString tag1 = taggedFile->getTagFormatV1();
    if (!tag1.isEmpty()) {
      lst << QLatin1String("Tag 1") << tag1;
    }
    QString tag2 = taggedFile->getTagFormatV2();
    if (!tag2.isEmpty()) {
      lst << QLatin1String("Tag 2") << tag2;
    }
  }
  return lst;
}

/**
 * Set tag from file name.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromFileName(int tagMask)
{
  m_app->getTagsFromFilename(TrackData::tagVersionCast(tagMask));
}

/**
 * Set tag from other tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromOtherTag(int tagMask)
{
  m_app->copyToOtherTag(TrackData::tagVersionCast(tagMask));
}

/**
 * Copy tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::copyTag(int tagMask)
{
  m_app->copyTags(TrackData::tagVersionCast(tagMask));
}

/**
 * Paste tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::pasteTag(int tagMask)
{
  m_app->pasteTags(TrackData::tagVersionCast(tagMask));
}

/**
 * Remove tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::removeTag(int tagMask)
{
  m_app->removeTags(TrackData::tagVersionCast(tagMask));
}

/**
 * Reparse the configuration.
 * Automated configuration changes are possible by modifying
 * the configuration file and then reparsing the configuration.
 */
void ScriptInterface::reparseConfiguration()
{
  m_app->readConfig();
}

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
/**
 * Play selected audio files.
 */
void ScriptInterface::playAudio()
{
  m_app->playAudio();
}
#endif

#endif // HAVE_QTDBUS
