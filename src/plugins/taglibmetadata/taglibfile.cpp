/**
 * \file taglibfile.cpp
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 *
 * Copyright (C) 2006-2018  Urs Fleisch
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

#include "taglibfile.h"
#include <QDir>
#include <QString>
#include <QTextCodec>
#include <QByteArray>
#include <QImage>
#include <QVarLengthArray>
#include "genres.h"
#include "attributedata.h"
#include "pictureframe.h"

// Just using include <oggfile.h>, include <flacfile.h> as recommended in the
// TagLib documentation does not work, as there are files with these names
// in this directory.
#include <mpegfile.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <flacfile.h>
#include <mpcfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <id3v2header.h>
#include <apetag.h>
#include <textidentificationframe.h>
#include <commentsframe.h>
#include <attachedpictureframe.h>
#include <uniquefileidentifierframe.h>

#if TAGLIB_VERSION <= 0x010400
#include "taglibext/generalencapsulatedobjectframe.h"
#include "taglibext/urllinkframe.h"
#include "taglibext/unsynchronizedlyricsframe.h"
#include "taglibext/speex/speexfile.h"
#include "taglibext/speex/taglib_speexfiletyperesolver.h"
#include "taglibext/trueaudio/ttafile.h"
#include "taglibext/trueaudio/taglib_trueaudiofiletyperesolver.h"
#include "taglibext/wavpack/wvfile.h"
#include "taglibext/wavpack/taglib_wavpackfiletyperesolver.h"
#else
#include <generalencapsulatedobjectframe.h>
#include <urllinkframe.h>
#include <unsynchronizedlyricsframe.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <wavpackfile.h>
#endif

#if TAGLIB_VERSION >= 0x010500
#include <oggflacfile.h>
#include <relativevolumeframe.h>
#endif

#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
#include <mp4file.h>
#endif
#ifdef TAGLIB_WITH_ASF
#include <asffile.h>
#endif
#include <aifffile.h>
#include <wavfile.h>
#include <popularimeterframe.h>
#include <privateframe.h>
#endif

#if TAGLIB_VERSION >= 0x010700
#include <apefile.h>
#endif

#if TAGLIB_VERSION >= 0x010800
#include <ownershipframe.h>
#include <modfile.h>
#include <s3mfile.h>
#include <itfile.h>
#include <tfilestream.h>
#ifdef HAVE_TAGLIB_XM_SUPPORT
#include <xmfile.h>
#endif
#endif

#if TAGLIB_VERSION >= 0x010900
#include <opusfile.h>
#endif

#if TAGLIB_VERSION >= 0x010901
#include "taglibext/dsf/dsffiletyperesolver.h"
#include "taglibext/dsf/dsffile.h"
#endif

#if TAGLIB_VERSION >= 0x010a00
#include <synchronizedlyricsframe.h>
#include <eventtimingcodesframe.h>
#include <chapterframe.h>
#include <tableofcontentsframe.h>
#else
#include "taglibext/synchronizedlyricsframe.h"
#include "taglibext/eventtimingcodesframe.h"
#endif

#if TAGLIB_VERSION >= 0x010b00
#include <podcastframe.h>
#endif
#include "taglibext/aac/aacfiletyperesolver.h"
#include "taglibext/mp2/mp2filetyperesolver.h"

/** for loop through all supported tag number values. */
#define FOR_TAGLIB_TAGS(variable) \
  for (Frame::TagNumber variable = Frame::Tag_1; \
       variable < NUM_TAGS; \
       variable = static_cast<Frame::TagNumber>(variable + 1))

namespace {

/** Convert QString @a s to a TagLib::String. */
TagLib::String toTString(const QString& s)
{
  int len = s.length();
  QVarLengthArray<wchar_t> a(len + 1);
  wchar_t* ws = a.data();
  len = s.toWCharArray(ws);
  ws[len] = 0;
  return TagLib::String(ws);
}

/** Convert TagLib::String @a s to a QString. */
inline QString toQString(const TagLib::String& s)
{
#if TAGLIB_VERSION >= 0x010900
  return QString::fromWCharArray(s.toCWString(), s.size());
#else
  return TStringToQString(s);
#endif
}

#if TAGLIB_VERSION >= 0x010700
/**
 * Set a picture frame from a FLAC picture.
 *
 * @param pic FLAC picture
 * @param frame the picture frame is returned here
 */
void flacPictureToFrame(const TagLib::FLAC::Picture* pic, Frame& frame)
{
  TagLib::ByteVector picData(pic->data());
  QByteArray ba(picData.data(), picData.size());
  PictureFrame::ImageProperties imgProps(
        pic->width(), pic->height(), pic->colorDepth(),
        pic->numColors(), ba);
  PictureFrame::setFields(
    frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), toQString(pic->mimeType()),
    static_cast<PictureFrame::PictureType>(pic->type()),
    toQString(pic->description()),
    ba, &imgProps);
}

/**
 * Set a FLAC picture from a frame.
 *
 * @param frame picture frame
 * @param pic the FLAC picture to set
 */
void frameToFlacPicture(const Frame& frame, TagLib::FLAC::Picture* pic)
{
  Frame::TextEncoding enc;
  QString imgFormat;
  QString mimeType;
  PictureFrame::PictureType pictureType;
  QString description;
  QByteArray data;
  PictureFrame::ImageProperties imgProps;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, data, &imgProps);
  pic->setType(static_cast<TagLib::FLAC::Picture::Type>(pictureType));
  pic->setMimeType(toTString(mimeType));
  pic->setDescription(toTString(description));
  pic->setData(TagLib::ByteVector(data.data(), data.size()));
  if (!imgProps.isValidForImage(data)) {
    imgProps = PictureFrame::ImageProperties(data);
  }
  pic->setWidth(imgProps.width());
  pic->setHeight(imgProps.height());
  pic->setColorDepth(imgProps.depth());
  pic->setNumColors(imgProps.numColors());
}
#endif

}


#if TAGLIB_VERSION >= 0x010900
/**
 * TagLib::RIFF::WAV::File subclass with additional method for id3 chunk name.
 */
class WavFile : public TagLib::RIFF::WAV::File {
public:
  /**
   * Constructor.
   * @param stream stream to open
   */
  explicit WavFile(TagLib::IOStream *stream);

  /**
   * Destructor.
   */
  virtual ~WavFile();

  /**
   * Replace the "ID3 " chunk with a lowercase named "id3 " chunk.
   * This method has to be called after successully calling save() to use
   * lowercase "id3 " chunk names.
   */
  void changeToLowercaseId3Chunk();
};

WavFile::WavFile(TagLib::IOStream *stream) : TagLib::RIFF::WAV::File(stream)
{
}

WavFile::~WavFile()
{
}

void WavFile::changeToLowercaseId3Chunk()
{
  if (readOnly() || !isValid())
    return;

  int i;
  for (i = chunkCount() - 1; i >= 0; --i) {
    if (chunkName(i) == "ID3 ") {
      break;
    }
  }
  if (i >= 0) {
    TagLib::ByteVector data = chunkData(i);
    removeChunk(i);
    setChunkData("id3 ", data);
  }
}
#endif


#if TAGLIB_VERSION >= 0x010800
/**
 * Wrapper around TagLib::FileStream which reduces the number of open file
 * descriptors.
 *
 * Using streams, closing the file descriptor is also possible for modified
 * files because the TagLib file does not have to be deleted just to close the
 * file descriptor.
 */
class FileIOStream : public TagLib::IOStream {
public:
  /**
   * Constructor.
   * @param fileName path to file
   */
  explicit FileIOStream(const QString& fileName);

  /**
   * Destructor.
   */
  virtual ~FileIOStream();

  /**
   * Close the file handle.
   * The file will automatically be opened again if needed.
   */
  void closeFileHandle();

  // Reimplemented from TagLib::IOStream, delegate to TagLib::FileStream.
  TagLib::FileName name() const;
  TagLib::ByteVector readBlock(ulong length);
  void writeBlock(const TagLib::ByteVector &data);
  void insert(const TagLib::ByteVector &data,
              ulong start = 0, ulong replace = 0);
  void removeBlock(ulong start = 0, ulong length = 0);
  bool readOnly() const;
  bool isOpen() const;
  void seek(long offset, Position p = Beginning);
  void clear();
  long tell() const;
  long length();
  void truncate(long length);

  /**
   * Create a TagLib file for a stream.
   * TagLib::FileRef::create() adapted for IOStream.
   * @param stream stream with name() of which the extension is used to decduce
   * the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* create(IOStream* stream);

private:
  /**
   * Open file handle, is called by operations which need a file handle.
   *
   * @return true if file is open.
   */
  bool openFileHandle() const;

  /**
   * Register open files, so that the number of open files can be limited.
   * If the number of open files exceeds a limit, files are closed.
   *
   * @param stream new open file to be registered
   */
  static void registerOpenFile(FileIOStream* stream);

  /**
   * Deregister open file.
   *
   * @param stream file which is no longer open
   */
  static void deregisterOpenFile(FileIOStream* stream);

#ifdef Q_OS_WIN32
  wchar_t* m_fileName;
#else
  char* m_fileName;
#endif
  TagLib::FileStream* m_fileStream;
  long m_offset;

  /** list of file streams with open file descriptor */
  static QList<FileIOStream*> s_openFiles;
};

QList<FileIOStream*> FileIOStream::s_openFiles;

FileIOStream::FileIOStream(const QString& fileName) :
  m_fileStream(0), m_offset(0)
{
#ifdef Q_OS_WIN32
  int fnLen = fileName.length();
  m_fileName = new wchar_t[fnLen + 1];
  m_fileName[fnLen] = 0;
  fileName.toWCharArray(m_fileName);
#else
  QByteArray fn = QFile::encodeName(fileName);
  m_fileName = new char[fn.size() + 1];
  qstrcpy(m_fileName, fn.data());
#endif
}

FileIOStream::~FileIOStream()
{
  deregisterOpenFile(this);
  delete m_fileStream;
  delete [] m_fileName;
}

bool FileIOStream::openFileHandle() const
{
  if (!m_fileStream) {
    FileIOStream* self = const_cast<FileIOStream*>(this);
    self->m_fileStream =
        new TagLib::FileStream(TagLib::FileName(m_fileName));
    if (!self->m_fileStream->isOpen()) {
      delete self->m_fileStream;
      self->m_fileStream = 0;
      return false;
    }
    if (m_offset > 0) {
      m_fileStream->seek(m_offset);
    }
    registerOpenFile(self);
  }
  return true;
}

void FileIOStream::closeFileHandle()
{
  if (m_fileStream) {
    m_offset = m_fileStream->tell();
    delete m_fileStream;
    m_fileStream = 0;
    deregisterOpenFile(this);
  }
}

TagLib::FileName FileIOStream::name() const
{
  if (m_fileStream) {
    return m_fileStream->name();
  }
  return TagLib::FileName(m_fileName);
}

TagLib::ByteVector FileIOStream::readBlock(ulong length)
{
  if (openFileHandle()) {
    return m_fileStream->readBlock(length);
  }
  return TagLib::ByteVector();
}

void FileIOStream::writeBlock(const TagLib::ByteVector &data)
{
  if (openFileHandle()) {
    m_fileStream->writeBlock(data);
  }
}

void FileIOStream::insert(const TagLib::ByteVector &data,
                          ulong start, ulong replace)
{
  if (openFileHandle()) {
    m_fileStream->insert(data, start, replace);
  }
}

void FileIOStream::removeBlock(ulong start, ulong length)
{
  if (openFileHandle()) {
    m_fileStream->removeBlock(start, length);
  }
}

bool FileIOStream::readOnly() const
{
  if (openFileHandle()) {
    return m_fileStream->readOnly();
  }
  return true;
}

bool FileIOStream::isOpen() const
{
  if (m_fileStream) {
    return m_fileStream->isOpen();
  }
  return true;
}

void FileIOStream::seek(long offset, Position p)
{
  if (openFileHandle()) {
    m_fileStream->seek(offset, p);
  }
}

void FileIOStream::clear()
{
  if (openFileHandle()) {
    m_fileStream->clear();
  }
}

long FileIOStream::tell() const
{
  if (openFileHandle()) {
    return m_fileStream->tell();
  }
  return 0;
}

long FileIOStream::length()
{
  if (openFileHandle()) {
    return m_fileStream->length();
  }
  return 0;
}

void FileIOStream::truncate(long length)
{
  if (openFileHandle()) {
    m_fileStream->truncate(length);
  }
}

TagLib::File* FileIOStream::create(TagLib::IOStream* stream)
{
#ifdef Q_OS_WIN32
  TagLib::String fn = stream->name().toString();
#else
  TagLib::String fn = stream->name();
#endif
  const int extPos = fn.rfind(".");
  if (extPos != -1) {
    TagLib::String ext = fn.substr(extPos + 1).upper();
    if (ext == "MP3" || ext == "MP2" || ext == "AAC")
      return new TagLib::MPEG::File(stream,
                                    TagLib::ID3v2::FrameFactory::instance());
    if (ext == "OGG") {
      TagLib::File* file = new TagLib::Vorbis::File(stream);
      if (!file->isValid()) {
        delete file;
        file = new TagLib::Ogg::FLAC::File(stream);
      }
      return file;
    }
    if (ext == "OGA") {
      TagLib::File* file = new TagLib::Ogg::FLAC::File(stream);
      if (!file->isValid()) {
        delete file;
        file = new TagLib::Vorbis::File(stream);
      }
      return file;
    }
    if (ext == "FLAC")
      return new TagLib::FLAC::File(stream,
                                    TagLib::ID3v2::FrameFactory::instance());
    if (ext == "MPC")
      return new TagLib::MPC::File(stream);
    if (ext == "WV")
      return new TagLib::WavPack::File(stream);
    if (ext == "SPX")
      return new TagLib::Ogg::Speex::File(stream);
#if TAGLIB_VERSION >= 0x010900
    if (ext == "OPUS")
      return new TagLib::Ogg::Opus::File(stream);
#endif
    if (ext == "TTA")
      return new TagLib::TrueAudio::File(stream);
    if (ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" ||
        ext == "MP4" || ext == "3G2")
      return new TagLib::MP4::File(stream);
    if (ext == "WMA" || ext == "ASF")
      return new TagLib::ASF::File(stream);
    if (ext == "AIF" || ext == "AIFF")
      return new TagLib::RIFF::AIFF::File(stream);
    if (ext == "WAV")
#if TAGLIB_VERSION >= 0x010900
      return new WavFile(stream);
#else
      return new TagLib::RIFF::WAV::File(stream);
#endif
    if (ext == "APE")
      return new TagLib::APE::File(stream);
    if (ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
      return new TagLib::Mod::File(stream);
    if (ext == "S3M")
      return new TagLib::S3M::File(stream);
    if (ext == "IT")
      return new TagLib::IT::File(stream);
#ifdef HAVE_TAGLIB_XM_SUPPORT
    if (ext == "XM")
      return new TagLib::XM::File(stream);
#endif
#if TAGLIB_VERSION >= 0x010901
    if (ext == "DSF")
      return new DSFFile(stream, TagLib::ID3v2::FrameFactory::instance());
#endif
  }
  return 0;
}

void FileIOStream::registerOpenFile(FileIOStream* stream)
{
  if (s_openFiles.contains(stream))
    return;

  int numberOfFilesToClose = s_openFiles.size() - 15;
  if (numberOfFilesToClose > 5) {
    for (QList<FileIOStream*>::iterator it = s_openFiles.begin();
         it != s_openFiles.end();
         ++it) {
      (*it)->closeFileHandle();
      if (--numberOfFilesToClose <= 0) {
        break;
      }
    }
  }
  s_openFiles.append(stream);
}

/**
 * Deregister open file.
 *
 * @param stream file which is no longer open
 */
void FileIOStream::deregisterOpenFile(FileIOStream* stream)
{
  s_openFiles.removeAll(stream);
}
#endif

/**
 * Data encoding in ID3v1 tags.
 */
class TextCodecStringHandler : public TagLib::ID3v1::StringHandler {
public:
  /**
   * Constructor.
   */
  TextCodecStringHandler() {}

  /**
   * Destructor.
   */
  virtual ~TextCodecStringHandler() {}

  /**
   * Decode a string from data.
   *
   * @param data data to decode
   */
  virtual TagLib::String parse(const TagLib::ByteVector& data) const;

  /**
   * Encode a byte vector with the data from a string.
   *
   * @param s string to encode
   */
  virtual TagLib::ByteVector render(const TagLib::String& s) const;

  /**
   * Set text codec.
   * @param codec text codec, 0 for default behavior (ISO 8859-1)
   */
  static void setTextCodec(const QTextCodec* codec) { s_codec = codec; }

private:
  static const QTextCodec* s_codec;
};

const QTextCodec* TextCodecStringHandler::s_codec = 0;

/**
 * Decode a string from data.
 *
 * @param data data to decode
 */
TagLib::String TextCodecStringHandler::parse(const TagLib::ByteVector& data) const
{
  return s_codec ?
    toTString(s_codec->toUnicode(data.data(), data.size())).stripWhiteSpace() :
    TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
}

/**
 * Encode a byte vector with the data from a string.
 *
 * @param s string to encode
 */
TagLib::ByteVector TextCodecStringHandler::render(const TagLib::String& s) const
{
  if (s_codec) {
    QByteArray ba(s_codec->fromUnicode(toQString(s)));
    return TagLib::ByteVector(ba.data(), ba.size());
  } else {
    return s.data(TagLib::String::Latin1);
  }
}


/** Default text encoding */
TagLib::String::Type TagLibFile::s_defaultTextEncoding = TagLib::String::Latin1;

#if TAGLIB_VERSION < 0x010800
/** List of TagLib files with open file descriptor */
QList<TagLibFile*> TagLibFile::s_openFiles;
#endif


/**
 * Constructor.
 *
 * @param idx index in file proxy model
 */
TagLibFile::TagLibFile(const QPersistentModelIndex& idx) :
  TaggedFile(idx),
  m_tagInformationRead(false), m_fileRead(false),
#if TAGLIB_VERSION >= 0x010800
  m_stream(0),
  m_id3v2Version(0),
#endif
  m_activatedFeatures(0), m_duration(0)
{
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_isTagSupported[tagNr] = tagNr == Frame::Tag_2;
    m_tag[tagNr] = 0;
    m_tagType[tagNr] = TT_Unknown;
  }
}

/**
 * Destructor.
 */
TagLibFile::~TagLibFile()
{
  closeFile(true);
}

/**
 * Get key of tagged file format.
 * @return "TaglibMetadata".
 */
QString TagLibFile::taggedFileKey() const
{
  return QLatin1String("TaglibMetadata");
}

/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int TagLibFile::taggedFileFeatures() const
{
  return TF_ID3v11 | TF_ID3v22 |
#if TAGLIB_VERSION >= 0x010500
      TF_OggFlac |
#endif
#if TAGLIB_VERSION >= 0x010700
      TF_OggPictures |
#endif
#if TAGLIB_VERSION >= 0x010800
      TF_ID3v23 |
#endif
      TF_ID3v24;
}

/**
 * Get currently active tagged file features.
 * @return active tagged file features (TF_ID3v23, TF_ID3v24, or 0).
 * @see setActiveTaggedFileFeatures()
 */
int TagLibFile::activeTaggedFileFeatures() const
{
  return m_activatedFeatures;
}

/**
 * Activate some features provided by the tagged file.
 * TagLibFile provides the TF_ID3v23 and TF_ID3v24 features, which determine
 * the ID3v2 version used in writeTags() (the overload without id3v2Version).
 * If 0 is set, the default behavior applies, i.e. for new files,
 * TagConfig::id3v2Version() is used, else the existing version.
 *
 * @param features TF_ID3v23, TF_ID3v24, or 0
 */
void TagLibFile::setActiveTaggedFileFeatures(int features)
{
  m_activatedFeatures = features;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void TagLibFile::clearTags(bool force)
{
  if (isChanged() && !force)
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  closeFile(true);
#if TAGLIB_VERSION >= 0x010700
  m_pictures.clear();
  m_pictures.setRead(false);
#endif
  m_tagInformationRead = false;
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_tagFormat[tagNr].clear();
    m_tagType[tagNr] = TT_Unknown;
    markTagUnchanged(tagNr);
  }
  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void TagLibFile::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  QString fileName = currentFilePath();
  QByteArray fn = QFile::encodeName(fileName);

  if (force || m_fileRef.isNull()) {
#if TAGLIB_VERSION >= 0x010800
    delete m_stream;
    m_stream = new FileIOStream(fileName);
    m_fileRef = TagLib::FileRef(FileIOStream::create(m_stream));
#else
#if TAGLIB_VERSION > 0x010400 && defined Q_OS_WIN32
    int fnLen = fileName.length();
    wchar_t* fnWs = new wchar_t[fnLen + 1];
    fnWs[fnLen] = 0;
    fileName.toWCharArray(fnWs);
    m_fileRef = TagLib::FileRef(TagLib::FileName(fnWs));
    delete [] fnWs;
#else
    m_fileRef = TagLib::FileRef(fn);
#endif
    registerOpenFile(this);
#endif
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = 0;
      markTagUnchanged(tagNr);
    }
    m_fileRead = true;

#if TAGLIB_VERSION >= 0x010700
    m_pictures.clear();
    m_pictures.setRead(false);
#endif
  }

  TagLib::File* file;
  if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
    TagLib::MPEG::File* mpegFile;
    TagLib::FLAC::File* flacFile;
#if TAGLIB_VERSION >= 0x010b00
    TagLib::MPC::File* mpcFile;
    TagLib::WavPack::File* wvFile;
#endif
    TagLib::TrueAudio::File* ttaFile;
#if TAGLIB_VERSION >= 0x010600
    TagLib::RIFF::WAV::File* wavFile;
#endif
#if TAGLIB_VERSION >= 0x010700
    TagLib::APE::File* apeFile;
#endif
    m_fileExtension = QLatin1String(".mp3");
    m_isTagSupported[Frame::Tag_1] = false;
    if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
      QString ext(fileName.right(4).toLower());
      m_fileExtension =
          (ext == QLatin1String(".aac") || ext == QLatin1String(".mp2"))
          ? ext : QLatin1String(".mp3");
      m_isTagSupported[Frame::Tag_1] = true;
      m_isTagSupported[Frame::Tag_3] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = mpegFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        TagLib::ID3v2::Tag* id3v2Tag = mpegFile->ID3v2Tag();
#if TAGLIB_VERSION >= 0x010800
        setId3v2VersionFromTag(id3v2Tag);
#endif
        m_tag[Frame::Tag_2] = id3v2Tag;
        markTagUnchanged(Frame::Tag_2);
      }
      if (!m_tag[Frame::Tag_3]) {
        m_tag[Frame::Tag_3] = mpegFile->APETag();
        markTagUnchanged(Frame::Tag_3);
      }
    } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".flac");
      m_isTagSupported[Frame::Tag_1] = true;
      m_isTagSupported[Frame::Tag_3] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = flacFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = flacFile->xiphComment();
        markTagUnchanged(Frame::Tag_2);
      }
      if (!m_tag[Frame::Tag_3]) {
        m_tag[Frame::Tag_3] = flacFile->ID3v2Tag();
        markTagUnchanged(Frame::Tag_3);
      }
#if TAGLIB_VERSION >= 0x010700
      if (!m_pictures.isRead()) {
        TagLib::List<TagLib::FLAC::Picture*> pics(flacFile->pictureList());
        int i = 0;
        for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it =
             pics.begin(); it != pics.end(); ++it) {
          PictureFrame frame;
          flacPictureToFrame(*it, frame);
          frame.setIndex(i++);
          m_pictures.append(frame);
        }
        m_pictures.setRead(true);
      }
#endif
#if TAGLIB_VERSION >= 0x010b00
    } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".mpc");
      m_isTagSupported[Frame::Tag_1] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = mpcFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = mpcFile->APETag();
        markTagUnchanged(Frame::Tag_2);
      }
    } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".wv");
      m_isTagSupported[Frame::Tag_1] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = wvFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = wvFile->APETag();
        markTagUnchanged(Frame::Tag_2);
      }
#endif
    } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".tta");
      m_isTagSupported[Frame::Tag_1] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = ttaFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = ttaFile->ID3v2Tag();
        markTagUnchanged(Frame::Tag_2);
      }
#if TAGLIB_VERSION >= 0x010700
    } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".ape");
      m_isTagSupported[Frame::Tag_1] = true;
      if (!m_tag[Frame::Tag_1]) {
        m_tag[Frame::Tag_1] = apeFile->ID3v1Tag();
        markTagUnchanged(Frame::Tag_1);
      }
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = apeFile->APETag();
        markTagUnchanged(Frame::Tag_2);
      }
#endif
#if TAGLIB_VERSION >= 0x010600
    } else if ((wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) != 0) {
      m_fileExtension = QLatin1String(".wav");
      m_tag[Frame::Tag_1] = 0;
      markTagUnchanged(Frame::Tag_1);
#if TAGLIB_VERSION >= 0x010a00
      m_isTagSupported[Frame::Tag_3] = true;
      if (!m_tag[Frame::Tag_2]) {
        TagLib::ID3v2::Tag* id3v2Tag = wavFile->ID3v2Tag();
#if TAGLIB_VERSION >= 0x010900
        setId3v2VersionFromTag(id3v2Tag);
#endif
        m_tag[Frame::Tag_2] = id3v2Tag;
        markTagUnchanged(Frame::Tag_2);
      }
      if (!m_tag[Frame::Tag_3]) {
        m_tag[Frame::Tag_3] = wavFile->InfoTag();
        markTagUnchanged(Frame::Tag_3);
      }
#else
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = wavFile->tag();
        markTagUnchanged(Frame::Tag_2);
      }
#endif
#endif
    } else {
      if (dynamic_cast<TagLib::Vorbis::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".ogg");
      } else if (dynamic_cast<TagLib::Ogg::Speex::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".spx");
#if TAGLIB_VERSION < 0x010b00
      } else if (dynamic_cast<TagLib::MPC::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".mpc");
      } else if (dynamic_cast<TagLib::WavPack::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".wv");
#endif
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      } else if (dynamic_cast<TagLib::MP4::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".m4a");
#endif
#ifdef TAGLIB_WITH_ASF
      } else if (dynamic_cast<TagLib::ASF::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".wma");
#endif
      } else if (dynamic_cast<TagLib::RIFF::AIFF::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".aiff");
#endif
#if TAGLIB_VERSION >= 0x010800
      } else if (dynamic_cast<TagLib::Mod::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".mod");
      } else if (dynamic_cast<TagLib::S3M::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".s3m");
      } else if (dynamic_cast<TagLib::IT::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".it");
#ifdef HAVE_TAGLIB_XM_SUPPORT
      } else if (dynamic_cast<TagLib::XM::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".xm");
#endif
#endif
#if TAGLIB_VERSION >= 0x010900
      } else if (dynamic_cast<TagLib::Ogg::Opus::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".opus");
#endif
#if TAGLIB_VERSION >= 0x010901
      } else if (dynamic_cast<DSFFile*>(file) != 0) {
        m_fileExtension = QLatin1String(".dsf");
#endif
      }
      m_tag[Frame::Tag_1] = 0;
      markTagUnchanged(Frame::Tag_1);
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = m_fileRef.tag();
        markTagUnchanged(Frame::Tag_2);
      }
#if TAGLIB_VERSION >= 0x010b00
      if (!m_pictures.isRead()) {
        if (TagLib::Ogg::XiphComment* xiphComment =
            dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[Frame::Tag_2])) {
          TagLib::List<TagLib::FLAC::Picture*> pics(xiphComment->pictureList());
          int i = 0;
          for (TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it =
               pics.begin(); it != pics.end(); ++it) {
            PictureFrame frame;
            flacPictureToFrame(*it, frame);
            frame.setIndex(i++);
            m_pictures.append(frame);
          }
          m_pictures.setRead(true);
        }
      }
#endif
    }
  }

  // Cache information, so that it is available after file is closed.
  m_tagInformationRead = true;
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = m_tag[tagNr] && !m_tag[tagNr]->isEmpty();
    m_tagFormat[tagNr] = getTagFormat(m_tag[tagNr], m_tagType[tagNr]);
  }
  readAudioProperties();

  if (force) {
    setFilename(currentFilename());
  }

  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Close file handle.
 * TagLib keeps the file handle open until the FileRef is destroyed.
 * This causes problems when the operating system has a limited number of
 * open file handles. This method closes the file by assigning a new file
 * reference. Note that this will also invalidate the tag pointers.
 * The file is only closed if there are no unsaved tag changes or if the
 * @a force parameter is set.
 *
 * @param force true to close the file even if tags are changed
 */
void TagLibFile::closeFile(bool force)
{
#if TAGLIB_VERSION >= 0x010800
  if (force) {
    m_fileRef = TagLib::FileRef();
    delete m_stream;
    m_stream = 0;
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = 0;
    }
    m_fileRead = false;
  } else if (m_stream) {
    m_stream->closeFileHandle();
  }
#else
  bool tagChanged = false;
  FOR_TAGLIB_TAGS(tagNr) {
    if (isTagChanged(tagNr)) {
      tagChanged = true;
      break;
    }
  }
  if (force || !tagChanged) {
    m_fileRef = TagLib::FileRef();
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = 0;
    }
    m_fileRead = false;
    deregisterOpenFile(this);
  }
#endif
}

/**
 * Make sure that file is open.
 * This method should be called before accessing m_fileRef, m_tag.
 *
 * @param force true to force reopening of file even if it is already open
 */
void TagLibFile::makeFileOpen(bool force) const
{
  if (!m_fileRead || force) {
    const_cast<TagLibFile*>(this)->readTags(force);
  }
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force    true to force writing even if file was not changed.
 * @param renamed  will be set to true if the file was renamed,
 *                 i.e. the file name is no longer valid, else *renamed
 *                 is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool TagLibFile::writeTags(bool force, bool* renamed, bool preserve)
{
  int id3v2Version;
  if (m_activatedFeatures & TF_ID3v24)
    id3v2Version = 4;
  else if (m_activatedFeatures & TF_ID3v23)
    id3v2Version = 3;
  else
    id3v2Version = 0;
  return writeTags(force, renamed, preserve, id3v2Version);
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force    true to force writing even if file was not changed.
 * @param renamed  will be set to true if the file was renamed,
 *                 i.e. the file name is no longer valid, else *renamed
 *                 is left unchanged
 * @param preserve true to preserve file time stamps
 * @param id3v2Version ID3v2 version to use, 0 to use existing or preferred,
 *                     3 to force ID3v2.3.0, 4 to force ID3v2.4.0. Is ignored
 *                     if TagLib version is less than 1.8.0.
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool TagLibFile::writeTags(bool force, bool* renamed, bool preserve,
                           int id3v2Version)
{
  QString fnStr(currentFilePath());
  if (isChanged() && !QFileInfo(fnStr).isWritable()) {
#if TAGLIB_VERSION >= 0x010800
    closeFile(false);
#endif
    revertChangedFilename();
    return false;
  }

  // store time stamp if it has to be preserved
  quint64 actime = 0, modtime = 0;
  if (preserve) {
    getFileTimeStamps(fnStr, actime, modtime);
  }

  bool fileChanged = false;
  TagLib::File* file;
  if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
    TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(file);
    if (mpegFile) {
      static const int tagTypes[NUM_TAGS] = {
        TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
        TagLib::MPEG::File::APE
      };
      int saveMask = 0;
      FOR_TAGLIB_TAGS(tagNr) {
        if (m_tag[tagNr] && (force || isTagChanged(tagNr))) {
          if (m_tag[tagNr]->isEmpty()) {
            mpegFile->strip(tagTypes[tagNr]);
            fileChanged = true;
            markTagUnchanged(tagNr);
            m_tag[tagNr] = 0;
          } else {
            saveMask |= tagTypes[tagNr];
          }
        }
      }
      if (saveMask != 0) {
#if TAGLIB_VERSION >= 0x010800
        setId3v2VersionOrDefault(id3v2Version);
#else
        Q_UNUSED(id3v2Version);
#endif
        if (mpegFile->save(saveMask, false
#if TAGLIB_VERSION >= 0x010800
                           , m_id3v2Version
#endif
#if TAGLIB_VERSION >= 0x010900
                           , false
#endif
                           )) {
          fileChanged = true;
          FOR_TAGLIB_TAGS(tagNr) {
            if (saveMask & tagTypes[tagNr]) {
              markTagUnchanged(tagNr);
            }
          }
        }
      }
    } else {
      bool needsSave = false;
      FOR_TAGLIB_TAGS(tagNr) {
        if (m_tag[tagNr] && (force || isTagChanged(tagNr))) {
          needsSave = true;
          break;
        }
      }
      if (needsSave) {
        if (TagLib::TrueAudio::File* ttaFile =
            dynamic_cast<TagLib::TrueAudio::File*>(file)) {
          static const int tagTypes[NUM_TAGS] = {
            TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
            TagLib::MPEG::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) && m_tag[tagNr]->isEmpty()) {
              ttaFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              markTagUnchanged(tagNr);
              m_tag[tagNr] = 0;
            }
          }
        } else if (TagLib::MPC::File* mpcFile =
                   dynamic_cast<TagLib::MPC::File*>(file)) {
#if TAGLIB_VERSION >= 0x010b00
          static const int tagTypes[NUM_TAGS] = {
            TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2,
            TagLib::MPC::File::APE, TagLib::MPC::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) &&
                m_tag[tagNr]->isEmpty()) {
              mpcFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              markTagUnchanged(tagNr);
              m_tag[tagNr] = 0;
            }
          }
#else
          // it does not work if there is also an ID3 tag (bug in TagLib)
          mpcFile->remove(TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2);
          fileChanged = true;
#endif
        } else if (TagLib::WavPack::File* wvFile =
                   dynamic_cast<TagLib::WavPack::File*>(file)) {
#if TAGLIB_VERSION >= 0x010b00
          static const int tagTypes[NUM_TAGS] = {
            TagLib::WavPack::File::ID3v1, TagLib::WavPack::File::APE,
            TagLib::WavPack::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) &&
                m_tag[tagNr]->isEmpty()) {
              wvFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              markTagUnchanged(tagNr);
              m_tag[tagNr] = 0;
            }
          }
#else
          // it does not work if there is also an ID3 tag (bug in TagLib)
          wvFile->strip(TagLib::WavPack::File::ID3v1);
          fileChanged = true;
#endif
        }
#if TAGLIB_VERSION >= 0x010700
        else if (TagLib::APE::File* apeFile =
                 dynamic_cast<TagLib::APE::File*>(file)) {
          static const int tagTypes[NUM_TAGS] = {
            TagLib::MPEG::File::ID3v1, TagLib::APE::File::APE,
            TagLib::APE::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) && m_tag[tagNr]->isEmpty()) {
              apeFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              markTagUnchanged(tagNr);
              m_tag[tagNr] = 0;
            }
          }
        }
#endif
#if TAGLIB_VERSION >= 0x010700
        else if (TagLib::FLAC::File* flacFile =
            dynamic_cast<TagLib::FLAC::File*>(file)) {
#if TAGLIB_VERSION >= 0x010b00
          static const int tagTypes[NUM_TAGS] = {
            TagLib::FLAC::File::ID3v1, TagLib::FLAC::File::XiphComment,
            TagLib::FLAC::File::ID3v2
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) && m_tag[tagNr]->isEmpty()) {
              flacFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              markTagUnchanged(tagNr);
              m_tag[tagNr] = 0;
            }
          }
#endif
          flacFile->removePictures();
          foreach (const Frame& frame, m_pictures) {
            TagLib::FLAC::Picture* pic = new TagLib::FLAC::Picture;
            frameToFlacPicture(frame, pic);
            flacFile->addPicture(pic);
          }
        }
#endif
#if TAGLIB_VERSION >= 0x010900
        else if (WavFile* wavFile = dynamic_cast<WavFile*>(file)) {
          static const TagLib::RIFF::WAV::File::TagTypes tagTypes[NUM_TAGS] = {
            TagLib::RIFF::WAV::File::NoTags, TagLib::RIFF::WAV::File::ID3v2,
#if TAGLIB_VERSION >= 0x010a00
            TagLib::RIFF::WAV::File::Info
#else
            TagLib::RIFF::WAV::File::NoTags
#endif
          };
          int saveTags = 0;
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) &&
                m_tag[tagNr]->isEmpty()) {
              m_tag[tagNr] = 0;
            } else {
              saveTags |= tagTypes[tagNr];
            }
          }
          setId3v2VersionOrDefault(id3v2Version);
          if (wavFile->save(static_cast<TagLib::RIFF::WAV::File::TagTypes>(
                              saveTags), true, m_id3v2Version)) {
            if (TagConfig::instance().lowercaseId3RiffChunk()) {
              wavFile->changeToLowercaseId3Chunk();
            }
            fileChanged = true;
            FOR_TAGLIB_TAGS(tagNr) {
              markTagUnchanged(tagNr);
            }
            needsSave = false;
          }
        }
#endif
#if TAGLIB_VERSION >= 0x010b00
        else if (TagLib::Ogg::XiphComment* xiphComment =
                 dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[Frame::Tag_2])) {
          xiphComment->removeAllPictures();
          foreach (const Frame& frame, m_pictures) {
            TagLib::FLAC::Picture* pic = new TagLib::FLAC::Picture;
            frameToFlacPicture(frame, pic);
            xiphComment->addPicture(pic);
          }
        }
#endif
        if (needsSave && m_fileRef.save()) {
          fileChanged = true;
          FOR_TAGLIB_TAGS(tagNr) {
            markTagUnchanged(tagNr);
          }
        }
      }
    }
  }

  // If the file was changed, make sure it is written to disk.
  // This is done when the file is closed. Later the file is opened again.
  // If the file is not properly closed, doubled tags can be
  // written if the file is finally closed!
  // This can be reproduced with an untagged MP3 file, then add
  // an ID3v2 title, save, add an ID3v2 artist, save, reload
  // => double ID3v2 tags.
  // On Windows it is necessary to close the file before renaming it,
  // so it is done even if the file is not changed.
#ifndef Q_OS_WIN32
  if (fileChanged)
#endif
    closeFile(true);

  // restore time stamp
  if (actime || modtime) {
    setFileTimeStamps(fnStr, actime, modtime);
  }

  if (isFilenameChanged()) {
    if (!renameFile(currentFilename(), getFilename())) {
      return false;
    }
    markFilenameUnchanged();
    *renamed = true;
  }

#ifndef Q_OS_WIN32
  if (fileChanged)
#endif
    makeFileOpen(true);
  return true;
}

/**
 * Get a genre string from a string which can contain the genre itself,
 * or only the genre number or the genre number in parenthesis.
 *
 * @param str genre string
 *
 * @return genre.
 */
static QString getGenreString(const TagLib::String& str)
{
  if (!str.isNull()) {
    QString qs = toQString(str);
    int cpPos = 0, n = 0xff;
    bool ok = false;
    if (qs[0] == QLatin1Char('(') && (cpPos = qs.indexOf(QLatin1Char(')'), 2)) > 1) {
      n = qs.mid(1, cpPos - 1).toInt(&ok);
      if (!ok || n > 0xff) {
        n = 0xff;
      }
      return QString::fromLatin1(Genres::getName(n));
    } else if ((n = qs.toInt(&ok)) >= 0 && n <= 0xff && ok) {
      return QString::fromLatin1(Genres::getName(n));
    } else {
      return qs;
    }
  } else {
    return QLatin1String("");
  }
}

/**
 * Create tag if it does not already exist so that it can be set.
 *
 * @return true if tag can be set.
 */
bool TagLibFile::makeTagSettable(Frame::TagNumber tagNr)
{
  if (tagNr >= NUM_TAGS)
    return false;

  makeFileOpen();
  if (!m_tag[tagNr]) {
    TagLib::File* file;
    if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
      TagLib::MPEG::File* mpegFile;
      TagLib::FLAC::File* flacFile;
      TagLib::MPC::File* mpcFile;
      TagLib::WavPack::File* wvFile;
      TagLib::TrueAudio::File* ttaFile;
#if TAGLIB_VERSION >= 0x010700
      TagLib::APE::File* apeFile;
#endif
      if (tagNr == Frame::Tag_1) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
          m_tag[tagNr] = mpegFile->ID3v1Tag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
          m_tag[tagNr] = flacFile->ID3v1Tag(true);
#if TAGLIB_VERSION >= 0x010b00
        } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
          m_tag[tagNr] = mpcFile->ID3v1Tag(true);
        } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
          m_tag[tagNr] = wvFile->ID3v1Tag(true);
#endif
        } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
          m_tag[tagNr] = ttaFile->ID3v1Tag(true);
#if TAGLIB_VERSION >= 0x010700
        } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
          m_tag[tagNr] = apeFile->ID3v1Tag(true);
#endif
        }
      } else if (tagNr == Frame::Tag_2) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
          m_tag[tagNr] = mpegFile->ID3v2Tag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
          m_tag[tagNr] = flacFile->xiphComment(true);
        } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
          m_tag[tagNr] = mpcFile->APETag(true);
        } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
          m_tag[tagNr] = wvFile->APETag(true);
        } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
          m_tag[tagNr] = ttaFile->ID3v2Tag(true);
#if TAGLIB_VERSION >= 0x010700
        } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
          m_tag[tagNr] = apeFile->APETag(true);
#endif
#if TAGLIB_VERSION >= 0x010900
        } else if (TagLib::RIFF::WAV::File* wavFile =
                   dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
          m_tag[tagNr] = wavFile->ID3v2Tag();
#endif
        }
      } else if (tagNr == Frame::Tag_3) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
          m_tag[tagNr] = mpegFile->APETag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
          m_tag[tagNr] = flacFile->ID3v2Tag(true);
#if TAGLIB_VERSION >= 0x010a00
        } else if (TagLib::RIFF::WAV::File* wavFile =
                   dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
          m_tag[tagNr] = wavFile->InfoTag();
#endif
        }
      }
    }
  }
  return m_tag[tagNr] != 0;
}

/**
 * Check if string needs Unicode encoding.
 *
 * @return true if Unicode needed,
 *         false if Latin-1 sufficient.
 */
static bool needsUnicode(const QString& qstr)
{
  bool result = false;
  uint unicodeSize = qstr.length();
  const QChar* qcarray = qstr.unicode();
  for (uint i = 0; i < unicodeSize; ++i) {
    char ch = qcarray[i].toLatin1();
    if (ch == 0 || (ch & 0x80) != 0) {
      result = true;
      break;
    }
  }
  return result;
}

/**
 * Get the configured text encoding.
 *
 * @param unicode true if unicode is required
 *
 * @return text encoding.
 */
static TagLib::String::Type getTextEncodingConfig(bool unicode)
{
  TagLib::String::Type enc = TagLibFile::getDefaultTextEncoding();
  if (unicode && enc == TagLib::String::Latin1) {
    enc = TagLib::String::UTF8;
  }
  return enc;
}

/**
 * Remove the first COMM frame with an empty description.
 *
 * @param id3v2Tag ID3v2 tag
 */
static void removeCommentFrame(TagLib::ID3v2::Tag* id3v2Tag)
{
  const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList("COMM");
  for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
       it != frameList.end();
       ++it) {
    TagLib::ID3v2::CommentsFrame* id3Frame =
      dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
    if (id3Frame && id3Frame->description().isEmpty()) {
      id3v2Tag->removeFrame(id3Frame, true);
      break;
    }
  }
}

/**
 * Write a Unicode field if the tag is ID3v2 and Latin-1 is not sufficient.
 *
 * @param tag     tag
 * @param qstr    text as QString
 * @param tstr    text as TagLib::String
 * @param frameId ID3v2 frame ID
 *
 * @return true if an ID3v2 Unicode field was written.
 */
static bool setId3v2Unicode(TagLib::Tag* tag, const QString& qstr, const TagLib::String& tstr, const char* frameId)
{
  TagLib::ID3v2::Tag* id3v2Tag;
  if (tag && (id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) != 0) {
    // first check if this string needs to be stored as unicode
    TagLib::String::Type enc = getTextEncodingConfig(needsUnicode(qstr));
    TagLib::ByteVector id(frameId);
    if (enc != TagLib::String::Latin1 || id == "COMM" || id == "TDRC") {
      if (id == "COMM") {
        removeCommentFrame(id3v2Tag);
      } else {
        id3v2Tag->removeFrames(id);
      }
      if (!tstr.isEmpty()) {
        TagLib::ID3v2::Frame* frame;
        if (frameId[0] != 'C') {
          frame = new TagLib::ID3v2::TextIdentificationFrame(id, enc);
        } else {
          TagLib::ID3v2::CommentsFrame* commFrame =
              new TagLib::ID3v2::CommentsFrame(enc);
          frame = commFrame;
          commFrame->setLanguage("eng"); // for compatibility with iTunes
        }
        frame->setText(tstr);
#ifdef Q_OS_WIN32
        // freed in Windows DLL => must be allocated in the same DLL
        TagLib::ID3v2::Frame* dllAllocatedFrame =
          TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
        if (dllAllocatedFrame) {
          id3v2Tag->addFrame(dllAllocatedFrame);
        }
        delete frame;
#else
        id3v2Tag->addFrame(frame);
#endif
      }
      return true;
    }
  }
  return false;
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTag() does not return meaningful information.
 */
bool TagLibFile::isTagInformationRead() const
{
  return m_tagInformationRead;
}

/**
 * Check if tags are supported by the format of this file.
 *
 * @param tagNr tag number
 * @return true.
 */
bool TagLibFile::isTagSupported(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_isTagSupported[tagNr] : false;
}

/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if tag is available.
 * @see isTagInformationRead()
 */
bool TagLibFile::hasTag(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_hasTag[tagNr] : false;
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void TagLibFile::getDetailInfo(DetailInfo& info) const
{
  info = m_detailInfo;
}

/**
 * Cache technical detail information.
 */
void TagLibFile::readAudioProperties()
{
  TagLib::AudioProperties* audioProperties;
  if (!m_fileRef.isNull() &&
      (audioProperties = m_fileRef.audioProperties()) != 0) {
    TagLib::MPEG::Properties* mpegProperties;
    TagLib::Ogg::Speex::Properties* speexProperties;
    TagLib::TrueAudio::Properties* ttaProperties;
    TagLib::WavPack::Properties* wvProperties;
#if TAGLIB_VERSION >= 0x010700
    TagLib::APE::Properties* apeProperties;
#endif
#if TAGLIB_VERSION >= 0x010800
    TagLib::Mod::Properties* modProperties;
    TagLib::S3M::Properties* s3mProperties;
    TagLib::IT::Properties* itProperties;
#ifdef HAVE_TAGLIB_XM_SUPPORT
    TagLib::XM::Properties* xmProperties;
#endif
#endif
#if TAGLIB_VERSION >= 0x010900
    TagLib::Ogg::Opus::Properties* opusProperties;
#endif
#if TAGLIB_VERSION >= 0x010901
    DSFProperties* dsfProperties;
#endif
    m_detailInfo.valid = true;
    if ((mpegProperties =
         dynamic_cast<TagLib::MPEG::Properties*>(audioProperties)) != 0) {
      if (getFilename().right(4).toLower() == QLatin1String(".aac")) {
        m_detailInfo.format = QLatin1String("AAC");
        return;
      }
      switch (mpegProperties->version()) {
        case TagLib::MPEG::Header::Version1:
          m_detailInfo.format = QLatin1String("MPEG 1 ");
          break;
        case TagLib::MPEG::Header::Version2:
          m_detailInfo.format = QLatin1String("MPEG 2 ");
          break;
        case TagLib::MPEG::Header::Version2_5:
          m_detailInfo.format = QLatin1String("MPEG 2.5 ");
          break;
      }
      int layer = mpegProperties->layer();
      if (layer >= 1 && layer <= 3) {
        m_detailInfo.format += QLatin1String("Layer ");
        m_detailInfo.format += QString::number(layer);
      }
      switch (mpegProperties->channelMode()) {
        case TagLib::MPEG::Header::Stereo:
          m_detailInfo.channelMode = DetailInfo::CM_Stereo;
          m_detailInfo.channels = 2;
          break;
        case TagLib::MPEG::Header::JointStereo:
          m_detailInfo.channelMode = DetailInfo::CM_JointStereo;
          m_detailInfo.channels = 2;
          break;
        case TagLib::MPEG::Header::DualChannel:
          m_detailInfo.channels = 2;
          break;
        case TagLib::MPEG::Header::SingleChannel:
          m_detailInfo.channels = 1;
          break;
      }
    } else if (dynamic_cast<TagLib::Vorbis::Properties*>(audioProperties) !=
               0) {
      m_detailInfo.format = QLatin1String("Ogg Vorbis");
    } else if (dynamic_cast<TagLib::FLAC::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("FLAC");
    } else if (dynamic_cast<TagLib::MPC::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("MPC");
    } else if ((speexProperties =
                dynamic_cast<TagLib::Ogg::Speex::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("Speex %1")).arg(speexProperties->speexVersion());
    } else if ((ttaProperties =
                dynamic_cast<TagLib::TrueAudio::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QLatin1String("True Audio ");
      m_detailInfo.format += QString::number(ttaProperties->ttaVersion());
      m_detailInfo.format += QLatin1Char(' ');
      m_detailInfo.format += QString::number(ttaProperties->bitsPerSample());
      m_detailInfo.format += QLatin1String(" bit");
    } else if ((wvProperties =
                dynamic_cast<TagLib::WavPack::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QLatin1String("WavPack ");
      m_detailInfo.format += QString::number(wvProperties->version(), 16);
      m_detailInfo.format += QLatin1Char(' ');
      m_detailInfo.format += QString::number(wvProperties->bitsPerSample());
      m_detailInfo.format += QLatin1String(" bit");
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
    } else if (dynamic_cast<TagLib::MP4::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("MP4");
#endif
#ifdef TAGLIB_WITH_ASF
    } else if (dynamic_cast<TagLib::ASF::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("ASF");
#endif
    } else if (dynamic_cast<TagLib::RIFF::AIFF::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("AIFF");
    } else if (dynamic_cast<TagLib::RIFF::WAV::Properties*>(audioProperties) != 0) {
      m_detailInfo.format = QLatin1String("WAV");
#endif
#if TAGLIB_VERSION >= 0x010700
    } else if ((apeProperties =
                dynamic_cast<TagLib::APE::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("APE %1.%2 %3 bit")).
        arg(apeProperties->version() / 1000).
        arg(apeProperties->version() % 1000).
        arg(apeProperties->bitsPerSample());
#endif
#if TAGLIB_VERSION >= 0x010800
    } else if ((modProperties =
                dynamic_cast<TagLib::Mod::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("Mod %1 %2 Instruments")).
          arg(getTrackerName()).
          arg(modProperties->instrumentCount());
    } else if ((s3mProperties =
                dynamic_cast<TagLib::S3M::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("S3M %1 V%2 T%3")).
          arg(getTrackerName()).
          arg(s3mProperties->fileFormatVersion()).
          arg(s3mProperties->trackerVersion(), 0, 16);
      m_detailInfo.channelMode = s3mProperties->stereo()
          ? DetailInfo::CM_Stereo : DetailInfo::CM_None;
    } else if ((itProperties =
                dynamic_cast<TagLib::IT::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("IT %1 V%2 %3 Instruments")).
          arg(getTrackerName()).
          arg(itProperties->version(), 0, 16).
          arg(itProperties->instrumentCount());
      m_detailInfo.channelMode = itProperties->stereo()
          ? DetailInfo::CM_Stereo : DetailInfo::CM_None;
#ifdef HAVE_TAGLIB_XM_SUPPORT
    } else if ((xmProperties =
                dynamic_cast<TagLib::XM::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("XM %1 V%2 %3 Instruments")).
          arg(getTrackerName()).
          arg(xmProperties->version(), 0, 16).
          arg(xmProperties->instrumentCount());
#endif
#endif
#if TAGLIB_VERSION >= 0x010900
    } else if ((opusProperties =
          dynamic_cast<TagLib::Ogg::Opus::Properties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("Opus %1")).
          arg(opusProperties->opusVersion());
#endif
#if TAGLIB_VERSION >= 0x010901
    } else if ((dsfProperties =
          dynamic_cast<DSFProperties*>(audioProperties)) != 0) {
      m_detailInfo.format = QString(QLatin1String("DSF %1")).
          arg(dsfProperties->version());
#endif
    }
    m_detailInfo.bitrate = audioProperties->bitrate();
    m_detailInfo.sampleRate = audioProperties->sampleRate();
    if (audioProperties->channels() > 0) {
      m_detailInfo.channels = audioProperties->channels();
    }
    m_detailInfo.duration = audioProperties->length();
  } else {
    m_detailInfo.valid = false;
  }
}

#if TAGLIB_VERSION >= 0x010800
/**
 * Get tracker name of a module file.
 *
 * @return tracker name, null if not found.
 */
QString TagLibFile::getTrackerName() const
{
  QString trackerName;
  if (TagLib::Mod::Tag* modTag = dynamic_cast<TagLib::Mod::Tag*>(m_tag[Frame::Tag_2])) {
    trackerName = toQString(modTag->trackerName()).trimmed();
  }
  return trackerName;
}


/**
 * Set m_id3v2Version to 3 or 4 from tag if it exists, else to 0.
 * @param id3v2Tag ID3v2 tag
 */
void TagLibFile::setId3v2VersionFromTag(TagLib::ID3v2::Tag* id3v2Tag)
{
  TagLib::ID3v2::Header* header;
  m_id3v2Version = 0;
  if (id3v2Tag && (header = id3v2Tag->header()) != 0) {
    if (!id3v2Tag->isEmpty()) {
      m_id3v2Version = header->majorVersion();
    } else {
      header->setMajorVersion(TagConfig::instance().id3v2Version() ==
                              TagConfig::ID3v2_3_0 ? 3 : 4);
    }
  }
}

/**
 * Set m_id3v2Version from given value (3 or 4) or use default from
 * configuration if not already set to 3 or 4.
 * @param id3v2Version 3 or 4 to force version, 0 to use existing version
 * or default
 */
void TagLibFile::setId3v2VersionOrDefault(int id3v2Version)
{
  if (id3v2Version == 3 || id3v2Version == 4) {
    m_id3v2Version = id3v2Version;
  }
  if (m_id3v2Version != 3 && m_id3v2Version != 4) {
    m_id3v2Version = TagConfig::instance().id3v2Version() ==
        TagConfig::ID3v2_3_0 ? 3 : 4;
  }
}
#endif

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned TagLibFile::getDuration() const
{
  return m_detailInfo.valid ? m_detailInfo.duration : 0;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString TagLibFile::getFileExtension() const
{
  return m_fileExtension;
}

/**
 * Get the format of a tag.
 *
 * @param tag tag, 0 if no tag available
 * @param type the tag type is returned here
 *
 * @return string describing format of tag,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormat(const TagLib::Tag* tag, TagType& type)
{
  if (tag && !tag->isEmpty()) {
    const TagLib::ID3v2::Tag* id3v2Tag;
    if (dynamic_cast<const TagLib::ID3v1::Tag*>(tag) != 0) {
      type = TT_Id3v1;
      return QLatin1String("ID3v1.1");
    } else if ((id3v2Tag = dynamic_cast<const TagLib::ID3v2::Tag*>(tag)) != 0) {
      type = TT_Id3v2;
      TagLib::ID3v2::Header* header = id3v2Tag->header();
      if (header) {
        uint majorVersion = header->majorVersion();
        uint revisionNumber = header->revisionNumber();
#if (TAGLIB_VERSION <= 0x010400)
        // A wrong majorVersion is returned if a new ID3v2.4.0 tag is created.
        if (majorVersion == 0 && revisionNumber == 0) {
          majorVersion = 4;
        }
#endif
        return QString(QLatin1String("ID3v2.%1.%2")).
          arg(majorVersion).arg(revisionNumber);
      } else {
        return QLatin1String("ID3v2");
      }
    } else if (dynamic_cast<const TagLib::Ogg::XiphComment*>(tag) != 0) {
      type = TT_Vorbis;
      return QLatin1String("Vorbis");
    } else if (dynamic_cast<const TagLib::APE::Tag*>(tag) != 0) {
      type = TT_Ape;
      return QLatin1String("APE");
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
    } else if (dynamic_cast<const TagLib::MP4::Tag*>(tag) != 0) {
      type = TT_Mp4;
      return QLatin1String("MP4");
#endif
#ifdef TAGLIB_WITH_ASF
    } else if (dynamic_cast<const TagLib::ASF::Tag*>(tag) != 0) {
      type = TT_Asf;
      return QLatin1String("ASF");
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
    } else if (dynamic_cast<const TagLib::RIFF::Info::Tag*>(tag) != 0) {
      type = TT_Info;
      return QLatin1String("RIFF INFO");
#endif
    }
  }
  type = TT_Unknown;
  return QString();
}

/**
 * Get the format of tag.
 *
 * @param tagNr tag number
 * @return string describing format of tag,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormat(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_tagFormat[tagNr] : QString();
}


/** Types and descriptions for id3lib frame IDs */
static const struct TypeStrOfId {
  Frame::Type type;
  const char* str;
  bool supported;
} typeStrOfId[] = {
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "AENC - Audio encryption"), false },
  { Frame::FT_Picture,        QT_TRANSLATE_NOOP("@default", "APIC - Attached picture"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "ASPI - Audio seek point index"), false },
#if TAGLIB_VERSION >= 0x010a00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "CHAP - Chapter"), true },
#endif
  { Frame::FT_Comment,        QT_TRANSLATE_NOOP("@default", "COMM - Comments"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "COMR - Commercial"), false },
#if TAGLIB_VERSION >= 0x010a00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "CTOC - Table of contents"), true },
#endif
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "ENCR - Encryption method registration"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "EQU2 - Equalisation (2)"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "ETCO - Event timing codes"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "GEOB - General encapsulated object"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "GRID - Group identification registration"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "LINK - Linked information"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MCDI - Music CD identifier"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MLLT - MPEG location lookup table"), false },
#ifdef TAGLIB_WITH_MP4_SHWM
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MVIN - Movement Number"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MVNM - Movement Name"), true },
#endif
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "OWNE - Ownership frame"),
#if TAGLIB_VERSION >= 0x010800
    true
#else
    false
#endif
  },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "PRIV - Private frame"),
#if TAGLIB_VERSION >= 0x010600
      true
#else
      false
#endif
  },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "PCNT - Play counter"), false },
#if TAGLIB_VERSION >= 0x010b00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "PCST - Podcast"), true },
#endif
  { Frame::FT_Rating,         QT_TRANSLATE_NOOP("@default", "POPM - Popularimeter"),
#if TAGLIB_VERSION >= 0x010600
    true
#else
    false
#endif
  },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "POSS - Position synchronisation frame"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RBUF - Recommended buffer size"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RVA2 - Relative volume adjustment (2)"),
#if TAGLIB_VERSION >= 0x010500
    true
#else
    false
#endif
  },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RVRB - Reverb"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SEEK - Seek frame"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SIGN - Signature frame"), false },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SYLT - Synchronized lyric/text"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SYTC - Synchronized tempo codes"), false },
  { Frame::FT_Album,          QT_TRANSLATE_NOOP("@default", "TALB - Album/Movie/Show title"), true },
  { Frame::FT_Bpm,            QT_TRANSLATE_NOOP("@default", "TBPM - BPM (beats per minute)"), true },
  { Frame::FT_Compilation,    QT_TRANSLATE_NOOP("@default", "TCMP - iTunes compilation flag"), true },
  { Frame::FT_Composer,       QT_TRANSLATE_NOOP("@default", "TCOM - Composer"), true },
  { Frame::FT_Genre,          QT_TRANSLATE_NOOP("@default", "TCON - Content type"), true },
  { Frame::FT_Copyright,      QT_TRANSLATE_NOOP("@default", "TCOP - Copyright message"), true },
  { Frame::FT_EncodingTime,   QT_TRANSLATE_NOOP("@default", "TDEN - Encoding time"), true },
#if TAGLIB_VERSION >= 0x010b00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TDES - Podcast description"), true },
#endif
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TDLY - Playlist delay"), true },
  { Frame::FT_OriginalDate,   QT_TRANSLATE_NOOP("@default", "TDOR - Original release time"), true },
  { Frame::FT_Date,           QT_TRANSLATE_NOOP("@default", "TDRC - Recording time"), true },
  { Frame::FT_ReleaseDate,    QT_TRANSLATE_NOOP("@default", "TDRL - Release time"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TDTG - Tagging time"), true },
  { Frame::FT_EncodedBy,      QT_TRANSLATE_NOOP("@default", "TENC - Encoded by"), true },
  { Frame::FT_Lyricist,       QT_TRANSLATE_NOOP("@default", "TEXT - Lyricist/Text writer"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TFLT - File type"), true },
#if TAGLIB_VERSION >= 0x010b00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TGID - Podcast identifier"), true },
#endif
  { Frame::FT_Arranger,       QT_TRANSLATE_NOOP("@default", "TIPL - Involved people list"), true },
  { Frame::FT_Grouping,       QT_TRANSLATE_NOOP("@default", "TIT1 - Content group description"), true },
  { Frame::FT_Title,          QT_TRANSLATE_NOOP("@default", "TIT2 - Title/songname/content description"), true },
  { Frame::FT_Subtitle,       QT_TRANSLATE_NOOP("@default", "TIT3 - Subtitle/Description refinement"), true },
  { Frame::FT_InitialKey,     QT_TRANSLATE_NOOP("@default", "TKEY - Initial key"), true },
  { Frame::FT_Language,       QT_TRANSLATE_NOOP("@default", "TLAN - Language(s)"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TLEN - Length"), true },
  { Frame::FT_Performer,      QT_TRANSLATE_NOOP("@default", "TMCL - Musician credits list"), true },
  { Frame::FT_Media,          QT_TRANSLATE_NOOP("@default", "TMED - Media type"), true },
  { Frame::FT_Mood,           QT_TRANSLATE_NOOP("@default", "TMOO - Mood"), true },
  { Frame::FT_OriginalAlbum,  QT_TRANSLATE_NOOP("@default", "TOAL - Original album/movie/show title"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TOFN - Original filename"), true },
  { Frame::FT_Author,         QT_TRANSLATE_NOOP("@default", "TOLY - Original lyricist(s)/text writer(s)"), true },
  { Frame::FT_OriginalArtist, QT_TRANSLATE_NOOP("@default", "TOPE - Original artist(s)/performer(s)"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TOWN - File owner/licensee"), true },
  { Frame::FT_Artist,         QT_TRANSLATE_NOOP("@default", "TPE1 - Lead performer(s)/Soloist(s)"), true },
  { Frame::FT_AlbumArtist,    QT_TRANSLATE_NOOP("@default", "TPE2 - Band/orchestra/accompaniment"), true },
  { Frame::FT_Conductor,      QT_TRANSLATE_NOOP("@default", "TPE3 - Conductor/performer refinement"), true },
  { Frame::FT_Remixer,        QT_TRANSLATE_NOOP("@default", "TPE4 - Interpreted, remixed, or otherwise modified by"), true },
  { Frame::FT_Disc,           QT_TRANSLATE_NOOP("@default", "TPOS - Part of a set"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TPRO - Produced notice"), true },
  { Frame::FT_Publisher,      QT_TRANSLATE_NOOP("@default", "TPUB - Publisher"), true },
  { Frame::FT_Track,          QT_TRANSLATE_NOOP("@default", "TRCK - Track number/Position in set"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TRSN - Internet radio station name"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TRSO - Internet radio station owner"), true },
  { Frame::FT_SortAlbumArtist,QT_TRANSLATE_NOOP("@default", "TSO2 - Album artist sort order"), true },
  { Frame::FT_SortAlbum,      QT_TRANSLATE_NOOP("@default", "TSOA - Album sort order"), true },
  { Frame::FT_SortComposer,   QT_TRANSLATE_NOOP("@default", "TSOC - Composer sort order"), true },
  { Frame::FT_SortArtist,     QT_TRANSLATE_NOOP("@default", "TSOP - Performer sort order"), true },
  { Frame::FT_SortName,       QT_TRANSLATE_NOOP("@default", "TSOT - Title sort order"), true },
  { Frame::FT_Isrc,           QT_TRANSLATE_NOOP("@default", "TSRC - ISRC (international standard recording code)"), true },
  { Frame::FT_EncoderSettings,QT_TRANSLATE_NOOP("@default", "TSSE - Software/Hardware and settings used for encoding"), true },
  { Frame::FT_Part,           QT_TRANSLATE_NOOP("@default", "TSST - Set subtitle"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TXXX - User defined text information"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "UFID - Unique file identifier"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "USER - Terms of use"), false },
  { Frame::FT_Lyrics,         QT_TRANSLATE_NOOP("@default", "USLT - Unsynchronized lyric/text transcription"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WCOM - Commercial information"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WCOP - Copyright/Legal information"), true },
#if TAGLIB_VERSION >= 0x010b00
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WFED - Podcast feed"), true },
#endif
  { Frame::FT_WWWAudioFile,   QT_TRANSLATE_NOOP("@default", "WOAF - Official audio file webpage"), true },
  { Frame::FT_Website,        QT_TRANSLATE_NOOP("@default", "WOAR - Official artist/performer webpage"), true },
  { Frame::FT_WWWAudioSource, QT_TRANSLATE_NOOP("@default", "WOAS - Official audio source webpage"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WORS - Official internet radio station homepage"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WPAY - Payment"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WPUB - Official publisher webpage"), true },
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WXXX - User defined URL link"), true }
};

/**
 * Get type and description of frame.
 *
 * @param id   ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here
 */
static void getTypeStringForFrameId(TagLib::ByteVector id, Frame::Type& type,
                                    const char*& str)
{
  static TagLib::Map<TagLib::ByteVector, unsigned> idIndexMap;
  if (idIndexMap.isEmpty()) {
    for (unsigned i = 0;
         i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]);
         ++i) {
      idIndexMap.insert(TagLib::ByteVector(typeStrOfId[i].str, 4), i);
    }
  }
  if (idIndexMap.contains(id)) {
    const TypeStrOfId& ts = typeStrOfId[idIndexMap[id]];
    type = ts.type;
    str = ts.str;
  } else {
    type = Frame::FT_UnknownFrame;
    str = "????";
  }
}

/**
 * Get string description starting with 4 bytes ID.
 *
 * @param type type of frame
 *
 * @return string.
 */
static const char* getStringForType(Frame::Type type)
{
  if (type != Frame::FT_Other) {
    for (unsigned i = 0;
         i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]);
         ++i) {
      const TypeStrOfId& ts = typeStrOfId[i];
      if (ts.type == type) {
        return ts.str;
      }
    }
  }
  return "????";
}

/**
 * Get the fields from a text identification frame.
 *
 * @param tFrame text identification frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromTextFrame(
  const TagLib::ID3v2::TextIdentificationFrame* tFrame,
  Frame::FieldList& fields, Frame::Type type)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = tFrame->textEncoding();
  fields.push_back(field);

  const TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame;
  if ((txxxFrame =
       dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(tFrame))
      != 0) {
    field.m_id = Frame::ID_Description;
    field.m_value = toQString(txxxFrame->description());
    fields.push_back(field);

    TagLib::StringList slText = tFrame->fieldList();
    text = slText.size() > 1 ? toQString(slText[1]) : QLatin1String("");
  } else {
    // if there are multiple items, put them into one string
    // separated by a special separator.
    text = toQString(tFrame->fieldList().toString(Frame::stringListSeparator().toLatin1()));
  }
  field.m_id = Frame::ID_Text;
  if (type == Frame::FT_Genre) {
    text = Genres::getNameString(text);
  }
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an attached picture frame.
 *
 * @param apicFrame attached picture frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromApicFrame(
  const TagLib::ID3v2::AttachedPictureFrame* apicFrame,
  Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = apicFrame->textEncoding();
  fields.push_back(field);

  // for compatibility with ID3v2.3 id3lib
  field.m_id = Frame::ID_ImageFormat;
  field.m_value = QString(QLatin1String(""));
  fields.push_back(field);

  field.m_id = Frame::ID_MimeType;
  field.m_value = toQString(apicFrame->mimeType());
  fields.push_back(field);

  field.m_id = Frame::ID_PictureType;
  field.m_value = apicFrame->type();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  text = toQString(apicFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector pic = apicFrame->picture();
  QByteArray ba;
  ba = QByteArray(pic.data(), pic.size());
  field.m_value = ba;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a comments frame.
 *
 * @param commFrame comments frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromCommFrame(
  const TagLib::ID3v2::CommentsFrame* commFrame, Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = commFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = commFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(commFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Text;
  text = toQString(commFrame->toString());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a unique file identifier frame.
 *
 * @param ufidFrame unique file identifier frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUfidFrame(
  const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Owner;
  field.m_value = toQString(ufidFrame->owner());
  fields.push_back(field);

  field.m_id = Frame::ID_Id;
  TagLib::ByteVector id = ufidFrame->identifier();
  QByteArray ba;
  ba = QByteArray(id.data(), id.size());
  field.m_value = ba;
  fields.push_back(field);

  if (!ba.isEmpty()) {
    QString text(QString::fromLatin1(ba));
    if (ba.size() - text.length() <= 1 &&
        AttributeData::isHexString(text, 'Z')) {
      return text;
    }
  }
  return QString();
}

/**
 * Get the fields from a general encapsulated object frame.
 *
 * @param geobFrame general encapsulated object frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromGeobFrame(
  const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame,
  Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = geobFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_MimeType;
  field.m_value = toQString(geobFrame->mimeType());
  fields.push_back(field);

  field.m_id = Frame::ID_Filename;
  field.m_value = toQString(geobFrame->fileName());
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  text = toQString(geobFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector obj = geobFrame->object();
  QByteArray ba;
  ba = QByteArray(obj.data(), obj.size());
  field.m_value = ba;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a URL link frame.
 *
 * @param wFrame URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUrlFrame(
  const TagLib::ID3v2::UrlLinkFrame* wFrame, Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_Url;
  text = toQString(wFrame->url());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a user URL link frame.
 *
 * @param wxxxFrame user URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUserUrlFrame(
  const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame, Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = wxxxFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(wxxxFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Url;
  text = toQString(wxxxFrame->url());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an unsynchronized lyrics frame.
 * This is copy-pasted from editCommFrame().
 *
 * @param usltFrame unsynchronized frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUsltFrame(
  const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame,
  Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = usltFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = usltFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(usltFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Text;
  text = toQString(usltFrame->toString());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a synchronized lyrics frame.
 *
 * @param syltFrame synchronized lyrics frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromSyltFrame(
  const TagLib::ID3v2::SynchronizedLyricsFrame* syltFrame,
  Frame::FieldList& fields)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = syltFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = syltFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = syltFrame->timestampFormat();
  fields.push_back(field);

  field.m_id = Frame::ID_ContentType;
  field.m_value = syltFrame->type();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  text = toQString(syltFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList synchedData;
  TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList stl =
      syltFrame->synchedText();
  for (TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList::ConstIterator
       it = stl.begin();
       it != stl.end();
       ++it) {
    synchedData.append(static_cast<quint32>(it->time));
    synchedData.append(toQString(it->text));
  }
  field.m_value = synchedData;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an event timing codes frame.
 *
 * @param etcoFrame event timing codes frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromEtcoFrame(
  const TagLib::ID3v2::EventTimingCodesFrame* etcoFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = etcoFrame->timestampFormat();
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList synchedData;
  TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList sel =
      etcoFrame->synchedEvents();
  for (TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList::ConstIterator
       it = sel.begin();
       it != sel.end();
       ++it) {
    synchedData.append(static_cast<quint32>(it->time));
    synchedData.append(static_cast<int>(it->type));
  }
  field.m_value = synchedData;
  fields.push_back(field);

  return QString();
}

#if TAGLIB_VERSION >= 0x010600
/**
 * Get the fields from a private frame.
 *
 * @param privFrame private frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromPrivFrame(
  const TagLib::ID3v2::PrivateFrame* privFrame,
  Frame::FieldList& fields)
{
  QString owner;
  Frame::Field field;
  field.m_id = Frame::ID_Owner;
  owner = toQString(privFrame->owner());
  field.m_value = owner;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector data = privFrame->data();
  QByteArray ba;
  ba = QByteArray(data.data(), data.size());
  field.m_value = ba;
  fields.push_back(field);

  if (!owner.isEmpty() && !ba.isEmpty()) {
    QString str;
    if (AttributeData(owner).toString(ba, str)) {
      return str;
    }
  }
  return QString();
}

/**
 * Get the fields from a popularimeter frame.
 *
 * @param popmFrame popularimeter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromPopmFrame(
  const TagLib::ID3v2::PopularimeterFrame* popmFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Email;
  field.m_value = toQString(popmFrame->email());
  fields.push_back(field);

  field.m_id = Frame::ID_Rating;
  field.m_value = popmFrame->rating();
  QString text(field.m_value.toString());
  fields.push_back(field);

  field.m_id = Frame::ID_Counter;
  field.m_value = popmFrame->counter();
  fields.push_back(field);

  return text;
}
#endif

#if TAGLIB_VERSION >= 0x010800
/**
 * Get the fields from an ownership frame.
 *
 * @param owneFrame ownership frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromOwneFrame(
  const TagLib::ID3v2::OwnershipFrame* owneFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = owneFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Date;
  field.m_value = toQString(owneFrame->datePurchased());
  fields.push_back(field);

  field.m_id = Frame::ID_Price;
  field.m_value = toQString(owneFrame->pricePaid());
  fields.push_back(field);

  field.m_id = Frame::ID_Seller;
  QString text(toQString(owneFrame->seller()));
  field.m_value = text;
  fields.push_back(field);

  return text;
}
#endif

#if TAGLIB_VERSION >= 0x010500
/**
 * Get a string representation of the data in an RVA2 frame.
 * @param rva2Frame RVA2 frame
 * @return string containing lines with space separated values for
 * type of channel, volume adjustment, bits representing peak,
 * peak volume. The peak volume is a hex byte array, the other values
 * are integers, the volume adjustment is signed. Bits representing peak
 * and peak volume are omitted if they have zero bits.
 */
static QString rva2FrameToString(
    const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame)
{
  QString text;
  TagLib::List<TagLib::ID3v2::RelativeVolumeFrame::ChannelType> channels =
      rva2Frame->channels();
  for (TagLib::List<TagLib::ID3v2::RelativeVolumeFrame::ChannelType>::
       ConstIterator it = channels.begin();
       it != channels.end();
       ++it) {
    TagLib::ID3v2::RelativeVolumeFrame::ChannelType type = *it;
    if (!text.isEmpty()) {
      text += QLatin1Char('\n');
    }
    short adj = rva2Frame->volumeAdjustmentIndex(type);
    TagLib::ID3v2::RelativeVolumeFrame::PeakVolume peak =
        rva2Frame->peakVolume(type);
    text += QString::number(type);
    text += QLatin1Char(' ');
    text += QString::number(adj);
    if (peak.bitsRepresentingPeak > 0) {
      text += QLatin1Char(' ');
      text += QString::number(peak.bitsRepresentingPeak);
      text += QLatin1Char(' ');
      text += QString::fromLatin1(
            QByteArray(peak.peakVolume.data(), peak.peakVolume.size()).toHex());
    }
  }
  return text;
}

/**
 * Set the data in an RVA2 frame from a string representation.
 * @param rva2Frame RVA2 frame to set
 * @param text string representation
 * @see rva2FrameToString()
 */
static void rva2FrameFromString(TagLib::ID3v2::RelativeVolumeFrame* rva2Frame,
                                const TagLib::String& text)
{
  // Unfortunately, it is not possible to remove data for a specific channel.
  // Only the whole frame could be deleted and a new one created.
  foreach (const QString& line, toQString(text).split(QLatin1Char('\n'))) {
    QStringList strs = line.split(QLatin1Char(' '));
    if (strs.size() > 1) {
      bool ok;
      int typeInt = strs.at(0).toInt(&ok);
      if (ok && typeInt >= 0 && typeInt <= 8) {
        short adj = strs.at(1).toShort(&ok);
        if (ok) {
          TagLib::ID3v2::RelativeVolumeFrame::ChannelType type =
              static_cast<TagLib::ID3v2::RelativeVolumeFrame::ChannelType>(
                typeInt);
          rva2Frame->setVolumeAdjustmentIndex(adj, type);
          TagLib::ID3v2::RelativeVolumeFrame::PeakVolume peak;
          if (strs.size() > 3) {
            int bitsInt = strs.at(2).toInt(&ok);
            QByteArray ba = QByteArray::fromHex(strs.at(3).toLatin1());
            if (ok && bitsInt > 0 && bitsInt <= 255 &&
                bitsInt <= ba.size() * 8) {
              peak.bitsRepresentingPeak = bitsInt;
              peak.peakVolume.setData(ba.constData(), ba.size());
              rva2Frame->setPeakVolume(peak, type);
            }
          }
        }
      }
    }
  }
}

/**
 * Get the fields from a relative volume frame.
 *
 * @param rva2Frame relative volume frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromRva2Frame(
  const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  field.m_value = toQString(rva2Frame->identification());
  fields.push_back(field);

  QString text = rva2FrameToString(rva2Frame);
  field.m_id = Frame::ID_Text;
  field.m_value = text;
  fields.push_back(field);
  return text;
}
#endif

#if TAGLIB_VERSION >= 0x010a00
static Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index);

/**
 * Get the fields from a chapter frame.
 *
 * @param chapFrame chapter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromChapFrame(
  const TagLib::ID3v2::ChapterFrame* chapFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  QString text = toQString(
        TagLib::String(chapFrame->elementID(), TagLib::String::Latin1));
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList data;
  data.append(chapFrame->startTime());
  data.append(chapFrame->endTime());
  data.append(chapFrame->startOffset());
  data.append(chapFrame->endOffset());
  field.m_value = data;
  fields.push_back(field);

  field.m_id = Frame::ID_Subframe;
  const TagLib::ID3v2::FrameList& frameList = chapFrame->embeddedFrameList();
  for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
       it != frameList.end();
       ++it) {
    Frame frame(createFrameFromId3Frame(*it, -1));
    field.m_value = frame.getExtendedType().getName();
    fields.push_back(field);
    fields.append(frame.getFieldList());
  }

  return text;
}

/**
 * Get the fields from a table of contents frame.
 *
 * @param ctocFrame table of contents frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromCtocFrame(
  const TagLib::ID3v2::TableOfContentsFrame* ctocFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  QString text = toQString(
        TagLib::String(ctocFrame->elementID(), TagLib::String::Latin1));
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList data;
  data.append(ctocFrame->isTopLevel());
  data.append(ctocFrame->isOrdered());
  QStringList elements;
  TagLib::ByteVectorList childElements = ctocFrame->childElements();
  for (TagLib::ByteVectorList::ConstIterator it = childElements.begin();
       it != childElements.end();
       ++it) {
    elements.append(toQString(TagLib::String(*it, TagLib::String::Latin1)));
  }
  data.append(elements);
  field.m_value = data;
  fields.push_back(field);

  field.m_id = Frame::ID_Subframe;
  const TagLib::ID3v2::FrameList& frameList = ctocFrame->embeddedFrameList();
  for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
       it != frameList.end();
       ++it) {
    Frame frame(createFrameFromId3Frame(*it, -1));
    field.m_value = frame.getExtendedType().getName();
    fields.push_back(field);
    fields.append(frame.getFieldList());
  }

  return text;
}
#endif

/**
 * Get the fields from an unknown frame.
 *
 * @param unknownFrame unknown frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUnknownFrame(
  const TagLib::ID3v2::Frame* unknownFrame, Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Data;
  TagLib::ByteVector dat = unknownFrame->render();
  QByteArray ba;
  ba = QByteArray(dat.data(), dat.size());
  field.m_value = ba;
  fields.push_back(field);
  return QString();
}

/**
 * Get the fields from an ID3v2 tag.
 *
 * @param frame  frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromId3Frame(const TagLib::ID3v2::Frame* frame,
                                     Frame::FieldList& fields, Frame::Type type)
{
  if (frame) {
    const TagLib::ID3v2::TextIdentificationFrame* tFrame;
    const TagLib::ID3v2::AttachedPictureFrame* apicFrame;
    const TagLib::ID3v2::CommentsFrame* commFrame;
    const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame;
    const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
    const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
    const TagLib::ID3v2::UrlLinkFrame* wFrame;
    const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
    const TagLib::ID3v2::SynchronizedLyricsFrame* syltFrame;
    const TagLib::ID3v2::EventTimingCodesFrame* etcoFrame;
#if TAGLIB_VERSION >= 0x010600
    const TagLib::ID3v2::PrivateFrame* privFrame;
    const TagLib::ID3v2::PopularimeterFrame* popmFrame;
#endif
#if TAGLIB_VERSION >= 0x010800
    const TagLib::ID3v2::OwnershipFrame* owneFrame;
#endif
#if TAGLIB_VERSION >= 0x010500
    const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame;
#endif
#if TAGLIB_VERSION >= 0x010a00
    const TagLib::ID3v2::ChapterFrame* chapFrame;
    const TagLib::ID3v2::TableOfContentsFrame* ctocFrame;
#endif
    if ((tFrame =
         dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(frame)) !=
        0) {
      return getFieldsFromTextFrame(tFrame, fields, type);
    } else if ((apicFrame =
                dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame))
               != 0) {
      return getFieldsFromApicFrame(apicFrame, fields);
    } else if ((commFrame = dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(
                  frame)) != 0) {
      return getFieldsFromCommFrame(commFrame, fields);
    } else if ((ufidFrame =
                dynamic_cast<const TagLib::ID3v2::UniqueFileIdentifierFrame*>(
                  frame)) != 0) {
      return getFieldsFromUfidFrame(ufidFrame, fields);
    } else if ((geobFrame =
                dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
                  frame)) != 0) {
      return getFieldsFromGeobFrame(geobFrame, fields);
    } else if ((wxxxFrame = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(
                  frame)) != 0) {
      return getFieldsFromUserUrlFrame(wxxxFrame, fields);
    } else if ((wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(
                  frame)) != 0) {
      return getFieldsFromUrlFrame(wFrame, fields);
    } else if ((usltFrame = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
                frame)) != 0) {
      return getFieldsFromUsltFrame(usltFrame, fields);
    } else if ((syltFrame = dynamic_cast<const TagLib::ID3v2::SynchronizedLyricsFrame*>(
                frame)) != 0) {
      return getFieldsFromSyltFrame(syltFrame, fields);
    } else if ((etcoFrame = dynamic_cast<const TagLib::ID3v2::EventTimingCodesFrame*>(
                frame)) != 0) {
      return getFieldsFromEtcoFrame(etcoFrame, fields);
#if TAGLIB_VERSION >= 0x010600
    } else if ((privFrame = dynamic_cast<const TagLib::ID3v2::PrivateFrame*>(
                frame)) != 0) {
      return getFieldsFromPrivFrame(privFrame, fields);
    } else if ((popmFrame = dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>(
                frame)) != 0) {
      return getFieldsFromPopmFrame(popmFrame, fields);
#endif
#if TAGLIB_VERSION >= 0x010800
    } else if ((owneFrame = dynamic_cast<const TagLib::ID3v2::OwnershipFrame*>(
                frame)) != 0) {
      return getFieldsFromOwneFrame(owneFrame, fields);
#endif
#if TAGLIB_VERSION >= 0x010500
    } else if ((rva2Frame = dynamic_cast<const TagLib::ID3v2::RelativeVolumeFrame*>(
                frame)) != 0) {
      return getFieldsFromRva2Frame(rva2Frame, fields);
#endif
#if TAGLIB_VERSION >= 0x010a00
    } else if ((chapFrame = dynamic_cast<const TagLib::ID3v2::ChapterFrame*>(
                frame)) != 0) {
      return getFieldsFromChapFrame(chapFrame, fields);
    } else if ((ctocFrame = dynamic_cast<const TagLib::ID3v2::TableOfContentsFrame*>(
                frame)) != 0) {
      return getFieldsFromCtocFrame(ctocFrame, fields);
#endif
    } else {
      TagLib::ByteVector id = frame->frameID();
#if TAGLIB_VERSION < 0x010500
      if (id.startsWith("WXXX")) {
        TagLib::ID3v2::UserUrlLinkFrame userUrlLinkFrame(frame->render());
        return getFieldsFromUserUrlFrame(&userUrlLinkFrame, fields);
      } else if (id.startsWith("W")) {
        TagLib::ID3v2::UrlLinkFrame urlLinkFrame(frame->render());
        return getFieldsFromUrlFrame(&urlLinkFrame, fields);
      } else if (id.startsWith("USLT")) {
        TagLib::ID3v2::UnsynchronizedLyricsFrame usltFrm(frame->render());
        return getFieldsFromUsltFrame(&usltFrm, fields);
      } else if (id.startsWith("GEOB")) {
        TagLib::ID3v2::GeneralEncapsulatedObjectFrame geobFrm(frame->render());
        return getFieldsFromGeobFrame(&geobFrm, fields);
      } else
#endif
#if TAGLIB_VERSION < 0x010a00
      if (id.startsWith("SYLT")) {
        TagLib::ID3v2::SynchronizedLyricsFrame syltFrm(frame->render());
        return getFieldsFromSyltFrame(&syltFrm, fields);
      } else if (id.startsWith("ETCO")) {
        TagLib::ID3v2::EventTimingCodesFrame etcoFrm(frame->render());
        return getFieldsFromEtcoFrame(&etcoFrm, fields);
      } else
#endif
        return getFieldsFromUnknownFrame(frame, fields);
    }
  }
  return QString();
}

/**
 * Convert a string to a language code byte vector.
 *
 * @param str string containing language code.
 *
 * @return 3 byte vector with language code.
 */
static TagLib::ByteVector languageCodeByteVector(QString str)
{
  uint len = str.length();
  if (len > 3) {
    str.truncate(3);
  } else if (len < 3) {
    for (uint i = len; i < 3; ++i) {
      str += QLatin1Char(' ');
    }
  }
  return TagLib::ByteVector(str.toLatin1().data(), str.length());
}

/**
 * The following template functions are used to uniformly set fields
 * in the different ID3v2 frames.
 */
//! @cond
template <class T>
void setTextEncoding(T*, TagLib::String::Type) {}

template <>
void setTextEncoding(TagLib::ID3v2::TextIdentificationFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::AttachedPictureFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::CommentsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UserUrlLinkFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}


template <class T>
void setDescription(T*, const Frame::Field&) {}

template <>
void setDescription(TagLib::ID3v2::UserTextIdentificationFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::AttachedPictureFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <class T>
void setMimeType(T*, const Frame::Field&) {}

template <>
void setMimeType(TagLib::ID3v2::AttachedPictureFrame* f,
                 const Frame::Field& fld)
{
  f->setMimeType(toTString(fld.m_value.toString()));
}

template <>
void setMimeType(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                 const Frame::Field& fld)
{
  f->setMimeType(toTString(fld.m_value.toString()));
}

template <class T>
void setPictureType(T*, const Frame::Field&) {}

template <>
void setPictureType(TagLib::ID3v2::AttachedPictureFrame* f,
                    const Frame::Field& fld)
{
  f->setType(
    static_cast<TagLib::ID3v2::AttachedPictureFrame::Type>(
      fld.m_value.toInt()));
}

template <class T>
void setData(T*, const Frame::Field&) {}

template <>
void setData(TagLib::ID3v2::Frame* f, const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setData(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::AttachedPictureFrame* f, const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setPicture(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setObject(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::SynchronizedLyricsFrame* f,
             const Frame::Field& fld)
{
  TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList stl;
  QVariantList synchedData(fld.m_value.toList());
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 time = it.next().toUInt();
    if (!it.hasNext())
      break;

    TagLib::String text = toTString(it.next().toString());
    stl.append(TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText(time, text));
  }
  f->setSynchedText(stl);
}

template <>
void setData(TagLib::ID3v2::EventTimingCodesFrame* f,
             const Frame::Field& fld)
{
  TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList sel;
  QVariantList synchedData(fld.m_value.toList());
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 time = it.next().toUInt();
    if (!it.hasNext())
      break;

    TagLib::ID3v2::EventTimingCodesFrame::EventType type =
        static_cast<TagLib::ID3v2::EventTimingCodesFrame::EventType>(
          it.next().toInt());
    sel.append(TagLib::ID3v2::EventTimingCodesFrame::SynchedEvent(time, type));
  }
  f->setSynchedEvents(sel);
}

template <class T>
void setLanguage(T*, const Frame::Field&) {}

template <>
void setLanguage(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <>
void setLanguage(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                 const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <>
void setLanguage(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                 const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <class T>
void setOwner(T*, const Frame::Field&) {}

template <>
void setOwner(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
              const Frame::Field& fld)
{
  f->setOwner(toTString(fld.m_value.toString()));
}

#if TAGLIB_VERSION >= 0x010600
template <>
void setOwner(TagLib::ID3v2::PrivateFrame* f,
              const Frame::Field& fld)
{
  f->setOwner(toTString(fld.m_value.toString()));
}

template <>
void setData(TagLib::ID3v2::PrivateFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setData(TagLib::ByteVector(ba.data(), ba.size()));
}
#endif

template <class T>
void setIdentifier(T*, const Frame::Field&) {}

template <>
void setIdentifier(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
                   const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setFilename(T*, const Frame::Field&) {}

template <>
void setFilename(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                 const Frame::Field& fld)
{
  f->setFileName(toTString(fld.m_value.toString()));
}

template <class T>
void setUrl(T*, const Frame::Field&) {}

template <>
void setUrl(TagLib::ID3v2::UrlLinkFrame* f, const Frame::Field& fld)
{
  f->setUrl(toTString(fld.m_value.toString()));
}

template <>
void setUrl(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
  f->setUrl(toTString(fld.m_value.toString()));
}

template <class T>
void setValue(T* f, const TagLib::String& text)
{
  f->setText(text);
}

template <>
void setValue(TagLib::ID3v2::AttachedPictureFrame* f, const TagLib::String& text)
{
  f->setDescription(text);
}

template <>
void setValue(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
              const TagLib::String& text)
{
  f->setDescription(text);
}

static void setStringOrList(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
  if (text.find(Frame::stringListSeparator().toLatin1()) == -1) {
    f->setText(text);
  } else {
    f->setText(TagLib::StringList::split(text, Frame::stringListSeparator().toLatin1()));
  }
}

template <>
void setValue(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
  setStringOrList(f, text);
}

template <>
void setValue(TagLib::ID3v2::UniqueFileIdentifierFrame* f, const TagLib::String& text)
{
  if (AttributeData::isHexString(toQString(text), 'Z')) {
    TagLib::ByteVector data(text.data(TagLib::String::Latin1));
    data.append('\0');
    f->setIdentifier(data);
  }
}

template <>
void setValue(TagLib::ID3v2::SynchronizedLyricsFrame* f, const TagLib::String& text)
{
  f->setDescription(text);
}

#if TAGLIB_VERSION >= 0x010600
template <>
void setValue(TagLib::ID3v2::PrivateFrame* f, const TagLib::String& text)
{
  QByteArray newData;
  TagLib::String owner = f->owner();
  if (!owner.isEmpty() &&
      AttributeData(toQString(owner)).
      toByteArray(toQString(text), newData)) {
    f->setData(TagLib::ByteVector(newData.data(), newData.size()));
  }
}

template <>
void setValue(TagLib::ID3v2::PopularimeterFrame* f, const TagLib::String& text)
{
  f->setRating(text.toInt());
}
#endif

template <class T>
void setText(T* f, const TagLib::String& text)
{
  f->setText(text);
}

template <>
void setText(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
  setStringOrList(f, text);
}

#if TAGLIB_VERSION >= 0x010600
template <class T>
void setEmail(T*, const Frame::Field&) {}

template <>
void setEmail(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setEmail(toTString(fld.m_value.toString()));
}

template <class T>
void setRating(T*, const Frame::Field&) {}

template <>
void setRating(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setRating(fld.m_value.toInt());
}

template <class T>
void setCounter(T*, const Frame::Field&) {}

template <>
void setCounter(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setCounter(fld.m_value.toUInt());
}
#endif

#if TAGLIB_VERSION >= 0x010800
template <class T>
void setDate(T*, const Frame::Field&) {}

template <>
void setDate(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  // The date string must have exactly 8 characters (should be YYYYMMDD)
  QString date(fld.m_value.toString().leftJustified(8, QLatin1Char(' '), true));
  f->setDatePurchased(toTString(date));
}

template <class T>
void setPrice(T*, const Frame::Field&) {}

template <>
void setPrice(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  f->setPricePaid(toTString(fld.m_value.toString()));
}

template <class T>
void setSeller(T*, const Frame::Field&) {}

template <>
void setSeller(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  f->setSeller(toTString(fld.m_value.toString()));
}

template <>
void setTextEncoding(TagLib::ID3v2::OwnershipFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setValue(TagLib::ID3v2::OwnershipFrame* f, const TagLib::String& text)
{
  f->setSeller(text);
}
#endif

template <class T>
void setTimestampFormat(T*, const Frame::Field&) {}

template <>
void setTimestampFormat(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                        const Frame::Field& fld)
{
  f->setTimestampFormat(
        static_cast<TagLib::ID3v2::SynchronizedLyricsFrame::TimestampFormat>(
          fld.m_value.toInt()));
}

template <>
void setTimestampFormat(TagLib::ID3v2::EventTimingCodesFrame* f,
                        const Frame::Field& fld)
{
  f->setTimestampFormat(
        static_cast<TagLib::ID3v2::EventTimingCodesFrame::TimestampFormat>(
          fld.m_value.toInt()));
}

template <class T>
void setContentType(T*, const Frame::Field&) {}

template <>
void setContentType(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setType(static_cast<TagLib::ID3v2::SynchronizedLyricsFrame::Type>(
               fld.m_value.toInt()));
}

#if TAGLIB_VERSION >= 0x010500
template <>
void setIdentifier(TagLib::ID3v2::RelativeVolumeFrame* f,
                   const Frame::Field& fld)
{
  f->setIdentification(toTString(fld.m_value.toString()));
}

template <>
void setText(TagLib::ID3v2::RelativeVolumeFrame* f, const TagLib::String& text)
{
  rva2FrameFromString(f, text);
}

template <>
void setValue(TagLib::ID3v2::RelativeVolumeFrame* f, const TagLib::String& text)
{
  rva2FrameFromString(f, text);
}
#endif

#if TAGLIB_VERSION >= 0x010a00
static TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
                                                     Frame& frame);

template <>
void setIdentifier(TagLib::ID3v2::ChapterFrame* f,
                   const Frame::Field& fld)
{
  QByteArray id = fld.m_value.toString().toLatin1();
  f->setElementID(TagLib::ByteVector(id.constData(), id.size()));
}

template <>
void setIdentifier(TagLib::ID3v2::TableOfContentsFrame* f,
                   const Frame::Field& fld)
{
  QByteArray id = fld.m_value.toString().toLatin1();
  f->setElementID(TagLib::ByteVector(id.constData(), id.size()));
}

template <>
void setData(TagLib::ID3v2::ChapterFrame* f,
             const Frame::Field& fld)
{
  QVariantList data(fld.m_value.toList());
  if (data.size() == 4) {
    f->setStartTime(data.at(0).toUInt());
    f->setEndTime(data.at(1).toUInt());
    f->setStartOffset(data.at(2).toUInt());
    f->setEndOffset(data.at(3).toUInt());
  }
  // The embedded frames are deleted here because frames without subframes
  // do not have an ID_Subframe field and setSubframes() is not called.
  TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  for (TagLib::ID3v2::FrameList::ConstIterator it = l.begin();
       it != l.end();
       ++it) {
    f->removeEmbeddedFrame(*it, true);
  }
}

template <>
void setData(TagLib::ID3v2::TableOfContentsFrame* f,
             const Frame::Field& fld)
{
  QVariantList data(fld.m_value.toList());
  if (data.size() >= 3) {
    f->setIsTopLevel(data.at(0).toBool());
    f->setIsOrdered(data.at(1).toBool());
    QStringList elementStrings = data.at(2).toStringList();
    TagLib::ByteVectorList elements;
    for (QStringList::const_iterator it = elementStrings.constBegin();
         it != elementStrings.constEnd();
         ++it) {
      QByteArray id = it->toLatin1();
      elements.append(TagLib::ByteVector(id.constData(), id.size()));
    }
    f->setChildElements(elements);
  }
  // The embedded frames are deleted here because frames without subframes
  // do not have an ID_Subframe field and setSubframes() is not called.
  TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  for (TagLib::ID3v2::FrameList::ConstIterator it = l.begin();
       it != l.end();
       ++it) {
    f->removeEmbeddedFrame(*it, true);
  }
}

template <class T>
void setSubframes(const TagLibFile*, T*, Frame::FieldList::const_iterator,
                  Frame::FieldList::const_iterator) {}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::ChapterFrame* f,
                  Frame::FieldList::const_iterator begin,
                  Frame::FieldList::const_iterator end)
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (FrameCollection::iterator it = frames.begin();
       it != frames.end();
       ++it) {
    f->addEmbeddedFrame(createId3FrameFromFrame(self, const_cast<Frame&>(*it)));
  }
}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::TableOfContentsFrame* f,
                  Frame::FieldList::const_iterator begin,
                  Frame::FieldList::const_iterator end)
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (FrameCollection::iterator it = frames.begin();
       it != frames.end();
       ++it) {
    f->addEmbeddedFrame(createId3FrameFromFrame(self, const_cast<Frame&>(*it)));
  }
}
#endif

//! @endcond

/**
 * Set the fields in a TagLib ID3v2 frame.
 *
 * @param self   this TagLibFile instance
 * @param tFrame TagLib frame to set
 * @param frame  frame with field values
 */
template <class T>
void setTagLibFrame(const TagLibFile* self, T* tFrame, const Frame& frame)
{
  const Frame::FieldList& fieldList = frame.getFieldList();
  // If value is changed or field list is empty,
  // set from value, else from FieldList.
  if (frame.isValueChanged() || fieldList.empty()) {
    QString text(frame.getValue());
    if (frame.getType() == Frame::FT_Genre) {
      if (!TagConfig::instance().genreNotNumeric()) {
        text = Genres::getNumberString(text, false);
      }
    } else if (frame.getType() == Frame::FT_Track) {
      self->formatTrackNumberIfEnabled(text, true);
    }
    setValue(tFrame, toTString(text));
    setTextEncoding(tFrame, getTextEncodingConfig(needsUnicode(text)));
  } else {
    for (Frame::FieldList::const_iterator fldIt = fieldList.begin();
         fldIt != fieldList.end();
         ++fldIt) {
      const Frame::Field& fld = *fldIt;
      switch (fld.m_id) {
        case Frame::ID_Text:
        {
          QString value(fld.m_value.toString());
          if (frame.getType() == Frame::FT_Genre) {
            if (!TagConfig::instance().genreNotNumeric()) {
              value = Genres::getNumberString(value, false);
            }
          } else if (frame.getType() == Frame::FT_Track) {
            self->formatTrackNumberIfEnabled(value, true);
          }
          setText(tFrame, toTString(value));
          break;
        }
        case Frame::ID_TextEnc:
          setTextEncoding(tFrame, static_cast<TagLib::String::Type>(
                            fld.m_value.toInt()));
          break;
        case Frame::ID_Description:
          setDescription(tFrame, fld);
          break;
        case Frame::ID_MimeType:
          setMimeType(tFrame, fld);
          break;
        case Frame::ID_PictureType:
          setPictureType(tFrame, fld);
          break;
        case Frame::ID_Data:
          setData(tFrame, fld);
          break;
        case Frame::ID_Language:
          setLanguage(tFrame, fld);
          break;
        case Frame::ID_Owner:
          setOwner(tFrame, fld);
          break;
        case Frame::ID_Id:
          setIdentifier(tFrame, fld);
          break;
        case Frame::ID_Filename:
          setFilename(tFrame, fld);
          break;
        case Frame::ID_Url:
          setUrl(tFrame, fld);
          break;
#if TAGLIB_VERSION >= 0x010600
        case Frame::ID_Email:
          setEmail(tFrame, fld);
          break;
        case Frame::ID_Rating:
          setRating(tFrame, fld);
          break;
        case Frame::ID_Counter:
          setCounter(tFrame, fld);
          break;
#endif
#if TAGLIB_VERSION >= 0x010800
        case Frame::ID_Price:
          setPrice(tFrame, fld);
          break;
        case Frame::ID_Date:
          setDate(tFrame, fld);
          break;
        case Frame::ID_Seller:
          setSeller(tFrame, fld);
          break;
#endif
        case Frame::ID_TimestampFormat:
          setTimestampFormat(tFrame, fld);
          break;
        case Frame::ID_ContentType:
          setContentType(tFrame, fld);
          break;
#if TAGLIB_VERSION >= 0x010a00
        case Frame::ID_Subframe:
          setSubframes(self, tFrame, fldIt, fieldList.end());
          return;
#endif
      }
    }
  }
}

/**
 * Modify an ID3v2 frame.
 *
 * @param self     this TagLibFile instance
 * @param id3Frame original ID3v2 frame
 * @param frame    frame with fields to set in new frame
 */
static void setId3v2Frame(const TagLibFile* self,
  TagLib::ID3v2::Frame* id3Frame, const Frame& frame)
{
  if (id3Frame) {
    TagLib::ID3v2::TextIdentificationFrame* tFrame;
    TagLib::ID3v2::AttachedPictureFrame* apicFrame;
    TagLib::ID3v2::CommentsFrame* commFrame;
    TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame;
    TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
    TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
    TagLib::ID3v2::UrlLinkFrame* wFrame;
    TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
    TagLib::ID3v2::SynchronizedLyricsFrame* syltFrame;
    TagLib::ID3v2::EventTimingCodesFrame* etcoFrame;
#if TAGLIB_VERSION >= 0x010600
    TagLib::ID3v2::PrivateFrame* privFrame;
    TagLib::ID3v2::PopularimeterFrame* popmFrame;
#endif
#if TAGLIB_VERSION >= 0x010800
    TagLib::ID3v2::OwnershipFrame* owneFrame;
#endif
#if TAGLIB_VERSION >= 0x010500
    TagLib::ID3v2::RelativeVolumeFrame* rva2Frame;
#endif
#if TAGLIB_VERSION >= 0x010a00
    TagLib::ID3v2::ChapterFrame* chapFrame;
    TagLib::ID3v2::TableOfContentsFrame* ctocFrame;
#endif
    if ((tFrame =
         dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3Frame))
        != 0) {
      TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame =
        dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(id3Frame);
      if (txxxFrame) {
        setTagLibFrame(self, txxxFrame, frame);
      } else {
        setTagLibFrame(self, tFrame, frame);
      }
    } else if ((apicFrame =
                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame))
               != 0) {
      setTagLibFrame(self, apicFrame, frame);
    } else if ((commFrame = dynamic_cast<TagLib::ID3v2::CommentsFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, commFrame, frame);
    } else if ((ufidFrame =
                dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, ufidFrame, frame);
    } else if ((geobFrame =
                dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, geobFrame, frame);
    } else if ((wxxxFrame = dynamic_cast<TagLib::ID3v2::UserUrlLinkFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, wxxxFrame, frame);
    } else if ((wFrame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, wFrame, frame);
    } else if ((usltFrame =
                dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, usltFrame, frame);
    } else if ((syltFrame =
                dynamic_cast<TagLib::ID3v2::SynchronizedLyricsFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, syltFrame, frame);
    } else if ((etcoFrame =
                dynamic_cast<TagLib::ID3v2::EventTimingCodesFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, etcoFrame, frame);
#if TAGLIB_VERSION >= 0x010600
    } else if ((privFrame = dynamic_cast<TagLib::ID3v2::PrivateFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, privFrame, frame);
    } else if ((popmFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, popmFrame, frame);
#endif
#if TAGLIB_VERSION >= 0x010800
    } else if ((owneFrame = dynamic_cast<TagLib::ID3v2::OwnershipFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, owneFrame, frame);
#endif
#if TAGLIB_VERSION >= 0x010500
    } else if ((rva2Frame = dynamic_cast<TagLib::ID3v2::RelativeVolumeFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, rva2Frame, frame);
#endif
#if TAGLIB_VERSION >= 0x010a00
    } else if ((chapFrame = dynamic_cast<TagLib::ID3v2::ChapterFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, chapFrame, frame);
    } else if ((ctocFrame = dynamic_cast<TagLib::ID3v2::TableOfContentsFrame*>(
                  id3Frame)) != 0) {
      setTagLibFrame(self, ctocFrame, frame);
#endif
    } else {
      TagLib::ByteVector id(id3Frame->frameID());
      // create temporary objects for frames not known by TagLib,
      // an UnknownFrame copy will be created by the edit method.
#if TAGLIB_VERSION < 0x010500
      if (id.startsWith("WXXX")) {
        TagLib::ID3v2::UserUrlLinkFrame userUrlLinkFrame(id3Frame->render());
        setTagLibFrame(self, &userUrlLinkFrame, frame);
        id3Frame->setData(userUrlLinkFrame.render());
      } else if (id.startsWith("W")) {
        TagLib::ID3v2::UrlLinkFrame urlLinkFrame(id3Frame->render());
        setTagLibFrame(self, &urlLinkFrame, frame);
        id3Frame->setData(urlLinkFrame.render());
      } else if (id.startsWith("USLT")) {
        TagLib::ID3v2::UnsynchronizedLyricsFrame usltFrm(id3Frame->render());
        setTagLibFrame(self, &usltFrm, frame);
        id3Frame->setData(usltFrm.render());
      } else if (id.startsWith("GEOB")) {
        TagLib::ID3v2::GeneralEncapsulatedObjectFrame geobFrm(id3Frame->render());
        setTagLibFrame(self, &geobFrm, frame);
        id3Frame->setData(geobFrm.render());
      } else
#endif
#if TAGLIB_VERSION < 0x010a00
      if (id.startsWith("SYLT")) {
        TagLib::ID3v2::SynchronizedLyricsFrame syltFrm(id3Frame->render());
        setTagLibFrame(self, &syltFrm, frame);
        id3Frame->setData(syltFrm.render());
      } else if (id.startsWith("ETCO")) {
        TagLib::ID3v2::EventTimingCodesFrame etcoFrm(id3Frame->render());
        setTagLibFrame(self, &etcoFrm, frame);
        id3Frame->setData(etcoFrm.render());
      } else
#endif
      {
        setTagLibFrame(self, id3Frame, frame);
      }
    }
  }
}

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
static const char* getVorbisNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    "TITLE",           // FT_Title,
    "ARTIST",          // FT_Artist,
    "ALBUM",           // FT_Album,
    "COMMENT",         // FT_Comment,
    "DATE",            // FT_Date,
    "TRACKNUMBER",     // FT_Track,
    "GENRE",           // FT_Genre,
                       // FT_LastV1Frame = FT_Track,
    "ALBUMARTIST",     // FT_AlbumArtist,
    "ARRANGER",        // FT_Arranger,
    "AUTHOR",          // FT_Author,
    "BPM",             // FT_Bpm,
    "CATALOGNUMBER",   // FT_CatalogNumber,
    "COMPILATION",     // FT_Compilation,
    "COMPOSER",        // FT_Composer,
    "CONDUCTOR",       // FT_Conductor,
    "COPYRIGHT",       // FT_Copyright,
    "DISCNUMBER",      // FT_Disc,
    "ENCODED-BY",      // FT_EncodedBy,
    "ENCODERSETTINGS", // FT_EncoderSettings,
    "ENCODINGTIME",    // FT_EncodingTime,
    "GROUPING",        // FT_Grouping,
    "INITIALKEY",      // FT_InitialKey,
    "ISRC",            // FT_Isrc,
    "LANGUAGE",        // FT_Language,
    "LYRICIST",        // FT_Lyricist,
    "LYRICS",          // FT_Lyrics,
    "SOURCEMEDIA",     // FT_Media,
    "MOOD",            // FT_Mood,
    "ORIGINALALBUM",   // FT_OriginalAlbum,
    "ORIGINALARTIST",  // FT_OriginalArtist,
    "ORIGINALDATE",    // FT_OriginalDate,
    "PART",            // FT_Part,
    "PERFORMER",       // FT_Performer,
    "METADATA_BLOCK_PICTURE", // FT_Picture,
    "PUBLISHER",       // FT_Publisher,
    "RELEASECOUNTRY",  // FT_ReleaseCountry,
    "REMIXER",         // FT_Remixer,
    "ALBUMSORT",       // FT_SortAlbum,
    "ALBUMARTISTSORT", // FT_SortAlbumArtist,
    "ARTISTSORT",      // FT_SortArtist,
    "COMPOSERSORT",    // FT_SortComposer,
    "TITLESORT",       // FT_SortName,
    "SUBTITLE",        // FT_Subtitle,
    "WEBSITE",         // FT_Website,
    "WWWAUDIOFILE",    // FT_WWWAudioFile,
    "WWWAUDIOSOURCE",  // FT_WWWAudioSource,
    "RELEASEDATE",     // FT_ReleaseDate,
    "RATING"           // FT_Rating,
                       // FT_LastFrame = FT_Rating
  };
  struct not_used { int array_size_check[
      sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
      ? 1 : -1 ]; };
  if (type == Frame::FT_Picture &&
      TagConfig::instance().pictureNameIndex() == TagConfig::VP_COVERART) {
    return "COVERART";
  }
  return type <= Frame::FT_LastFrame ? names[type] : "UNKNOWN";
}

/**
 * Get the frame type for a Vorbis name.
 *
 * @param name Vorbis tag name
 *
 * @return frame type.
 */
static Frame::Type getTypeFromVorbisName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      Frame::Type type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getVorbisNameFromType(type)), type);
    }
    strNumMap.insert(QLatin1String("DESCRIPTION"), Frame::FT_Comment);
    strNumMap.insert(QLatin1String("COVERART"), Frame::FT_Picture);
    strNumMap.insert(QLatin1String("METADATA_BLOCK_PICTURE"), Frame::FT_Picture);
  }
  QMap<QString, int>::const_iterator it =
    strNumMap.find(name.remove(QLatin1Char('=')).toUpper());
  if (it != strNumMap.end()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

/**
 * Get the frame type for an APE name.
 *
 * @param name APE tag name
 *
 * @return frame type.
 */
static Frame::Type getTypeFromApeName(const QString& name)
{
  Frame::Type type = getTypeFromVorbisName(name);
  if (type == Frame::FT_Other) {
    if (name == QLatin1String("YEAR")) {
      type = Frame::FT_Date;
    } else if (name == QLatin1String("TRACK")) {
      type = Frame::FT_Track;
    } else if (name == QLatin1String("ENCODED BY")) {
      type = Frame::FT_EncodedBy;
#if TAGLIB_VERSION >= 0x010800
    } else if (name.startsWith(QLatin1String("COVER ART"))) {
      type = Frame::FT_Picture;
#endif
    }
  }
  return type;
}

/**
 * Get internal name of a Vorbis frame.
 *
 * @param frame frame
 *
 * @return Vorbis key.
 */
QString TagLibFile::getVorbisName(const Frame& frame) const
{
  Frame::Type type = frame.getType();
  if (type == Frame::FT_Comment) {
    return getCommentFieldName();
  } else if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  } else {
    return frame.getName().remove(QLatin1Char('=')).toUpper();
  }
}

#if TAGLIB_VERSION >= 0x010800
/**
 * Get internal name of an APE picture frame.
 *
 * @param pictureType picture type
 *
 * @return APE key.
 */
static TagLib::String getApePictureName(PictureFrame::PictureType pictureType)
{
  TagLib::String name("COVER ART (");
  name += TagLib::String(PictureFrame::getPictureTypeString(pictureType)).
      upper();
  name += ')';
  return name;
}
#endif

/**
 * Get internal name of an APE frame.
 *
 * @param frame frame
 *
 * @return APE key.
 */
static QString getApeName(const Frame& frame)
{
  Frame::Type type = frame.getType();
  if (type == Frame::FT_Date) {
    return QLatin1String("YEAR");
  } else if (type == Frame::FT_Track) {
    return QLatin1String("TRACK");
#if TAGLIB_VERSION >= 0x010800
  } else if (type == Frame::FT_Picture) {
    PictureFrame::PictureType pictureType;
    if (!PictureFrame::getPictureType(frame, pictureType)) {
      pictureType = Frame::PT_CoverFront;
    }
    return toQString(getApePictureName(pictureType));
#endif
  } else if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  } else {
    return frame.getName().toUpper();
  }
}

#ifdef TAGLIB_WITH_MP4
/** Type of data in MP4 frame. */
enum Mp4ValueType {
  MVT_ByteArray,
#if TAGLIB_VERSION >= 0x010602
  MVT_CoverArt,
#endif
  MVT_String,
  MVT_Bool,
  MVT_Int,
  MVT_IntPair,
  MVT_Byte,
  MVT_UInt,
  MVT_LongLong
};

/** MP4 name, frame type and value type. */
struct Mp4NameTypeValue {
  const char* name;
  Frame::Type type;
  Mp4ValueType value;
};

/** Mapping between frame types and field names. */
static const Mp4NameTypeValue mp4NameTypeValues[] = {
  { "\251nam", Frame::FT_Title, MVT_String },
  { "\251ART", Frame::FT_Artist, MVT_String },
  { "\251wrt", Frame::FT_Composer, MVT_String },
  { "\251alb", Frame::FT_Album, MVT_String },
  { "\251day", Frame::FT_Date, MVT_String },
  { "\251enc", Frame::FT_EncodedBy, MVT_String },
  { "\251cmt", Frame::FT_Comment, MVT_String },
  { "gnre", Frame::FT_Genre, MVT_String },
  // (c)gen is after gnre so that it is used in the maps because TagLib uses it
  { "\251gen", Frame::FT_Genre, MVT_String },
  { "trkn", Frame::FT_Track, MVT_IntPair },
  { "disk", Frame::FT_Disc, MVT_IntPair },
  { "cpil", Frame::FT_Compilation, MVT_Bool },
  { "tmpo", Frame::FT_Bpm, MVT_Int },
  { "\251grp", Frame::FT_Grouping, MVT_String },
  { "aART", Frame::FT_AlbumArtist, MVT_String },
  { "pgap", Frame::FT_Other, MVT_Bool },
  { "cprt", Frame::FT_Copyright, MVT_String },
  { "\251lyr", Frame::FT_Lyrics, MVT_String },
  { "tvsh", Frame::FT_Other, MVT_String },
  { "tvnn", Frame::FT_Other, MVT_String },
  { "tven", Frame::FT_Other, MVT_String },
  { "tvsn", Frame::FT_Other, MVT_UInt },
  { "tves", Frame::FT_Other, MVT_UInt },
  { "desc", Frame::FT_Other, MVT_String },
  { "ldes", Frame::FT_Other, MVT_String },
  { "sonm", Frame::FT_SortName, MVT_String },
  { "soar", Frame::FT_SortArtist, MVT_String },
  { "soaa", Frame::FT_SortAlbumArtist, MVT_String },
  { "soal", Frame::FT_SortAlbum, MVT_String },
  { "soco", Frame::FT_SortComposer, MVT_String },
  { "sosn", Frame::FT_Other, MVT_String },
  { "\251too", Frame::FT_EncoderSettings, MVT_String },
  { "purd", Frame::FT_Other, MVT_String },
  { "pcst", Frame::FT_Other, MVT_Bool },
  { "keyw", Frame::FT_Other, MVT_String },
  { "catg", Frame::FT_Other, MVT_String },
  { "hdvd", Frame::FT_Other, MVT_Bool },
  { "stik", Frame::FT_Other, MVT_Byte },
  { "rtng", Frame::FT_Other, MVT_Byte },
  { "apID", Frame::FT_Other, MVT_String },
  { "akID", Frame::FT_Other, MVT_Byte },
  { "sfID", Frame::FT_Other, MVT_UInt },
  { "cnID", Frame::FT_Other, MVT_UInt },
  { "atID", Frame::FT_Other, MVT_UInt },
  { "plID", Frame::FT_Other, MVT_LongLong },
  { "geID", Frame::FT_Other, MVT_UInt },
  { "ownr", Frame::FT_Other, MVT_String },
  { "xid ", Frame::FT_Other, MVT_String },
#if TAGLIB_VERSION >= 0x010602
  { "covr", Frame::FT_Picture, MVT_CoverArt },
#else
  { "covr", Frame::FT_Picture, MVT_ByteArray },
#endif
#ifdef TAGLIB_WITH_MP4_SHWM
  { "\251wrk", Frame::FT_Other, MVT_String },
  { "\251mvn", Frame::FT_Other, MVT_String },
  { "\251mvi", Frame::FT_Other, MVT_Int },
  { "\251mvc", Frame::FT_Other, MVT_Int },
  { "shwm", Frame::FT_Other, MVT_Bool },
#endif
  { "ARRANGER", Frame::FT_Arranger, MVT_String },
  { "AUTHOR", Frame::FT_Author, MVT_String },
  { "CATALOGNUMBER", Frame::FT_CatalogNumber, MVT_String },
  { "CONDUCTOR", Frame::FT_Conductor, MVT_String },
  { "ENCODINGTIME", Frame::FT_EncodingTime, MVT_String },
  { "INITIALKEY", Frame::FT_InitialKey, MVT_String },
  { "ISRC", Frame::FT_Isrc, MVT_String },
  { "LANGUAGE", Frame::FT_Language, MVT_String },
  { "LYRICIST", Frame::FT_Lyricist, MVT_String },
  { "MOOD", Frame::FT_Mood, MVT_String },
  { "SOURCEMEDIA", Frame::FT_Media, MVT_String },
  { "ORIGINALALBUM", Frame::FT_OriginalAlbum, MVT_String },
  { "ORIGINALARTIST", Frame::FT_OriginalArtist, MVT_String },
  { "ORIGINALDATE", Frame::FT_OriginalDate, MVT_String },
  { "PART", Frame::FT_Part, MVT_String },
  { "PERFORMER", Frame::FT_Performer, MVT_String },
  { "PUBLISHER", Frame::FT_Publisher, MVT_String },
  { "RELEASECOUNTRY", Frame::FT_ReleaseCountry, MVT_String },
  { "REMIXER", Frame::FT_Remixer, MVT_String },
  { "SUBTITLE", Frame::FT_Subtitle, MVT_String },
  { "WEBSITE", Frame::FT_Website, MVT_String },
  { "WWWAUDIOFILE", Frame::FT_WWWAudioFile, MVT_String },
  { "WWWAUDIOSOURCE", Frame::FT_WWWAudioSource, MVT_String },
  { "RELEASEDATE", Frame::FT_ReleaseDate, MVT_String },
  { "RATING", Frame::FT_Rating, MVT_String }
};

/**
 * Get MP4 name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
static void getMp4NameForType(Frame::Type type, TagLib::String& name,
                              Mp4ValueType& value)
{
  static QMap<Frame::Type, unsigned> typeNameMap;
  if (typeNameMap.empty()) {
    // first time initialization
    for (unsigned i = 0;
         i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
      if (mp4NameTypeValues[i].type != Frame::FT_Other) {
        typeNameMap.insert(mp4NameTypeValues[i].type, i);
      }
    }
  }
  name = "";
  value = MVT_String;
  if (type != Frame::FT_Other) {
    QMap<Frame::Type, unsigned>::const_iterator it = typeNameMap.find(type);
    if (it != typeNameMap.end()) {
      name = mp4NameTypeValues[*it].name;
      value = mp4NameTypeValues[*it].value;
    }
  }
}

/**
 * Get MP4 value type and frame type for an MP4 name.
 *
 * @param name  MP4 name
 * @param type  the frame type is returned here
 * @param value the MP4 value type is returned here
 *
 * @return true if free-form frame.
 */
static bool getMp4TypeForName(const TagLib::String& name, Frame::Type& type,
                              Mp4ValueType& value)
{
  static QMap<TagLib::String, unsigned> nameTypeMap;
  if (nameTypeMap.empty()) {
    // first time initialization
    for (unsigned i = 0;
         i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
      nameTypeMap.insert(mp4NameTypeValues[i].name, i);
    }
  }
  QMap<TagLib::String, unsigned>::const_iterator it = nameTypeMap.find(name);
  if (it != nameTypeMap.end()) {
    type = mp4NameTypeValues[*it].type;
    value = mp4NameTypeValues[*it].value;
    return name[0] >= 'A' && name[0] <= 'Z';
  } else {
    type = Frame::FT_Other;
    value = MVT_String;
    return true;
  }
}

/**
 * Strip free form prefix from MP4 frame name.
 *
 * @param name MP4 frame name to be stripped
 */
static void stripMp4FreeFormName(TagLib::String& name)
{
  if (name.startsWith("----")) {
    int nameStart = name.rfind(":");
    if (nameStart == -1) {
      nameStart = 5;
    } else {
      ++nameStart;
    }
    name = name.substr(nameStart);

    Frame::Type type;
    Mp4ValueType valueType;
    if (!getMp4TypeForName(name, type, valueType)) {
      // not detected as free form => mark with ':' as first character
      name = ':' + name;
    }
  }
}

/**
 * Prepend free form prefix to MP4 frame name.
 * Only names starting with a capital letter or ':' are prefixed.
 *
 * @param name MP4 frame name to be prefixed
 * @param mp4Tag tag to check for existing item
 */
static void prefixMp4FreeFormName(TagLib::String& name,
                                  const TagLib::MP4::Tag* mp4Tag)
{
  if (!mp4Tag->contains(name) && !name.startsWith("----")) {
    Frame::Type type;
    Mp4ValueType valueType;
    if (getMp4TypeForName(name, type, valueType)) {
      // free form
      if (name[0] == ':') name = name.substr(1);
      name = "----:com.apple.iTunes:" + name;
    }
  }
}

/**
 * Get an MP4 type for a frame.
 *
 * @param frame frame
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
static void getMp4TypeForFrame(const Frame& frame, TagLib::String& name,
                               Mp4ValueType& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getMp4NameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(frame.getInternalName());
    getMp4TypeForName(name, type, value);
  }
}

/**
 * Get an MP4 item for a frame.
 *
 * @param frame frame
 * @param name  the name for the item is returned here
 *
 * @return MP4 item, an invalid item is returned if not supported.
 */
static TagLib::MP4::Item getMp4ItemForFrame(const Frame& frame, TagLib::String& name)
{
  Mp4ValueType valueType;
  getMp4TypeForFrame(frame, name, valueType);
  switch (valueType) {
    case MVT_String:
      return TagLib::MP4::Item(toTString(frame.getValue()));
    case MVT_Bool:
      return TagLib::MP4::Item(frame.getValue().toInt() != 0);
    case MVT_Int:
      return TagLib::MP4::Item(frame.getValue().toInt());
    case MVT_IntPair:
    {
      QString str1 = frame.getValue(), str2 = QLatin1String("0");
      int slashPos = str1.indexOf(QLatin1Char('/'));
      if (slashPos != -1) {
        str2 = str1.mid(slashPos + 1);
        str1.truncate(slashPos);
      }
      return TagLib::MP4::Item(str1.toInt(), str2.toInt());
    }
#if TAGLIB_VERSION >= 0x010602
    case MVT_CoverArt:
    {
      QByteArray ba;
      TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
      if (PictureFrame::getData(frame, ba)) {
        QString mimeType;
        if (PictureFrame::getMimeType(frame, mimeType) &&
            mimeType == QLatin1String("image/png")) {
          format = TagLib::MP4::CoverArt::PNG;
        }
      }
      TagLib::MP4::CoverArt coverArt(format,
                                     TagLib::ByteVector(ba.data(), ba.size()));
      TagLib::MP4::CoverArtList coverArtList;
      coverArtList.append(coverArt);
      return TagLib::MP4::Item(coverArtList);
    }
#endif
#if TAGLIB_VERSION >= 0x010800
    case MVT_Byte:
      return TagLib::MP4::Item(static_cast<uchar>(frame.getValue().toInt()));
    case MVT_UInt:
      return TagLib::MP4::Item(frame.getValue().toUInt());
    case MVT_LongLong:
      return TagLib::MP4::Item(frame.getValue().toLongLong());
#endif
    case MVT_ByteArray:
    default:
      // binary data and album art are not handled by TagLib
      return TagLib::MP4::Item();
  }
}

#if TAGLIB_VERSION >= 0x010600
/**
 * Set a frame in an MP4 tag.
 * @param frame frame to set
 * @param mp4Tag MP4 tag
 */
void TagLibFile::setMp4Frame(const Frame& frame, TagLib::MP4::Tag* mp4Tag)
{
  TagLib::String name;
  TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
  if (item.isValid()) {
    int numTracks;
    if (name == "trkn" &&
        (numTracks = getTotalNumberOfTracksIfEnabled()) > 0) {
      TagLib::MP4::Item::IntPair pair = item.toIntPair();
      if (pair.second == 0) {
        item = TagLib::MP4::Item(pair.first, numTracks);
      }
    }
    prefixMp4FreeFormName(name, mp4Tag);
    mp4Tag->itemListMap()[name] = item;
    markTagChanged(Frame::Tag_2, frame.getType());
  }
}
#endif
#endif

#ifdef TAGLIB_WITH_ASF
/** Indices of fixed ASF frames. */
enum AsfFrameIndex {
  AFI_Title,
  AFI_Artist,
  AFI_Comment,
  AFI_Copyright,
  AFI_Rating,
  AFI_Attributes
};

/** ASF name, frame type and value type. */
struct AsfNameTypeValue {
  const char* name;
  Frame::Type type;
  TagLib::ASF::Attribute::AttributeTypes value;
};

/** Mapping between frame types and field names. */
static const AsfNameTypeValue asfNameTypeValues[] = {
  { "Title", Frame::FT_Title, TagLib::ASF::Attribute::UnicodeType },
  { "Author", Frame::FT_Artist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumTitle", Frame::FT_Album, TagLib::ASF::Attribute::UnicodeType },
  { "Description", Frame::FT_Comment, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Year", Frame::FT_Date, TagLib::ASF::Attribute::UnicodeType },
  { "Copyright", Frame::FT_Copyright, TagLib::ASF::Attribute::UnicodeType },
  { "Rating Information", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/TrackNumber", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Track", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Genre", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
  { "WM/GenreID", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumArtist", Frame::FT_AlbumArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumSortOrder", Frame::FT_SortAlbum, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ArtistSortOrder", Frame::FT_SortArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/TitleSortOrder", Frame::FT_SortName, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Producer", Frame::FT_Arranger, TagLib::ASF::Attribute::UnicodeType },
  { "WM/BeatsPerMinute", Frame::FT_Bpm, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Composer", Frame::FT_Composer, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Conductor", Frame::FT_Conductor, TagLib::ASF::Attribute::UnicodeType },
  { "WM/PartOfSet", Frame::FT_Disc, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodedBy", Frame::FT_EncodedBy, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ContentGroupDescription", Frame::FT_Grouping, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ISRC", Frame::FT_Isrc, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Language", Frame::FT_Language, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Writer", Frame::FT_Lyricist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Lyrics", Frame::FT_Lyrics, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AudioSourceURL", Frame::FT_WWWAudioSource, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalAlbumTitle", Frame::FT_OriginalAlbum, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalArtist", Frame::FT_OriginalArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalReleaseYear", Frame::FT_OriginalDate, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SubTitleDescription", Frame::FT_Part, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Picture", Frame::FT_Picture, TagLib::ASF::Attribute::BytesType },
  { "WM/Publisher", Frame::FT_Publisher, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ModifiedBy", Frame::FT_Remixer, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SubTitle", Frame::FT_Subtitle, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AuthorURL", Frame::FT_Website, TagLib::ASF::Attribute::UnicodeType },
  { "AverageLevel", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
  { "PeakValue", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
  { "WM/AudioFileURL", Frame::FT_WWWAudioFile, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodingSettings", Frame::FT_EncoderSettings, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodingTime", Frame::FT_EncodingTime, TagLib::ASF::Attribute::BytesType },
  { "WM/InitialKey", Frame::FT_InitialKey, TagLib::ASF::Attribute::UnicodeType },
  // incorrect WM/Lyrics_Synchronised data make file inaccessible in Windows
  // { "WM/Lyrics_Synchronised", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
  { "WM/MCDI", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
  { "WM/MediaClassPrimaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/MediaClassSecondaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/Mood", Frame::FT_Mood, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalFilename", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalLyricist", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/PromotionURL", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SharedUserRating", Frame::FT_Rating, TagLib::ASF::Attribute::UnicodeType },
  { "WM/WMCollectionGroupID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/WMCollectionID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/WMContentID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType }
};

/**
 * Get ASF name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the ASF name is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfNameForType(Frame::Type type, TagLib::String& name,
                              TagLib::ASF::Attribute::AttributeTypes& value)
{
  static QMap<Frame::Type, unsigned> typeNameMap;
  if (typeNameMap.empty()) {
    // first time initialization
    for (unsigned i = 0;
         i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
      if (asfNameTypeValues[i].type != Frame::FT_Other &&
          !typeNameMap.contains(asfNameTypeValues[i].type)) {
        typeNameMap.insert(asfNameTypeValues[i].type, i);
      }
    }
  }
  name = "";
  value = TagLib::ASF::Attribute::UnicodeType;
  if (type != Frame::FT_Other) {
    QMap<Frame::Type, unsigned>::const_iterator it = typeNameMap.find(type);
    if (it != typeNameMap.end()) {
      name = asfNameTypeValues[*it].name;
      value = asfNameTypeValues[*it].value;
    }
  }
}

/**
 * Get ASF value type and frame type for an ASF name.
 *
 * @param name  ASF name
 * @param type  the frame type is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfTypeForName(const TagLib::String& name, Frame::Type& type,
                              TagLib::ASF::Attribute::AttributeTypes& value)
{
  static QMap<TagLib::String, unsigned> nameTypeMap;
  if (nameTypeMap.empty()) {
    // first time initialization
    for (unsigned i = 0;
         i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
      nameTypeMap.insert(asfNameTypeValues[i].name, i);
    }
  }
  QMap<TagLib::String, unsigned>::const_iterator it = nameTypeMap.find(name);
  if (it != nameTypeMap.end()) {
    type = asfNameTypeValues[*it].type;
    value = asfNameTypeValues[*it].value;
  } else {
    type = Frame::FT_Other;
    value = TagLib::ASF::Attribute::UnicodeType;
  }
}

/**
 * Get an ASF type for a frame.
 *
 * @param frame frame
 * @param name  the name for the attribute is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfTypeForFrame(const Frame& frame, TagLib::String& name,
                               TagLib::ASF::Attribute::AttributeTypes& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getAsfNameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(frame.getInternalName());
    getAsfTypeForName(name, type, value);
  }
}

#if TAGLIB_VERSION >= 0x010700
/**
 * Get a picture frame from a WM/Picture.
 *
 * @param picture ASF picture
 * @param frame   the picture frame is returned here
 *
 * @return true if ok.
 */
static bool parseAsfPicture(const TagLib::ASF::Picture& picture, Frame& frame)
{
  if (!picture.isValid())
    return false;

  TagLib::ByteVector data = picture.picture();
  QString description(toQString(picture.description()));
  PictureFrame::setFields(frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
                          toQString(picture.mimeType()),
                          static_cast<PictureFrame::PictureType>(picture.type()),
                          description,
                          QByteArray(data.data(), data.size()));
  frame.setType(Frame::FT_Picture);
  return true;
}

/**
 * Render the bytes of a WM/Picture from a picture frame.
 *
 * @param frame   picture frame
 * @param picture the ASF picture is returned here
 */
static void renderAsfPicture(const Frame& frame, TagLib::ASF::Picture& picture)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray data;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, data);

  if (frame.isValueChanged()) {
    description = frame.getValue();
  }
  picture.setMimeType(toTString(mimeType));
  picture.setType(static_cast<TagLib::ASF::Picture::Type>(pictureType));
  picture.setDescription(toTString(description));
  picture.setPicture(TagLib::ByteVector(data.data(), data.size()));
}
#elif TAGLIB_VERSION >= 0x010602
/**
 * Get a picture frame from the bytes in a WM/Picture frame.
 * The WM/Picture frame has the following data:
 * 1 byte picture type, 4 bytes (little endian) size of picture data,
 * UTF16_LE mime type, UTF16_LE description, picture data.
 *
 * @param data bytes in WM/Picture frame
 * @param frame the picture frame is returned here
 *
 * @return true if ok.
 */
static bool parseAsfPicture(const TagLib::ByteVector& data, Frame& frame)
{
  uint len = data.size();
  if (len < 10)
    return false;
  char pictureType = data[0];
  if (pictureType < 0 || pictureType > 20)
    return false;
  uint picSize =
     ((unsigned char)data[1] & 0xff)        |
    (((unsigned char)data[2] & 0xff) << 8)  |
    (((unsigned char)data[3] & 0xff) << 16) |
    (((unsigned char)data[4] & 0xff) << 24);
  if (picSize > len - 9)
    return false;
  uint offset = 5;
  QString mimeType = QString::fromUtf16(
    reinterpret_cast<const ushort*>(data.mid(offset, len - offset).data()));
  offset += mimeType.length() * 2 + 2;
  if (offset >= len - 1)
    return false;
  QString description = QString::fromUtf16(
    reinterpret_cast<const ushort*>(data.mid(offset, len - offset).data()));
  offset += description.length() * 2 + 2;
  if (offset > len)
    return false;
  TagLib::ByteVector picture = data.mid(offset, len - offset);
  PictureFrame::setFields(frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
                          mimeType,
                          static_cast<PictureFrame::PictureType>(pictureType),
                          description,
                          QByteArray(picture.data(), picture.size()));
  frame.setType(Frame::FT_Picture);
  return true;
}

/**
 * Render the bytes of a WM/Picture frame from a picture frame.
 *
 * @param frame picture frame
 * @param data  the bytes for the WM/Picture are returned here
 */
static void renderAsfPicture(const Frame& frame, TagLib::ByteVector& data)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray picture;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, picture);
  if (frame.isValueChanged()) {
    description = frame.getValue();
  }

  data.resize(5);
  data[0] = pictureType;
  uint size = picture.size();
  data[1] = size & 0xff;
  size >>= 8;
  data[2] = size & 0xff;
  size >>= 8;
  data[3] = size & 0xff;
  size >>= 8;
  data[4] = size & 0xff;
  data.append(toTString(mimeType).data(TagLib::String::UTF16LE));
  data.append(TagLib::ByteVector(2, 0));
  data.append(toTString(description).data(TagLib::String::UTF16LE));
  data.append(TagLib::ByteVector(2, 0));
  data.append(TagLib::ByteVector(picture.data(), picture.size()));
}
#endif

/**
 * Get an ASF attribute for a frame.
 *
 * @param frame     frame
 * @param valueType ASF value type
 *
 * @return ASF attribute, an empty attribute is returned if not supported.
 */
static TagLib::ASF::Attribute getAsfAttributeForFrame(
  const Frame& frame,
  TagLib::ASF::Attribute::AttributeTypes valueType)
{
  switch (valueType) {
    case TagLib::ASF::Attribute::UnicodeType:
      return TagLib::ASF::Attribute(toTString(frame.getValue()));
    case TagLib::ASF::Attribute::BoolType:
      return TagLib::ASF::Attribute(frame.getValue() == QLatin1String("1"));
    case TagLib::ASF::Attribute::WordType:
      return TagLib::ASF::Attribute(frame.getValue().toUShort());
    case TagLib::ASF::Attribute::DWordType:
      return TagLib::ASF::Attribute(frame.getValue().toUInt());
    case TagLib::ASF::Attribute::QWordType:
      return TagLib::ASF::Attribute(frame.getValue().toULongLong());
    case TagLib::ASF::Attribute::BytesType:
    case TagLib::ASF::Attribute::GuidType:
    default:
      if (frame.getType() != Frame::FT_Picture) {
        QByteArray ba;
        if (AttributeData(frame.getInternalName()).toByteArray(frame.getValue(), ba)) {
          return TagLib::ASF::Attribute(TagLib::ByteVector(ba.data(), ba.size()));
        }
        QVariant fieldValue = frame.getFieldValue(Frame::ID_Data);
        if (fieldValue.isValid()) {
          ba = fieldValue.toByteArray();
          return TagLib::ASF::Attribute(TagLib::ByteVector(ba.data(), ba.size()));
        }
      }
#if TAGLIB_VERSION >= 0x010700
      else {
        TagLib::ASF::Picture picture;
        renderAsfPicture(frame, picture);
        return TagLib::ASF::Attribute(picture);
      }
#elif TAGLIB_VERSION >= 0x010602
      else {
        TagLib::ByteVector bv;
        renderAsfPicture(frame, bv);
        return TagLib::ASF::Attribute(bv);
      }
#endif
  }
  return TagLib::ASF::Attribute();
}
#endif

#if TAGLIB_VERSION >= 0x010800
/**
 * Get a picture frame from the bytes in an APE cover art frame.
 * The cover art frame has the following data:
 * zero terminated description string (UTF-8), picture data.
 *
 * @param name key of APE item
 * @param data bytes in APE cover art frame
 * @param frame the picture frame is returned here
 */
static void parseApePicture(const QString& name,
                            const TagLib::ByteVector& data, Frame& frame)
{
  QByteArray picture;
  TagLib::String description;
  // Do not search for a description if the first byte could start JPG or PNG
  // data.
  int picPos = data.isEmpty() || data.at(0) == '\xff' || data.at(0) == '\x89'
      ? -1 : data.find('\0');
  if (picPos >= 0) {
    description = TagLib::String(data.mid(0, picPos), TagLib::String::UTF8);
    picture = QByteArray(data.data() + picPos + 1, data.size() - picPos - 1);
  } else {
    picture = QByteArray(data.data(), data.size());
  }
  Frame::PictureType pictureType = Frame::PT_CoverFront;
  if (name.startsWith(QLatin1String("COVER ART (")) &&
      name.endsWith(QLatin1Char(')'))) {
    QString typeStr = name.mid(11);
    typeStr.chop(1);
    pictureType = PictureFrame::getPictureTypeFromString(typeStr.toLatin1());
  }
  PictureFrame::setFields(
        frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
        QLatin1String("image/jpeg"), pictureType,
        toQString(description), picture);
}

/**
 * Render the bytes of an APE cover art frame from a picture frame.
 *
 * @param frame picture frame
 * @param data  the bytes for the APE cover art are returned here
 */
static void renderApePicture(const Frame& frame, TagLib::ByteVector& data)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray picture;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, picture);
  if (frame.isValueChanged()) {
    description = frame.getValue();
  }
  data.append(toTString(description).data(TagLib::String::UTF8));
  data.append('\0');
  data.append(TagLib::ByteVector(picture.constData(), picture.size()));
}
#endif

#if TAGLIB_VERSION >= 0x010a00
/**
 * Get name of INFO tag from type.
 *
 * @param type type
 *
 * @return name, NULL if not supported.
 */
static TagLib::ByteVector getInfoNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    "INAM", // FT_Title,
    "IART", // FT_Artist,
    "IPRD", // FT_Album,
    "ICMT", // FT_Comment,
    "ICRD", // FT_Date,
    "IPRT", // FT_Track
    "IGNR", // FT_Genre,
            // FT_LastV1Frame = FT_Track,
    0,      // FT_AlbumArtist,
    "IENG", // FT_Arranger,
    0,      // FT_Author,
    "IBPM", // FT_Bpm,
    0,      // FT_CatalogNumber,
    0,      // FT_Compilation,
    "IMUS", // FT_Composer,
    0,      // FT_Conductor,
    "ICOP", // FT_Copyright,
    0,      // FT_Disc,
    "ITCH", // FT_EncodedBy,
    "ISFT", // FT_EncoderSettings,
    "IDIT", // FT_EncodingTime,
    0,      // FT_Grouping,
    0,      // FT_InitialKey,
    "ISRC", // FT_Isrc,
    "ILNG", // FT_Language,
    "IWRI", // FT_Lyricist,
    0,      // FT_Lyrics,
    "IMED", // FT_Media,
    0,      // FT_Mood,
    0,      // FT_OriginalAlbum,
    0,      // FT_OriginalArtist,
    0,      // FT_OriginalDate,
    "PRT1", // FT_Part,
    "ISTR", // FT_Performer,
    0,      // FT_Picture,
    "IPUB", // FT_Publisher,
    "ICNT", // FT_ReleaseCountry,
    "IEDT", // FT_Remixer,
    0,      // FT_SortAlbum,
    0,      // FT_SortAlbumArtist,
    0,      // FT_SortArtist,
    0,      // FT_SortComposer,
    0,      // FT_SortName,
    0,      // FT_Subtitle,
    "IBSU", // FT_Website,
    0,      // FT_WWWAudioFile,
    0,      // FT_WWWAudioSource,
    0,      // FT_ReleaseDate,
    "IRTD"  // FT_Rating,
            // FT_LastFrame = FT_Rating
  };
  struct not_used { int array_size_check[
      sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
      ? 1 : -1 ]; };
  if (type == Frame::FT_Track) {
    QByteArray ba = TagConfig::instance().riffTrackName().toLatin1();
    return TagLib::ByteVector(ba.constData(), ba.size());
  }
  const char* name = type <= Frame::FT_LastFrame ? names[type] : 0;
  return name ? TagLib::ByteVector(name, 4) : TagLib::ByteVector();
}

/**
 * Get the frame type for an INFO name.
 *
 * @param id INFO tag name
 *
 * @return frame type.
 */
static Frame::Type getTypeFromInfoName(const TagLib::ByteVector& id)
{
  static QMap<TagLib::ByteVector, int> strNumMap;
  if (strNumMap.isEmpty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      Frame::Type type = static_cast<Frame::Type>(i);
      TagLib::ByteVector str = getInfoNameFromType(type);
      if (!str.isEmpty()) {
        strNumMap.insert(str, type);
      }
    }
    QStringList riffTrackNames = TagConfig::getRiffTrackNames();
    riffTrackNames.append(TagConfig::instance().riffTrackName());
    foreach (const QString& str, riffTrackNames) {
      QByteArray ba = str.toLatin1();
      strNumMap.insert(TagLib::ByteVector(ba.constData(), ba.size()),
                       Frame::FT_Track);
    }
  }
  QMap<TagLib::ByteVector, int>::const_iterator it = strNumMap.constFind(id);
  if (it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

/**
 * Get internal name of an INFO frame.
 *
 * @param frame frame
 *
 * @return INFO id, "IKEY" if not found.
 */
static TagLib::ByteVector getInfoName(const Frame& frame)
{
  TagLib::ByteVector str = getInfoNameFromType(frame.getType());
  if (!str.isEmpty()) {
    return str;
  }

  QString name = frame.getInternalName();
  if (name.length() >= 4) {
    QByteArray ba = name.left(4).toUpper().toLatin1();
    return TagLib::ByteVector(ba.constData(), 4);
  }

  return "IKEY";
}
#endif

/**
 * Get a specific frame from the tags.
 *
 * @param tagNr tag number
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TagLibFile::getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const
{
  if (tagNr >= NUM_TAGS)
    return false;

  makeFileOpen();
  TagLib::Tag* tag = m_tag[tagNr];
  TagLib::String tstr;
  if (tag) {
    switch (type) {
    case Frame::FT_Album:
      tstr = tag->album();
      break;
    case Frame::FT_Artist:
      tstr = tag->artist();
      break;
    case Frame::FT_Comment:
      tstr = tag->comment();
      if (tagNr == Frame::Tag_Id3v1 && !tstr.isNull()) {
        tstr = tstr.substr(0, 28);
      }
      break;
    case Frame::FT_Date:
    {
      uint nr = tag->year();
      tstr = nr != 0 ? TagLib::String::number(nr) : "";
      break;
    }
    case Frame::FT_Genre:
      tstr = tag->genre();
      break;
    case Frame::FT_Title:
      tstr = tag->title();
      break;
    case Frame::FT_Track:
    {
      uint nr = tag->track();
      tstr = nr != 0 ? TagLib::String::number(nr) : "";
      break;
    }
    default:
      // maybe handled in a subclass
      return false;
    }
    QString str = tagNr != Frame::Tag_Id3v1 && type == Frame::FT_Genre
        ? getGenreString(tstr)
        : tstr.isNull()
          ? QLatin1String("") : toQString(tstr);
    frame.setValue(str);
  } else {
    frame.setValue(QString());
  }
  frame.setType(type);
  return true;
}

/**
 * Set a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool TagLibFile::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  if (tagNr != Frame::Tag_Id3v1) {
    makeFileOpen();
    // If the frame has an index, change that specific frame
    int index = frame.getIndex();
    if (index != -1 && m_tag[tagNr]) {
      TagLib::ID3v2::Tag* id3v2Tag;
      TagLib::Ogg::XiphComment* oggTag;
      TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
      TagLib::ASF::Tag* asfTag;
#endif
#endif
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        if (index < static_cast<int>(frameList.size())) {
          // This is a hack. The frameList should not be modified directly.
          // However when removing the old frame and adding a new frame,
          // the indices of all frames get invalid.
          setId3v2Frame(this, frameList[index], frame);
          markTagChanged(tagNr, frame.getType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != 0) {
        QString frameValue(frame.getValue());
        if (frame.getType() == Frame::FT_Picture) {
#if TAGLIB_VERSION >= 0x010700
          if (m_pictures.isRead()) {
            int idx = frame.getIndex();
            if (idx >= 0 && idx < m_pictures.size()) {
              Frame newFrame(frame);
              PictureFrame::setDescription(newFrame, frameValue);
              if (PictureFrame::areFieldsEqual(m_pictures[idx], newFrame)) {
                m_pictures[idx].setValueChanged(false);
              } else {
                m_pictures[idx] = newFrame;
                markTagChanged(tagNr, Frame::FT_Picture);
              }
              return true;
            } else {
              return false;
            }
          } else {
            Frame newFrame(frame);
            PictureFrame::setDescription(newFrame, frameValue);
            PictureFrame::getFieldsToBase64(newFrame, frameValue);
            if (!frameValue.isEmpty() &&
                frame.getInternalName() == QLatin1String("COVERART")) {
              QString mimeType;
              PictureFrame::getMimeType(frame, mimeType);
              oggTag->addField("COVERARTMIME", toTString(mimeType), true);
            }
          }
#else
          return false;
#endif
        }
        TagLib::String key = toTString(getVorbisName(frame));
        TagLib::String value = toTString(frameValue);
#if TAGLIB_VERSION <= 0x010400
        // Remove all fields with that key, because TagLib <= 1.4 crashes
        // using an invalidated iterator after calling erase().
        oggTag->addField(key, value, true);
#else
        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        if (fieldListMap.contains(key) && fieldListMap[key].size() > 1) {
          int i = 0;
          TagLib::String oldValue(TagLib::String::null);
          for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
               it != fieldListMap.end();
               ++it) {
            TagLib::StringList stringList = (*it).second;
            for (TagLib::StringList::ConstIterator slit = stringList.begin();
                 slit != stringList.end();
                 ++slit) {
              if (i++ == index) {
                oldValue = *slit;
                break;
              }
            }
          }
          oggTag->removeField(key, oldValue);
          oggTag->addField(key, value, false);
        } else {
          oggTag->addField(key, value, true);
        }
#endif
        if (frame.getType() == Frame::FT_Track) {
          int numTracks = getTotalNumberOfTracksIfEnabled();
          if (numTracks > 0) {
            oggTag->addField("TRACKTOTAL", TagLib::String::number(numTracks), true);
          }
        }
        markTagChanged(tagNr, frame.getType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
#if TAGLIB_VERSION >= 0x010800
        if (frame.getType() == Frame::FT_Picture) {
          TagLib::ByteVector data;
          renderApePicture(frame, data);
          QString oldName = frame.getInternalName();
          QString newName = getApeName(frame);
          if (newName != oldName) {
            // If the picture type changes, the frame with the old name has to
            // be replaced with a frame with the new name.
            apeTag->removeItem(toTString(oldName));
          }
          apeTag->setData(toTString(newName), data);
        } else {
          apeTag->addValue(toTString(getApeName(frame)),
                           toTString(frame.getValue()));
        }
#else
        if (frame.getType() == Frame::FT_Picture) {
          return false;
        }
        apeTag->addValue(toTString(getApeName(frame)),
                         toTString(frame.getValue()));
#endif
        markTagChanged(tagNr, frame.getType());
        return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
        setMp4Frame(frame, mp4Tag);
        return true;
#endif
#ifdef TAGLIB_WITH_ASF
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
#if !(TAGLIB_VERSION >= 0x010602)
        if (frame.getType() == Frame::FT_Picture) {
          return false;
        }
#endif
        switch (index) {
          case AFI_Title:
            asfTag->setTitle(toTString(frame.getValue()));
            break;
          case AFI_Artist:
            asfTag->setArtist(toTString(frame.getValue()));
            break;
          case AFI_Comment:
            asfTag->setComment(toTString(frame.getValue()));
            break;
          case AFI_Copyright:
            asfTag->setCopyright(toTString(frame.getValue()));
            break;
          case AFI_Rating:
            asfTag->setRating(toTString(frame.getValue()));
            break;
          case AFI_Attributes:
          default:
          {
            TagLib::String name;
            TagLib::ASF::Attribute::AttributeTypes valueType;
            getAsfTypeForFrame(frame, name, valueType);
            TagLib::ASF::Attribute attribute =
              getAsfAttributeForFrame(frame, valueType);
            TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
            if (attrListMap.contains(name) && attrListMap[name].size() > 1) {
              int i = AFI_Attributes;
              bool found = false;
              for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
                   it != attrListMap.end();
                   ++it) {
                TagLib::ASF::AttributeList& attrList = (*it).second;
                for (TagLib::ASF::AttributeList::Iterator ait = attrList.begin();
                     ait != attrList.end();
                     ++ait) {
                  if (i++ == index) {
                    found = true;
                    *ait = attribute;
                    break;
                  }
                }
                if (found) {
                  break;
                }
              }
            } else {
              asfTag->setAttribute(name, attribute);
            }
          }
        }
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
      } else if (TagLib::RIFF::Info::Tag* infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        infoTag->setFieldText(getInfoName(frame), toTString(frame.getValue()));
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
      }
    }
  }

  // Try the basic method
  QString str = frame.getValue();
  if (makeTagSettable(tagNr) && !str.isNull()) {
    TagLib::Tag* tag = m_tag[tagNr];
    if (!tag)
      return false;
    Frame::Type type = frame.getType();
    TagLib::String tstr = str.isEmpty() ?
      TagLib::String::null : toTString(str);
    TagLib::String oldTstr;
    uint oldNum;
    const char* frameId = 0;
    switch (type) {
    case Frame::FT_Album:
      oldTstr = tag->album();
      frameId = "TALB";
      break;
    case Frame::FT_Comment:
      oldTstr = tag->comment();
      frameId = "COMM";
      break;
    case Frame::FT_Artist:
      oldTstr = tag->artist();
      frameId = "TPE1";
      break;
    case Frame::FT_Title:
      oldTstr = tag->title();
      frameId = "TIT2";
      break;
    case Frame::FT_Genre:
      oldTstr = tag->genre();
      frameId = "TCON";
      break;
    case Frame::FT_Date:
      oldNum = tag->year();
      frameId = "TDRC";
      break;
    case Frame::FT_Track:
      oldNum = tag->track();
      frameId = "TRCK";
      break;
    default:
      return false;
    }
    if (type == Frame::FT_Date) {
      int num = frame.getValueAsNumber();
      if (tagNr == Frame::Tag_Id3v1) {
        if (num >= 0 && num != static_cast<int>(oldNum)) {
          tag->setYear(num);
          markTagChanged(tagNr, type);
        }
      } else {
        if (num > 0 && num != static_cast<int>(oldNum) &&
            getDefaultTextEncoding() == TagLib::String::Latin1) {
          tag->setYear(num);
          markTagChanged(tagNr, type);
        } else if (num == 0 || num != static_cast<int>(oldNum)){
          QString yearStr;
          if (num != 0) {
            yearStr.setNum(num);
          } else {
            yearStr = frame.getValue();
          }
          TagLib::String tstr = yearStr.isEmpty() ?
            TagLib::String::null : toTString(yearStr);
          bool ok = false;
          if (dynamic_cast<TagLib::ID3v2::Tag*>(tag) != 0) {
            ok = setId3v2Unicode(tag, yearStr, tstr, frameId);
#if TAGLIB_VERSION >= 0x010600 && defined TAGLIB_WITH_MP4
          } else if (TagLib::MP4::Tag* mp4Tag =
                     dynamic_cast<TagLib::MP4::Tag*>(tag)) {
            TagLib::String name;
            Mp4ValueType valueType;
            getMp4NameForType(type, name, valueType);
            TagLib::MP4::Item item = TagLib::MP4::Item(tstr);
            ok = valueType == MVT_String && item.isValid();
            if (ok) {
              mp4Tag->itemListMap()[name] = item;
            }
#endif
          } else if (TagLib::Ogg::XiphComment* oggTag =
                     dynamic_cast<TagLib::Ogg::XiphComment*>(tag)) {
            oggTag->addField(getVorbisNameFromType(type), tstr, true);
            ok = true;
          }
          if (!ok) {
            tag->setYear(num);
          }
          markTagChanged(tagNr, type);
        }
      }
    } else if (type == Frame::FT_Track) {
      int num = frame.getValueAsNumber();
      if (num >= 0 && num != static_cast<int>(oldNum)) {
        if (tagNr == Frame::Tag_Id3v1) {
          int n = checkTruncation(tagNr, num, 1ULL << type);
          if (n != -1) {
            num = n;
          }
          tag->setTrack(num);
        } else {
          int numTracks;
          num = splitNumberAndTotal(str, &numTracks);
          QString trackStr = trackNumberString(num, numTracks);
          if (num != static_cast<int>(oldNum)) {
            TagLib::ID3v2::Tag* id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag);
#if TAGLIB_VERSION >= 0x010600 && defined TAGLIB_WITH_MP4
            TagLib::MP4::Tag* mp4Tag;
#endif
            if (id3v2Tag) {
              TagLib::String tstr = trackStr.isEmpty() ?
                TagLib::String::null : toTString(trackStr);
              if (!setId3v2Unicode(tag, trackStr, tstr, frameId)) {
                TagLib::ID3v2::TextIdentificationFrame* frame =
                    new TagLib::ID3v2::TextIdentificationFrame(
                      frameId, getDefaultTextEncoding());
                frame->setText(tstr);
                id3v2Tag->removeFrames(frameId);
#ifdef Q_OS_WIN32
                // freed in Windows DLL => must be allocated in the same DLL
                TagLib::ID3v2::Frame* dllAllocatedFrame =
                  TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
                if (dllAllocatedFrame) {
                  id3v2Tag->addFrame(dllAllocatedFrame);
                }
                delete frame;
#else
                id3v2Tag->addFrame(frame);
#endif
              }
#if TAGLIB_VERSION >= 0x010600 && defined TAGLIB_WITH_MP4
            } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(tag)) != 0) {
              // Set a frame in order to store the total number too.
              Frame frame(Frame::FT_Track, str, QLatin1String(""), -1);
              setMp4Frame(frame, mp4Tag);
#endif
#if TAGLIB_VERSION >= 0x010a00
            } else if (TagLib::RIFF::Info::Tag* infoTag =
                       dynamic_cast<TagLib::RIFF::Info::Tag*>(tag)) {
              infoTag->setFieldText(getInfoNameFromType(Frame::FT_Track),
                                    toTString(trackStr));
#endif
            } else {
              tag->setTrack(num);
            }
          }
        }
        markTagChanged(tagNr, type);
      }
    } else {
      if (!(tstr == oldTstr)) {
        if (!setId3v2Unicode(tag, str, tstr, frameId)) {
          QString s = checkTruncation(tagNr, str, 1ULL << type,
                                      type == Frame::FT_Comment ? 28 : 30);
          if (!s.isNull()) {
            tstr = toTString(s);
          }
          switch (type) {
          case Frame::FT_Album:
            tag->setAlbum(tstr);
            break;
          case Frame::FT_Comment:
            tag->setComment(tstr);
            break;
          case Frame::FT_Artist:
            tag->setArtist(tstr);
            break;
          case Frame::FT_Title:
            tag->setTitle(tstr);
            break;
          case Frame::FT_Genre:
            if (tagNr == Frame::Tag_Id3v1) {
              tag->setGenre(tstr);
              // if the string cannot be converted to a number, set the truncation flag
              checkTruncation(tagNr, !str.isEmpty() && Genres::getNumber(str) == 0xff
                              ? 1 : 0, 1ULL << type, 0);
            } else {
              TagLib::ID3v2::TextIdentificationFrame* frame;
              TagLib::ID3v2::Tag* id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag);
              if (id3v2Tag && TagConfig::instance().genreNotNumeric() &&
                  (frame = new TagLib::ID3v2::TextIdentificationFrame(
                    frameId, getDefaultTextEncoding())) != 0) {
                frame->setText(tstr);
                id3v2Tag->removeFrames(frameId);
#ifdef Q_OS_WIN32
                // freed in Windows DLL => must be allocated in the same DLL
                TagLib::ID3v2::Frame* dllAllocatedFrame =
                  TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
                if (dllAllocatedFrame) {
                  id3v2Tag->addFrame(dllAllocatedFrame);
                }
                delete frame;
#else
                id3v2Tag->addFrame(frame);
#endif
              } else {
                tag->setGenre(tstr);
              }
            }
            break;
          default:
            return false;
          }
        }
        markTagChanged(tagNr, type);
      }
    }
  }
  return true;
}

/**
 * Check if an ID3v2.4.0 frame ID is valid.
 *
 * @param frameId frame ID (4 characters)
 *
 * @return true if frame ID is valid.
 */
static bool isFrameIdValid(const QString& frameId)
{
  Frame::Type type;
  const char* str;
  getTypeStringForFrameId(TagLib::ByteVector(frameId.toLatin1().data(), 4), type, str);
  return type != Frame::FT_UnknownFrame;
}

/**
 * Create a TagLib ID3 frame from a frame.
 * @param self this TagLibFile instance
 * @param frame frame
 * @return TagLib ID3 frame, 0 if invalid.
 */
static TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
                                                     Frame& frame)
{
  TagLib::String::Type enc = self->getDefaultTextEncoding();
  QString name = frame.getType() != Frame::FT_Other ?
    QString::fromLatin1(getStringForType(frame.getType())) :
    frame.getName();
  QString frameId = name;
  frameId.truncate(4);
  TagLib::ID3v2::Frame* id3Frame = 0;

  if (name == QLatin1String("AverageLevel") ||
      name == QLatin1String("PeakValue") ||
      name.startsWith(QLatin1String("WM/"))) {
    frameId = QLatin1String("PRIV");
  } else if (name.startsWith(QLatin1String("iTun"))) {
    frameId = QLatin1String("COMM");
  }

  if (frameId.startsWith(QLatin1String("T"))
#if TAGLIB_VERSION >= 0x010b00
      || frameId == QLatin1String("WFED")
#endif
#ifdef TAGLIB_WITH_MP4_SHWM
      || frameId == QLatin1String("MVIN") || frameId == QLatin1String("MVNM")
#endif
    ) {
    if (frameId == QLatin1String("TXXX")) {
      id3Frame = new TagLib::ID3v2::UserTextIdentificationFrame(enc);
    } else if (isFrameIdValid(frameId)) {
      id3Frame = new TagLib::ID3v2::TextIdentificationFrame(
        TagLib::ByteVector(frameId.toLatin1().data(), frameId.length()), enc);
      id3Frame->setText(""); // is necessary for createFrame() to work
    }
  } else if (frameId == QLatin1String("COMM")) {
    TagLib::ID3v2::CommentsFrame* commFrame =
        new TagLib::ID3v2::CommentsFrame(enc);
    id3Frame = commFrame;
    commFrame->setLanguage("eng"); // for compatibility with iTunes
    if (frame.getType() == Frame::FT_Other) {
      commFrame->setDescription(toTString(frame.getName()));
    }
  } else if (frameId == QLatin1String("APIC")) {
    id3Frame = new TagLib::ID3v2::AttachedPictureFrame;
    ((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setTextEncoding(enc);
    ((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setMimeType(
      "image/jpeg");
    ((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setType(
      TagLib::ID3v2::AttachedPictureFrame::FrontCover);
  } else if (frameId == QLatin1String("UFID")) {
    // the bytevector must not be empty
    TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame =
        new TagLib::ID3v2::UniqueFileIdentifierFrame(
                  TagLib::String("http://www.id3.org/dummy/ufid.html"),
                  TagLib::ByteVector(" "));
    id3Frame = ufidFrame;
    QByteArray data;
    if (AttributeData::isHexString(frame.getValue(), 'Z')) {
      data = (frame.getValue() + QLatin1Char('\0')).toLatin1();
      ufidFrame->setIdentifier(TagLib::ByteVector(data.constData(),
                                                  data.size()));
    }
  } else if (frameId == QLatin1String("GEOB")) {
    id3Frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
    ((TagLib::ID3v2::GeneralEncapsulatedObjectFrame*)id3Frame)->setTextEncoding(enc);
  } else if (frameId.startsWith(QLatin1String("W"))) {
    if (frameId == QLatin1String("WXXX")) {
      id3Frame = new TagLib::ID3v2::UserUrlLinkFrame(enc);
    } else if (isFrameIdValid(frameId)) {
      id3Frame = new TagLib::ID3v2::UrlLinkFrame(
        TagLib::ByteVector(frameId.toLatin1().data(), frameId.length()));
      id3Frame->setText("http://"); // is necessary for createFrame() to work
    }
  } else if (frameId == QLatin1String("USLT")) {
    id3Frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(enc);
    ((TagLib::ID3v2::UnsynchronizedLyricsFrame*)id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("SYLT")) {
    id3Frame = new TagLib::ID3v2::SynchronizedLyricsFrame(enc);
    ((TagLib::ID3v2::SynchronizedLyricsFrame*)id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("ETCO")) {
    id3Frame = new TagLib::ID3v2::EventTimingCodesFrame;
#if TAGLIB_VERSION >= 0x010600
  } else if (frameId == QLatin1String("POPM")) {
    TagLib::ID3v2::PopularimeterFrame* popmFrame =
        new TagLib::ID3v2::PopularimeterFrame;
    id3Frame = popmFrame;
    popmFrame->setEmail(toTString(TagConfig::instance().defaultPopmEmail()));
  } else if (frameId == QLatin1String("PRIV")) {
    TagLib::ID3v2::PrivateFrame* privFrame =
        new TagLib::ID3v2::PrivateFrame;
    id3Frame = privFrame;
    if (!frame.getName().startsWith(QLatin1String("PRIV"))) {
      privFrame->setOwner(toTString(frame.getName()));
      QByteArray data;
      if (AttributeData(frame.getName()).toByteArray(frame.getValue(), data)) {
        privFrame->setData(TagLib::ByteVector(data.constData(), data.size()));
      }
    }
#endif
#if TAGLIB_VERSION >= 0x010800
  } else if (frameId == QLatin1String("OWNE")) {
    id3Frame = new TagLib::ID3v2::OwnershipFrame(enc);
#endif
#if TAGLIB_VERSION >= 0x010500
  } else if (frameId == QLatin1String("RVA2")) {
    id3Frame = new TagLib::ID3v2::RelativeVolumeFrame;
#endif
#if TAGLIB_VERSION >= 0x010b00
  } else if (frameId == QLatin1String("PCST")) {
    id3Frame = new TagLib::ID3v2::PodcastFrame;
#endif
#if TAGLIB_VERSION >= 0x010a00
  } else if (frameId == QLatin1String("CHAP")) {
    // crashes with an empty elementID
    id3Frame = new TagLib::ID3v2::ChapterFrame("chp", 0, 0,
                                               0xffffffff, 0xffffffff);
  } else if (frameId == QLatin1String("CTOC")) {
    // crashes with an empty elementID
    id3Frame = new TagLib::ID3v2::TableOfContentsFrame("toc");
#endif
  }
  if (!id3Frame) {
    TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame =
      new TagLib::ID3v2::UserTextIdentificationFrame(enc);
    TagLib::String description;
    if (frame.getType() == Frame::FT_CatalogNumber) {
      description = "CATALOGNUMBER";
    } else if (frame.getType() == Frame::FT_ReleaseCountry) {
      description = "RELEASECOUNTRY";
    } else {
      description = toTString(frame.getName());
      frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                    QLatin1String("TXXX - User defined text information")));
    }
    txxxFrame->setDescription(description);
    id3Frame = txxxFrame;
  } else {
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
  }
  if (id3Frame) {
    if (!frame.fieldList().empty()) {
      frame.setValueFromFieldList();
      setId3v2Frame(self, id3Frame, frame);
    }
  }
  return id3Frame;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TagLibFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  if (tagNr != Frame::Tag_Id3v1) {
    // Add a new frame.
    if (makeTagSettable(tagNr)) {
      TagLib::ID3v2::Tag* id3v2Tag;
      TagLib::Ogg::XiphComment* oggTag;
      TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
      TagLib::ASF::Tag* asfTag;
#endif
#endif
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
        TagLib::ID3v2::Frame* id3Frame = createId3FrameFromFrame(this, frame);
        if (id3Frame) {
#ifdef Q_OS_WIN32
          // freed in Windows DLL => must be allocated in the same DLL
          TagLib::ID3v2::Frame* dllAllocatedFrame =
            TagLib::ID3v2::FrameFactory::instance()->createFrame(id3Frame->render());
          if (dllAllocatedFrame) {
            id3v2Tag->addFrame(dllAllocatedFrame);
          }
#else
          id3v2Tag->addFrame(id3Frame);
#endif
          frame.setIndex(id3v2Tag->frameList().size() - 1);
          if (frame.fieldList().empty()) {
            // add field list to frame
            getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
            frame.setFieldListFromValue();
          }
          if (frame.getType() == Frame::FT_Other) {
            // Set the correct frame type if the frame was added using the ID.
            Frame::Type type;
            const char* str;
            getTypeStringForFrameId(id3Frame->frameID(), type, str);
            if (type != Frame::FT_UnknownFrame) {
              frame.setExtendedType(
                    Frame::ExtendedType(type, QString::fromLatin1(str)));
            }
          }
#ifdef Q_OS_WIN32
          delete id3Frame;
#endif
          markTagChanged(tagNr, frame.getType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != 0) {
        QString name(getVorbisName(frame));
        QString value(frame.getValue());
        if (frame.getType() == Frame::FT_Picture) {
#if TAGLIB_VERSION >= 0x010700
          if (frame.getFieldList().empty()) {
            PictureFrame::setFields(
              frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), QLatin1String("image/jpeg"),
              PictureFrame::PT_CoverFront, QLatin1String(""), QByteArray());
          }
          if (m_pictures.isRead()) {
            PictureFrame::setDescription(frame, value);
            frame.setIndex(m_pictures.size());
            m_pictures.append(frame);
            markTagChanged(tagNr, Frame::FT_Picture);
            return true;
          } else {
            PictureFrame::getFieldsToBase64(frame, value);
          }
#else
          return false;
#endif
        }
        TagLib::String tname = toTString(name);
        TagLib::String tvalue = toTString(value);
        if (tvalue.isEmpty()) {
          tvalue = " "; // empty values are not added by TagLib
        }
#if TAGLIB_VERSION <= 0x010400
        oggTag->addField(tname, tvalue);
#else
        oggTag->addField(tname, tvalue, false);
#endif
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        int index = 0;
        bool found = false;
        for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
             it != fieldListMap.end();
             ++it) {
          if ((*it).first == tname) {
            index += (*it).second.size() - 1;
            found = true;
            break;
          }
          index += (*it).second.size();
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
#if TAGLIB_VERSION >= 0x010800
        if (frame.getType() == Frame::FT_Picture &&
            frame.getFieldList().isEmpty()) {
          // Do not replace an already existing picture.
          Frame::PictureType pictureType = Frame::PT_CoverFront;
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (int i = Frame::PT_CoverFront; i <= Frame::PT_PublisherLogo; ++i) {
            Frame::PictureType pt = static_cast<Frame::PictureType>(i);
            if (itemListMap.find(getApePictureName(pt)) == itemListMap.end()) {
              pictureType = pt;
              break;
            }
          }
          PictureFrame::setFields(
                frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
                QLatin1String("image/jpeg"), pictureType);
        }
        QString name(getApeName(frame));
        TagLib::String tname = toTString(name);
        if (frame.getType() == Frame::FT_Picture) {
          TagLib::ByteVector data;
          renderApePicture(frame, data);
          apeTag->setData(tname, data);
        } else {
          TagLib::String tvalue = toTString(frame.getValue());
          if (tvalue.isEmpty()) {
            tvalue = " "; // empty values are not added by TagLib
          }
          apeTag->addValue(tname, tvalue, true);
        }
#else
        if (frame.getType() == Frame::FT_Picture) {
          return false;
        }
        QString name(getApeName(frame));
        TagLib::String tname = toTString(name);
        TagLib::String tvalue = toTString(frame.getValue());
        if (tvalue.isEmpty()) {
          tvalue = " "; // empty values are not added by TagLib
        }
        apeTag->addValue(tname, tvalue, true);
#endif
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

        const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
        int index = 0;
        bool found = false;
        for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          if ((*it).first == tname) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getType());
        return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
#if TAGLIB_VERSION >= 0x010602
        if (frame.getType() == Frame::FT_Picture &&
            frame.getFieldList().empty()) {
          PictureFrame::setFields(frame);
        }
#endif
        TagLib::String name;
        TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
        if (!item.isValid()) {
          return false;
        }
        frame.setExtendedType(Frame::ExtendedType(frame.getType(),
                                                  toQString(name)));
        prefixMp4FreeFormName(name, mp4Tag);
        mp4Tag->itemListMap()[name] = item;
        const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
        int index = 0;
        bool found = false;
        for (TagLib::MP4::ItemListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          if ((*it).first == name) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
#ifdef TAGLIB_WITH_ASF
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
#if TAGLIB_VERSION >= 0x010602
        if (frame.getType() == Frame::FT_Picture &&
            frame.getFieldList().empty()) {
          PictureFrame::setFields(frame);
        }
#else
        if (frame.getType() == Frame::FT_Picture) {
          return false;
        }
#endif
        TagLib::String name;
        TagLib::ASF::Attribute::AttributeTypes valueType;
        getAsfTypeForFrame(frame, name, valueType);
        if (valueType == TagLib::ASF::Attribute::BytesType &&
            frame.getType() != Frame::FT_Picture) {
          Frame::Field field;
          field.m_id = Frame::ID_Data;
          field.m_value = QByteArray();
          frame.fieldList().push_back(field);
        }
        TagLib::ASF::Attribute attribute = getAsfAttributeForFrame(frame, valueType);
        asfTag->addAttribute(name, attribute);
        frame.setExtendedType(Frame::ExtendedType(frame.getType(),
                                                  toQString(name)));

        const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
        int index = AFI_Attributes;
        bool found = false;
        for (TagLib::ASF::AttributeListMap::ConstIterator it = attrListMap.begin();
             it != attrListMap.end();
             ++it) {
          if ((*it).first == name) {
            index += (*it).second.size() - 1;
            found = true;
            break;
          }
          index += (*it).second.size();
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
      } else if (TagLib::RIFF::Info::Tag* infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        TagLib::ByteVector id = getInfoName(frame);
        TagLib::String tvalue = toTString(frame.getValue());
        if (tvalue.isEmpty()) {
          tvalue = " "; // empty values are not added by TagLib
        }
        infoTag->setFieldText(id, tvalue);
        QString name = QString::fromLatin1(id.data(), id.size());
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
        TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
        int index = 0;
        bool found = false;
        for (TagLib::RIFF::Info::FieldListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          if ((*it).first == id) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
      }
    }
  }

  // Try the superclass method
  return TaggedFile::addFrame(tagNr, frame);
}

/**
 * Delete a frame from the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool TagLibFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  if (tagNr != Frame::Tag_Id3v1) {
    makeFileOpen();
    // If the frame has an index, delete that specific frame
    int index = frame.getIndex();
    if (index != -1 && m_tag[tagNr]) {
      TagLib::ID3v2::Tag* id3v2Tag;
      TagLib::Ogg::XiphComment* oggTag;
      TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
      TagLib::ASF::Tag* asfTag;
#endif
#endif
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        if (index < static_cast<int>(frameList.size())) {
          id3v2Tag->removeFrame(frameList[index]);
          markTagChanged(tagNr, frame.getType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != 0) {
        QString frameValue(frame.getValue());
#if TAGLIB_VERSION >= 0x010700
        if (frame.getType() == Frame::FT_Picture) {
          if (m_pictures.isRead()) {
            int idx = frame.getIndex();
            if (idx >= 0 && idx < m_pictures.size()) {
              m_pictures.removeAt(idx);
              while (idx < m_pictures.size()) {
                m_pictures[idx].setIndex(idx);
                ++idx;
              }
              markTagChanged(tagNr, Frame::FT_Picture);
              return true;
            }
          } else {
            PictureFrame::getFieldsToBase64(frame, frameValue);
          }
        }
#endif
        TagLib::String key =
          toTString(frame.getInternalName());
#if TAGLIB_VERSION <= 0x010400
        // Remove all fields with that key, because TagLib <= 1.4 crashes
        // using an invalidated iterator after calling erase().
        oggTag->removeField(key);
#else
        oggTag->removeField(key, toTString(frameValue));
#endif
        markTagChanged(tagNr, frame.getType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
        TagLib::String key = toTString(frame.getInternalName());
        apeTag->removeItem(key);
        markTagChanged(tagNr, frame.getType());
        return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
        TagLib::String name = toTString(frame.getInternalName());
        prefixMp4FreeFormName(name, mp4Tag);
        mp4Tag->itemListMap().erase(name);
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
#ifdef TAGLIB_WITH_ASF
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
        switch (index) {
          case AFI_Title:
            asfTag->setTitle("");
            break;
          case AFI_Artist:
            asfTag->setArtist("");
            break;
          case AFI_Comment:
            asfTag->setComment("");
            break;
          case AFI_Copyright:
            asfTag->setCopyright("");
            break;
          case AFI_Rating:
            asfTag->setRating("");
            break;
          case AFI_Attributes:
          default:
          {
            TagLib::String name = toTString(frame.getInternalName());
            TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
            if (attrListMap.contains(name) && attrListMap[name].size() > 1) {
              int i = AFI_Attributes;
              bool found = false;
              for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
                   it != attrListMap.end();
                   ++it) {
                TagLib::ASF::AttributeList& attrList = (*it).second;
                for (TagLib::ASF::AttributeList::Iterator ait = attrList.begin();
                     ait != attrList.end();
                     ++ait) {
                  if (i++ == index) {
                    found = true;
                    attrList.erase(ait);
                    break;
                  }
                }
                if (found) {
                  break;
                }
              }
            } else {
              asfTag->removeItem(name);
            }
          }
        }
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
      } else if (TagLib::RIFF::Info::Tag* infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        QByteArray ba = frame.getInternalName().toLatin1();
        TagLib::ByteVector id(ba.constData(), ba.size());
        infoTag->removeField(id);
        markTagChanged(tagNr, frame.getType());
        return true;
#endif
      }
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

/**
 * Create a frame from a TagLib ID3 frame.
 * @param id3Frame TagLib ID3 frame
 * @param index, -1 if not used
 * @return frame.
 */
static Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index)
{
  Frame::Type type;
  const char* name;
  getTypeStringForFrameId(id3Frame->frameID(), type, name);
  Frame frame(type, toQString(id3Frame->toString()), QString::fromLatin1(name), index);
  frame.setValue(getFieldsFromId3Frame(id3Frame, frame.fieldList(), type));
  if (id3Frame->frameID().mid(1, 3) == "XXX" ||
      type == Frame::FT_Comment) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Description);
    if (fieldValue.isValid()) {
      QString description = fieldValue.toString();
      if (!description.isEmpty()) {
        if (description == QLatin1String("CATALOGNUMBER")) {
          frame.setType(Frame::FT_CatalogNumber);
        } else if (description == QLatin1String("RELEASECOUNTRY")) {
          frame.setType(Frame::FT_ReleaseCountry);
        } else {
          if (description.startsWith(QLatin1String("QuodLibet::"))) {
            // remove ExFalso/QuodLibet "namespace"
            description = description.mid(11);
          }
          frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
            frame.getInternalName() + QLatin1Char('\n') + description));
        }
      }
    }
#if TAGLIB_VERSION >= 0x010600
  } else if (id3Frame->frameID().startsWith("PRIV")) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Owner);
    if (fieldValue.isValid()) {
      QString owner = fieldValue.toString();
      if (!owner.isEmpty()) {
        frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                  frame.getInternalName() + QLatin1Char('\n') + owner));
      }
    }
#endif
  }
  return frame;
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void TagLibFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr >= NUM_TAGS)
    return;

  makeFileOpen();
  if (tagNr == Frame::Tag_Id3v1) {
    if (m_tag[tagNr]) {
      TaggedFile::deleteFrames(tagNr, flt);
    }
  } else {
    if (m_tag[tagNr]) {
      TagLib::ID3v2::Tag* id3v2Tag;
      TagLib::Ogg::XiphComment* oggTag;
      TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
      TagLib::ASF::Tag* asfTag;
#endif
#endif
      if (flt.areAllEnabled()) {
        if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
          const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
               it != frameList.end();) {
            id3v2Tag->removeFrame(*it++, true);
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
        } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) !=
                   0) {
          const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
          for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
               it != fieldListMap.end();) {
            oggTag->removeField((*it++).first);
          }
#if TAGLIB_VERSION >= 0x010700
          m_pictures.clear();
#endif
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
        } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
               it != itemListMap.end();) {
            apeTag->removeItem((*it++).first);
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
        } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
          mp4Tag->itemListMap().clear();
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
#ifdef TAGLIB_WITH_ASF
        } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
          asfTag->setTitle("");
          asfTag->setArtist("");
          asfTag->setComment("");
          asfTag->setCopyright("");
          asfTag->setRating("");
          asfTag->attributeListMap().clear();
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
        } else if (TagLib::RIFF::Info::Tag* infoTag =
                   dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
          TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
          for (TagLib::RIFF::Info::FieldListMap::ConstIterator it = itemListMap.begin();
               it != itemListMap.end();
               ++it) {
            infoTag->removeField((*it).first);
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
        } else {
          TaggedFile::deleteFrames(tagNr, flt);
        }
      } else {
        if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
          const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
               it != frameList.end();) {
            Frame frame(createFrameFromId3Frame(*it, -1));
            if (flt.isEnabled(frame.getType(), frame.getName())) {
              id3v2Tag->removeFrame(*it++, true);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
        } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) !=
                   0) {
          const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
          for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
               it != fieldListMap.end();) {
            QString name(toQString((*it).first));
            if (flt.isEnabled(getTypeFromVorbisName(name), name)) {
              oggTag->removeField((*it++).first);
            } else {
              ++it;
            }
          }
#if TAGLIB_VERSION >= 0x010700
          if (flt.isEnabled(Frame::FT_Picture)) {
            m_pictures.clear();
          }
#endif
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
        } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
               it != itemListMap.end();) {
            QString name(toQString((*it).first));
            if (flt.isEnabled(getTypeFromApeName(name), name)) {
              apeTag->removeItem((*it++).first);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
        } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
          TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
          Frame::Type type;
          Mp4ValueType valueType;
          for (TagLib::MP4::ItemListMap::Iterator it = itemListMap.begin();
               it != itemListMap.end();) {
            TagLib::String name = (*it).first;
            stripMp4FreeFormName(name);
            getMp4TypeForName(name, type, valueType);
            if (flt.isEnabled(type, toQString(name))) {
              itemListMap.erase(it++);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
#ifdef TAGLIB_WITH_ASF
        } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
          if (flt.isEnabled(Frame::FT_Title))
            asfTag->setTitle("");
          if (flt.isEnabled(Frame::FT_Artist))
            asfTag->setArtist("");
          if (flt.isEnabled(Frame::FT_Comment))
            asfTag->setComment("");
          if (flt.isEnabled(Frame::FT_Copyright))
            asfTag->setCopyright("");
          if (flt.isEnabled(Frame::FT_Other, QLatin1String("Rating Information")))
            asfTag->setRating("");

          TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
          Frame::Type type;
          TagLib::ASF::Attribute::AttributeTypes valueType;
          for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
               it != attrListMap.end();) {
            getAsfTypeForName((*it).first, type, valueType);
            QString name(toQString((*it).first));
            if (flt.isEnabled(type, name)) {
              attrListMap.erase(it++);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
        } else if (TagLib::RIFF::Info::Tag* infoTag =
                   dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
          TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
          for (TagLib::RIFF::Info::FieldListMap::ConstIterator it = itemListMap.begin();
               it != itemListMap.end();
               ++it) {
            TagLib::ByteVector id = (*it).first;
            QString name = QString::fromLatin1(id.data(), id.size());
            if (flt.isEnabled(getTypeFromInfoName(id), name)) {
              infoTag->removeField(id);
            }
          }
          markTagChanged(tagNr, Frame::FT_UnknownFrame);
#endif
        } else {
          TaggedFile::deleteFrames(tagNr, flt);
        }
      }
    }
  }
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void TagLibFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  if (tagNr >= NUM_TAGS)
    return;

  if (tagNr != Frame::Tag_Id3v1) {
    makeFileOpen();
    frames.clear();
    if (m_tag[tagNr]) {
      TagLib::ID3v2::Tag* id3v2Tag;
      TagLib::Ogg::XiphComment* oggTag;
      TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
      TagLib::ASF::Tag* asfTag;
#endif
#endif
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != 0) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        int i = 0;
        for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
             it != frameList.end();
             ++it) {
          Frame frame(createFrameFromId3Frame(*it, i++));
          if (frame.getType() == Frame::FT_UnknownFrame) {
            TagLib::ByteVector frameID = (*it)->frameID().mid(0, 4);
            if (frameID == "TDAT" || frameID == "TIME" || frameID == "TRDA" ||
                frameID == "TYER") {
              // These frames are converted to a TDRC frame by TagLib.
              continue;
            }
          }
          frames.insert(frame);
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != 0) {
        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        int i = 0;
        for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
             it != fieldListMap.end();
             ++it) {
          QString name = toQString((*it).first);
          Frame::Type type = getTypeFromVorbisName(name);
          TagLib::StringList stringList = (*it).second;
          for (TagLib::StringList::ConstIterator slit = stringList.begin();
               slit != stringList.end();
               ++slit) {
            if (type == Frame::FT_Picture) {
              Frame frame(type, QLatin1String(""), name, i++);
              PictureFrame::setFieldsFromBase64(
                    frame, toQString(TagLib::String(*slit)));
              if (name == QLatin1String("COVERART")) {
                TagLib::StringList mt = oggTag->fieldListMap()["COVERARTMIME"];
                if (!mt.isEmpty()) {
                  PictureFrame::setMimeType(frame, toQString(mt.front()));
                }
              }
              frames.insert(frame);
            } else {
              frames.insert(Frame(type, toQString(TagLib::String(*slit)),
                                  name, i++));
            }
          }
        }
#if TAGLIB_VERSION >= 0x010700
        if (m_pictures.isRead()) {
          for (Pictures::iterator it = m_pictures.begin();
               it != m_pictures.end();
               ++it) {
            frames.insert(*it);
          }
        }
#endif
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != 0) {
        const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
        int i = 0;
        for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          QString name = toQString((*it).first);
          Frame::Type type = getTypeFromApeName(name);
          TagLib::StringList values;
          if (type != Frame::FT_Picture) {
            values = (*it).second.toStringList();
          }
          Frame frame(type, values.size() > 0
                      ? toQString(values.front()) : QLatin1String(""),
                      name, i++);
#if TAGLIB_VERSION >= 0x010800
          if (type == Frame::FT_Picture) {
            TagLib::ByteVector data = (*it).second.binaryData();
            parseApePicture(name, data, frame);
          }
#endif
          frames.insert(frame);
        }
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != 0) {
        const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
        int i = 0;
        for (TagLib::MP4::ItemListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          TagLib::String name = (*it).first;
          stripMp4FreeFormName(name);
          Frame::Type type;
          Mp4ValueType valueType;
          getMp4TypeForName(name, type, valueType);
          QString value;
          bool frameAlreadyInserted = false;
          switch (valueType) {
            case MVT_String:
            {
              TagLib::StringList strings = (*it).second.toStringList();
              value = strings.size() > 0 ? toQString(strings.front()) : QLatin1String("");
              break;
            }
            case MVT_Bool:
              value = (*it).second.toBool() ? QLatin1String("1") : QLatin1String("0");
              break;
            case MVT_Int:
              value.setNum((*it).second.toInt());
              break;
            case MVT_IntPair:
            {
              TagLib::MP4::Item::IntPair intPair = (*it).second.toIntPair();
              value.setNum(intPair.first);
              if (intPair.second != 0) {
                value += QLatin1Char('/');
                value += QString::number(intPair.second);
              }
              break;
            }
#if TAGLIB_VERSION >= 0x010602
            case MVT_CoverArt:
            {
              TagLib::MP4::CoverArtList coverArtList = (*it).second.toCoverArtList();
              if (coverArtList.size() > 0) {
                const TagLib::MP4::CoverArt& coverArt = coverArtList.front();
                TagLib::ByteVector bv = coverArt.data();
                Frame frame(type, QLatin1String(""), toQString(name), i++);
                QByteArray ba;
                ba = QByteArray(bv.data(), bv.size());
                PictureFrame::setFields(
                  frame, Frame::TE_ISO8859_1,
                  coverArt.format() == TagLib::MP4::CoverArt::PNG ? QLatin1String("PNG") : QLatin1String("JPG"),
                  coverArt.format() == TagLib::MP4::CoverArt::PNG ?
                  QLatin1String("image/png") : QLatin1String("image/jpeg"),
                  PictureFrame::PT_CoverFront, QLatin1String(""), ba);
                frames.insert(frame);
                frameAlreadyInserted = true;
              }
              break;
            }
#endif
#if TAGLIB_VERSION >= 0x010800
            case MVT_Byte:
              value.setNum((*it).second.toByte());
              break;
            case MVT_UInt:
              value.setNum((*it).second.toUInt());
              break;
            case MVT_LongLong:
              value.setNum((*it).second.toLongLong());
              break;
#endif
            case MVT_ByteArray:
            default:
              // binary data and album art are not handled by TagLib
              value = QLatin1String("");
          }
          if (!frameAlreadyInserted)
            frames.insert(
              Frame(type, value, toQString(name), i++));
        }
#endif
#ifdef TAGLIB_WITH_ASF
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != 0) {
        TagLib::String name;
        TagLib::ASF::Attribute::AttributeTypes valueType;
        Frame::Type type = Frame::FT_Title;
        getAsfNameForType(type, name, valueType);
        QString value = toQString(asfTag->title());
        frames.insert(Frame(type, value, toQString(name), AFI_Title));

        type = Frame::FT_Artist;
        getAsfNameForType(type, name, valueType);
        value = toQString(asfTag->artist());
        frames.insert(Frame(type, value, toQString(name), AFI_Artist));

        type = Frame::FT_Comment;
        getAsfNameForType(type, name, valueType);
        value = toQString(asfTag->comment());
        frames.insert(Frame(type, value, toQString(name), AFI_Comment));

        type = Frame::FT_Copyright;
        getAsfNameForType(type, name, valueType);
        value = toQString(asfTag->copyright());
        frames.insert(Frame(type, value, toQString(name), AFI_Copyright));

        name = QT_TRANSLATE_NOOP("@default", "Rating Information");
        getAsfTypeForName(name, type, valueType);
        value = toQString(asfTag->rating());
        frames.insert(Frame(type, value, toQString(name), AFI_Rating));

        int i = AFI_Attributes;
        QByteArray ba;
        const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
        for (TagLib::ASF::AttributeListMap::ConstIterator it = attrListMap.begin();
             it != attrListMap.end();
             ++it) {
          name = (*it).first;
          getAsfTypeForName(name, type, valueType);
          for (TagLib::ASF::AttributeList::ConstIterator ait = (*it).second.begin();
               ait != (*it).second.end();
               ++ait) {
            switch ((*ait).type()) {
              case TagLib::ASF::Attribute::UnicodeType:
                value = toQString((*ait).toString());
                break;
              case TagLib::ASF::Attribute::BoolType:
                value = (*ait).toBool() ? QLatin1String("1") : QLatin1String("0");
                break;
              case TagLib::ASF::Attribute::DWordType:
                value.setNum((*ait).toUInt());
                break;
              case TagLib::ASF::Attribute::QWordType:
                value.setNum((*ait).toULongLong());
                break;
              case TagLib::ASF::Attribute::WordType:
                value.setNum((*ait).toUShort());
                break;
              case TagLib::ASF::Attribute::BytesType:
              case TagLib::ASF::Attribute::GuidType:
              default:
              {
                TagLib::ByteVector bv = (*ait).toByteVector();
                ba = QByteArray(bv.data(), bv.size());
                value = QLatin1String("");
                AttributeData(toQString(name)).toString(ba, value);
              }
            }
            Frame frame(type, value, toQString(name), i);
            if ((*ait).type() == TagLib::ASF::Attribute::BytesType &&
                valueType == TagLib::ASF::Attribute::BytesType) {
              Frame::Field field;
              field.m_id = Frame::ID_Data;
              field.m_value = ba;
              frame.fieldList().push_back(field);
            }
#if TAGLIB_VERSION >= 0x010700
            ++i;
            if (type == Frame::FT_Picture) {
              parseAsfPicture((*ait).toPicture(), frame);
            }
            frames.insert(frame);
#elif TAGLIB_VERSION >= 0x010602
            ++i;
            if (type == Frame::FT_Picture) {
              parseAsfPicture((*ait).toByteVector(), frame);
            }
            frames.insert(frame);
#else
            if (type != Frame::FT_Picture) {
              ++i;
              frames.insert(frame);
            }
#endif
          }
        }
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
      } else if (TagLib::RIFF::Info::Tag* infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
        int i = 0;
        for (TagLib::RIFF::Info::FieldListMap::ConstIterator it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          TagLib::ByteVector id = (*it).first;
          TagLib::String s = (*it).second;
          QString name = QString::fromLatin1(id.data(), id.size());
          QString value = toQString(s);
          Frame::Type type = getTypeFromInfoName(id);
          Frame frame(type, value, name, i++);
          frames.insert(frame);
        }
#endif
      } else {
        TaggedFile::getAllFrames(tagNr, frames);
      }
    }
    updateMarkedState(tagNr, frames);
    if (tagNr <= Frame::Tag_2) {
      frames.addMissingStandardFrames();
    }
    return;
  }

  TaggedFile::getAllFrames(tagNr, frames);
}

/**
 * Close file handle which is held open by the TagLib object.
 */
void TagLibFile::closeFileHandle()
{
  closeFile(false);
}

/**
 * Add a suitable field list for the frame if missing.
 * If a frame is created, its field list is empty. This method will create
 * a field list appropriate for the frame type and tagged file type if no
 * field list exists.
 * @param tagNr tag number
 * @param frame frame where field list is added
 */
void TagLibFile::addFieldList(Frame::TagNumber tagNr, Frame& frame) const
{
  if (dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr]) != 0 &&
      frame.fieldList().isEmpty()) {
    TagLib::ID3v2::Frame* id3Frame = createId3FrameFromFrame(this, frame);
    getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
    frame.setFieldListFromValue();
    delete id3Frame;
  }
}

/**
 * Get a list of frame IDs which can be added.
 * @param tagNr tag number
 * @return list with frame IDs.
 */
QStringList TagLibFile::getFrameIds(Frame::TagNumber tagNr) const
{
  QStringList lst;
  if (m_tagType[tagNr] == TT_Id3v2) {
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      lst.append(Frame::ExtendedType(static_cast<Frame::Type>(k), QLatin1String("")).
                 getName());
    }
    for (unsigned i = 0; i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]); ++i) {
      const TypeStrOfId& ts = typeStrOfId[i];
      if (ts.type == Frame::FT_Other && ts.supported && ts.str) {
        lst.append(QString::fromLatin1(ts.str));
      }
    }
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
  } else if (m_tagType[tagNr] == TT_Mp4) {
    TagLib::String name;
    Mp4ValueType valueType;
    Frame::Type type;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      name = "";
      type = static_cast<Frame::Type>(k);
      getMp4NameForType(type, name, valueType);
      if (!name.isEmpty() && valueType != MVT_ByteArray &&
          !(name[0] >= 'A' && name[0] <= 'Z')) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName());
      }
    }
    for (unsigned i = 0; i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
      if (mp4NameTypeValues[i].type == Frame::FT_Other &&
          mp4NameTypeValues[i].value != MVT_ByteArray &&
          !(mp4NameTypeValues[i].name[0] >= 'A' &&
            mp4NameTypeValues[i].name[0] <= 'Z')) {
        lst.append(QString::fromLatin1(mp4NameTypeValues[i].name));
      }
    }
#endif
#ifdef TAGLIB_WITH_ASF
  } else if (m_tagType[tagNr] == TT_Asf) {
    TagLib::String name;
    TagLib::ASF::Attribute::AttributeTypes valueType;
    Frame::Type type;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      name = "";
      type = static_cast<Frame::Type>(k);
      getAsfNameForType(type, name, valueType);
      if (!name.isEmpty()
#if !(TAGLIB_VERSION >= 0x010602)
          && type != Frame::FT_Picture
#endif
        ) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName());
      }
    }
    for (unsigned i = 0; i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
      if (asfNameTypeValues[i].type == Frame::FT_Other) {
        lst.append(QString::fromLatin1(asfNameTypeValues[i].name));
      }
    }
#endif
#endif
#if TAGLIB_VERSION >= 0x010a00
  } else if (m_tagType[tagNr] == TT_Info) {
    static const char* const fieldNames[] = {
      "IARL", // Archival Location
      "ICMS", // Commissioned
      "ICRP", // Cropped
      "IDIM", // Dimensions
      "IDPI", // Dots Per Inch
      "IKEY", // Keywords
      "ILGT", // Lightness
      "IPLT", // Palette Setting
      "ISBJ", // Subject
      "ISHP", // Sharpness
      "ISRF", // Source Form
    };
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      Frame::Type type = static_cast<Frame::Type>(k);
      if (!getInfoNameFromType(type).isEmpty()) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName());
      }
    }
    for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
      lst.append(QString::fromLatin1(fieldNames[i]));
    }
#endif
  } else {
    static const char* const fieldNames[] = {
      "CONTACT",
      "DESCRIPTION",
      "DISCTOTAL",
      "EAN/UPN",
      "ENCODING",
      "ENGINEER",
      "ENSEMBLE",
      "GUESTARTIST",
      "LABEL",
      "LABELNO",
      "LICENSE",
      "LOCATION",
      "OPUS",
      "ORGANIZATION",
      "PARTNUMBER",
      "PRODUCER",
      "PRODUCTNUMBER",
      "RECORDINGDATE",
      "TRACKTOTAL",
      "VERSION",
      "VOLUME"
    };
#if TAGLIB_VERSION >= 0x010800
    const bool picturesSupported = m_pictures.isRead() ||
        m_tagType[tagNr] == TT_Vorbis || m_tagType[tagNr] == TT_Ape;
#elif TAGLIB_VERSION >= 0x010700
    const bool picturesSupported = m_pictures.isRead() ||
        m_tagType[tagNr] == TT_Vorbis;
#else
    const bool picturesSupported = false;
#endif
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      if (k != Frame::FT_Picture || picturesSupported) {
        lst.append(Frame::ExtendedType(static_cast<Frame::Type>(k), QLatin1String("")).
                   getName());
      }
    }
    for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
      lst.append(QString::fromLatin1(fieldNames[i]));
    }
  }
  return lst;
}

/**
 * Set the text codec to be used for tag 1.
 *
 * @param codec text codec, 0 to use default (ISO 8859-1)
 */
void TagLibFile::setTextCodecV1(const QTextCodec* codec)
{
  TextCodecStringHandler::setTextCodec(codec);
}

/**
 * Set the default text encoding.
 *
 * @param textEnc default text encoding
 */
void TagLibFile::setDefaultTextEncoding(TagConfig::TextEncoding textEnc)
{
  // Do not use TagLib::ID3v2::FrameFactory::setDefaultTextEncoding(),
  // it will change the encoding of existing frames read in, not only
  // of newly created frames, which is really not what we want!
  switch (textEnc) {
    case TagConfig::TE_ISO8859_1:
      s_defaultTextEncoding = TagLib::String::Latin1;
      break;
    case TagConfig::TE_UTF16:
      s_defaultTextEncoding = TagLib::String::UTF16;
      break;
    case TagConfig::TE_UTF8:
    default:
      s_defaultTextEncoding = TagLib::String::UTF8;
  }
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 */
void TagLibFile::notifyConfigurationChange()
{
  const QTextCodec* id3v1TextCodec =
    TagConfig::instance().textEncodingV1() != QLatin1String("ISO-8859-1") ?
    QTextCodec::codecForName(TagConfig::instance().textEncodingV1().toLatin1().data()) : 0;
  setDefaultTextEncoding(
    static_cast<TagConfig::TextEncoding>(TagConfig::instance().textEncoding()));
  setTextCodecV1(id3v1TextCodec);
}

#if TAGLIB_VERSION < 0x010800
/**
 * Register open TagLib file, so that the number of open files can be limited.
 * If the number of open files exceeds a limit, files are closed.
 *
 * @param tagLibFile new open file to be registered
 */
void TagLibFile::registerOpenFile(TagLibFile* tagLibFile)
{
  if (s_openFiles.contains(tagLibFile))
    return;

  int numberOfFilesToClose = s_openFiles.size() - 15;
  if (numberOfFilesToClose > 5) {
    QList<TagLibFile*> filesToClose;
    for (QList<TagLibFile*>::iterator it = s_openFiles.begin();
         it != s_openFiles.end();
         ++it) {
      TagLibFile* tlf = *it;
      bool tagChanged = false;
      FOR_TAGLIB_TAGS(tagNr) {
        if (tlf->isTagChanged(tagNr)) {
          tagChanged = true;
          break;
        }
      }
      if (!tagChanged) {
        filesToClose.append(tlf);
        if (--numberOfFilesToClose <= 0) {
          break;
        }
      }
    }
    for (QList<TagLibFile*>::iterator it = filesToClose.begin();
         it != filesToClose.end();
         ++it) {
      (*it)->closeFile(false);
    }
  }
  s_openFiles.append(tagLibFile);
}

/**
 * Deregister open TagLib file.
 *
 * @param tagLibFile file which is no longer open
 */
void TagLibFile::deregisterOpenFile(TagLibFile* tagLibFile)
{
  s_openFiles.removeAll(tagLibFile);
}
#endif


/**
 * Used to register file types at static initialization time.
 */
class TagLibInitializer {
public:
  /** Constructor. */
  TagLibInitializer();

  /** Destructor. */
  ~TagLibInitializer();

  /**
   * Initialization.
   * Is deferred because it will crash on Mac OS X if done in the constructor.
   */
  void init();

private:
  Q_DISABLE_COPY(TagLibInitializer)

#if TAGLIB_VERSION <= 0x010400
  SpeexFileTypeResolver* m_speexFileTypeResolver;
  WavPackFileTypeResolver* m_wavPackFileTypeResolver;
  TTAFileTypeResolver* m_ttaFileTypeResolver;
#endif
  AACFileTypeResolver* m_aacFileTypeResolver;
  MP2FileTypeResolver* m_mp2FileTypeResolver;
  TextCodecStringHandler* m_textCodecStringHandler;
};


TagLibInitializer::TagLibInitializer() :
#if TAGLIB_VERSION <= 0x010400
  m_speexFileTypeResolver(new SpeexFileTypeResolver),
  m_wavPackFileTypeResolver(new WavPackFileTypeResolver),
  m_ttaFileTypeResolver(new TTAFileTypeResolver),
#endif
  m_aacFileTypeResolver(new AACFileTypeResolver),
  m_mp2FileTypeResolver(new MP2FileTypeResolver),
  m_textCodecStringHandler(new TextCodecStringHandler)
{
}

void TagLibInitializer::init()
{
#if TAGLIB_VERSION <= 0x010400
  TagLib::FileRef::addFileTypeResolver(m_speexFileTypeResolver);
  TagLib::FileRef::addFileTypeResolver(m_wavPackFileTypeResolver);
  TagLib::FileRef::addFileTypeResolver(m_ttaFileTypeResolver);
#endif
  TagLib::FileRef::addFileTypeResolver(m_aacFileTypeResolver);
  TagLib::FileRef::addFileTypeResolver(m_mp2FileTypeResolver);
  TagLib::ID3v1::Tag::setStringHandler(m_textCodecStringHandler);
}

TagLibInitializer::~TagLibInitializer() {
  delete m_textCodecStringHandler;
  delete m_mp2FileTypeResolver;
  delete m_aacFileTypeResolver;
#if TAGLIB_VERSION <= 0x010400
  delete m_ttaFileTypeResolver;
  delete m_wavPackFileTypeResolver;
  delete m_speexFileTypeResolver;
#endif
}

static TagLibInitializer tagLibInitializer;

/**
 * Static initialization.
 * Registers file types.
 */
void TagLibFile::staticInit()
{
  tagLibInitializer.init();
}
