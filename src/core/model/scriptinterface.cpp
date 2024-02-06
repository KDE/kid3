/**
 * \file scriptinterface.cpp
 * D-Bus script adaptor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Dec 2007
 *
 * Copyright (C) 2007-2024  Urs Fleisch
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
#include "fileconfig.h"

/**
 * Constructor.
 *
 * @param app parent application
 */
ScriptInterface::ScriptInterface(Kid3Application* app)
  : QDBusAbstractAdaptor(app), m_app(app)
{
  setObjectName(QLatin1String("ScriptInterface"));
  setAutoRelaySignals(true);
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
  return m_app->openDirectory({path}, true);
}

/**
 * Unload all tags.
 * The tags of all files which are not modified or selected are freed to
 * reclaim their memory.
 */
void ScriptInterface::unloadAllTags()
{
  m_app->unloadAllTags();
}

/**
 * Save all modified files.
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::save()
{
  if (QStringList errorFiles = m_app->saveDirectory(); errorFiles.isEmpty()) {
    m_errorMsg.clear();
    return true;
  } else {
    m_errorMsg = QLatin1String("Error while writing file:\n") +
        errorFiles.join(QLatin1String("\n"));
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
  return m_app->importTags(Frame::tagVersionCast(tagMask), path, fmtIdx);
}

/**
 * Import from tags.
 *
 * @param tagMask tag mask
 * @param source format to get source text from tags
 * @param extraction regular expression with frame names and captures to
 * extract from source text
 */
void ScriptInterface::importFromTags(int tagMask,
                                     const QString& source,
                                     const QString& extraction)
{
  m_app->importFromTags(Frame::tagVersionCast(tagMask), source, extraction);
}

/**
 * Import from tags on selected files.
 *
 * @param tagMask tag mask
 * @param source format to get source text from tags
 * @param extraction regular expression with frame names and captures to
 * extract from source text
 *
 * @return extracted values for "%{__return}(.+)", empty if not used.
 */
QStringList ScriptInterface::importFromTagsToSelection(int tagMask,
                                                      const QString& source,
                                                      const QString& extraction)
{
  return m_app->importFromTagsToSelection(Frame::tagVersionCast(tagMask),
                                          source, extraction);
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
  return m_app->batchImport(profileName, Frame::tagVersionCast(tagMask));
}

/**
 * Download album cover art into the picture frame of the selected files.
 *
 * @param url           URL of picture file or album art resource
 * @param allFilesInDir true to add the image to all files in the directory
 */
void ScriptInterface::downloadAlbumArt(const QString& url, bool allFilesInDir)
{
  m_app->downloadImage(url, allFilesInDir);
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
  return m_app->exportTags(Frame::tagVersionCast(tagMask), path, fmtIdx);
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
 * Get items of a playlist.
 * @param path path to playlist file
 * @return list of absolute paths to playlist items.
 */
QStringList ScriptInterface::getPlaylistItems(const QString& path)
{
  return m_app->getPlaylistItems(path);
}

/**
 * Set items of a playlist.
 * @param path path to playlist file
 * @param items list of absolute paths to playlist items
 * @return true if ok, false if not all @a items were found and added or
 *         saving failed.
 */
bool ScriptInterface::setPlaylistItems(const QString& path,
                                       const QStringList& items)
{
  return m_app->setPlaylistItems(path, items);
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
  if (QModelIndex index(m_app->getFileSelectionModel()->currentIndex());
      !FileProxyModel::getPathIfIndexOfDir(index).isNull()) {
    m_app->expandDirectory(index);
    return true;
  }
  return false;
}

/**
 * Expand the file list.
 */
void ScriptInterface::expandFileList()
{
  m_app->requestExpandFileList();
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
  m_app->applyTagFormat();
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
  connect(m_app, &Kid3Application::renameActionsScheduled,
          this, &ScriptInterface::onRenameActionsScheduled);
  if (m_app->renameDirectory(Frame::tagVersionCast(tagMask), format,
                             create)) {
    return true;
  }
  disconnect(m_app, &Kid3Application::renameActionsScheduled,
             this, &ScriptInterface::onRenameActionsScheduled);
  return false;
}

void ScriptInterface::onRenameActionsScheduled()
{
  disconnect(m_app, &Kid3Application::renameActionsScheduled,
             this, &ScriptInterface::onRenameActionsScheduled);
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
  m_app->numberTracks(firstTrackNr, 0, Frame::tagVersionCast(tagMask));
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
  FileConfig::instance().setToFilenameFormat(format);
}

/**
 * Set the file names of the selected files from the tags.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @see setFileNameFormat()
 */
void ScriptInterface::setFileNameFromTag(int tagMask)
{
  m_app->getFilenameFromTags(Frame::tagVersionCast(tagMask));
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
  return m_app->getFrame(Frame::tagVersionCast(tagMask), name);
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
  return m_app->setFrame(Frame::tagVersionCast(tagMask), name, value);
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
  Frame::TagNumber tagNr =
      Frame::tagNumberFromMask(Frame::tagVersionCast(tagMask));
  if (tagNr >= Frame::Tag_NumValues)
    return QStringList();

  QStringList lst;
  FrameTableModel* ft = m_app->frameModel(tagNr);
  for (auto it = ft->frames().cbegin(); it != ft->frames().cend(); ++it) {
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
      if (info.bitrate > 0 && info.bitrate < 16384) {
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
    FOR_ALL_TAGS(tagNr) {
      if (QString tag = taggedFile->getTagFormat(tagNr); !tag.isEmpty()) {
        lst << QLatin1String("Tag ") + Frame::tagNumberToString(tagNr) << tag; // clazy:exclude=reserve-candidates
      }
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
  m_app->getTagsFromFilename(Frame::tagVersionCast(tagMask));
}

/**
 * Set tag from other tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromOtherTag(int tagMask)
{
  m_app->copyToOtherTag(Frame::tagVersionCast(tagMask));
}

/**
 * Copy tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::copyTag(int tagMask)
{
  m_app->copyTags(Frame::tagVersionCast(tagMask));
}

/**
 * Paste tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::pasteTag(int tagMask)
{
  m_app->pasteTags(Frame::tagVersionCast(tagMask));
}

/**
 * Remove tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::removeTag(int tagMask)
{
  m_app->removeTags(Frame::tagVersionCast(tagMask));
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

/**
 * Play selected audio files.
 */
void ScriptInterface::playAudio()
{
  m_app->playAudio();
}

#endif // HAVE_QTDBUS
