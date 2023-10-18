/**
 * \file taglibfile.cpp
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 *
 * Copyright (C) 2006-2023  Urs Fleisch
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
#if QT_VERSION >= 0x060000
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <QByteArray>
#include <QVarLengthArray>
#include <QScopedPointer>
#include <QMimeDatabase>
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
#include <id3v1genres.h>
#include <id3v2tag.h>
#include <id3v2header.h>
#include <apetag.h>
#include <textidentificationframe.h>
#include <commentsframe.h>
#include <attachedpictureframe.h>
#include <uniquefileidentifierframe.h>
#include <generalencapsulatedobjectframe.h>
#include <urllinkframe.h>
#include <unsynchronizedlyricsframe.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <wavpackfile.h>
#include <oggflacfile.h>
#include <relativevolumeframe.h>
#include <mp4file.h>
#include <asffile.h>
#include <aifffile.h>
#include <wavfile.h>
#include <popularimeterframe.h>
#include <privateframe.h>
#include <apefile.h>
#include <ownershipframe.h>
#include <modfile.h>
#include <s3mfile.h>
#include <itfile.h>
#include <tfilestream.h>
#include <xmfile.h>
#include <opusfile.h>
#if TAGLIB_VERSION >= 0x020000
#include <dsffile.h>
#include <dsdifffile.h>
#else
#include "taglibext/dsf/dsffiletyperesolver.h"
#include "taglibext/dsf/dsffile.h"
#include "taglibext/dsdiff/dsdifffiletyperesolver.h"
#include "taglibext/dsdiff/dsdifffile.h"
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

/** for loop through all supported tag number values in reverse order. */
#define FOR_TAGLIB_TAGS_REVERSE(variable) \
  for (Frame::TagNumber variable = static_cast<Frame::TagNumber>(NUM_TAGS - 1); \
       variable >= Frame::Tag_1; \
       variable = static_cast<Frame::TagNumber>(variable - 1))

#if defined TAGLIB_WITH_OFFSET_TYPE || TAGLIB_VERSION >= 0x020000
typedef TagLib::offset_t taglib_offset_t;
typedef TagLib::offset_t taglib_uoffset_t;
#else
typedef long taglib_offset_t;
typedef ulong taglib_uoffset_t;
#endif

namespace {

/** Convert QString @a s to a TagLib::String. */
TagLib::String toTString(const QString& s)
{
  int len = s.length();
  QVarLengthArray<wchar_t> a(len + 1);
  wchar_t* const ws = a.data();
  // Do not use `len = s.toWCharArray(ws); ws[len] = 0;`, this would construct
  // an array with UCS-4 encoded wide characters (if not on Windows), which
  // is not compatible with TagLib, which expects only 16 bit characters.
  // This works for Basic Multilingual Plane only, but not for surrogate pairs.
  wchar_t* wsPtr = ws;
  for (auto it = s.constBegin(); it != s.constEnd(); ++it) {
    *wsPtr++ = it->unicode();
  }
  *wsPtr = 0;
  return TagLib::String(ws);
}

/** Convert TagLib::String @a s to a QString. */
inline QString toQString(const TagLib::String& s)
{
  return QString::fromWCharArray(s.toCWString(), s.size());
}

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
  virtual ~WavFile() override;

  /**
   * Replace the "ID3 " chunk with a lowercase named "id3 " chunk.
   * This method has to be called after successfully calling save() to use
   * lowercase "id3 " chunk names.
   */
  void changeToLowercaseId3Chunk();
};

/**
 * Destructor.
 */
WavFile::~WavFile()
{
  // not inline or default to silence weak-vtables warning
}

WavFile::WavFile(TagLib::IOStream *stream) : TagLib::RIFF::WAV::File(stream)
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

}

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
  virtual ~FileIOStream() override;

  FileIOStream(const FileIOStream&) = delete;
  FileIOStream& operator=(const FileIOStream&) = delete;

  /**
   * Close the file handle.
   * The file will automatically be opened again if needed.
   */
  void closeFileHandle();

  /**
   * Change the file name.
   * Can be used to modify the file name when it has changed because a path
   * component was renamed.
   * @param fileName path to file
   */
  void setName(const QString& fileName);

  // Reimplemented from TagLib::IOStream, delegate to TagLib::FileStream.
  /** File name in local file system encoding. */
  virtual TagLib::FileName name() const override;
  /** Read block of size @a length at current pointer. */
  virtual TagLib::ByteVector readBlock(ulong length) override;
  /** Write block @a data at current pointer. */
  virtual void writeBlock(const TagLib::ByteVector &data) override;
  /**
   * Insert @a data at position @a start in the file overwriting @a replace
   * bytes of the original content.
   */
  virtual void insert(const TagLib::ByteVector &data,
              taglib_uoffset_t start = 0, ulong replace = 0) override;
  /** Remove block starting at @a start for @a length bytes. */
  virtual void removeBlock(taglib_uoffset_t start = 0, ulong length = 0) override;
  /** True if the file is read only. */
  virtual bool readOnly() const override;
  /** Check if open in constructor succeeded. */
  virtual bool isOpen() const override;
  /** Move I/O pointer to @a offset in the file from position @a p. */
  virtual void seek(taglib_offset_t offset, Position p = Beginning) override;
  /** Reset the end-of-file and error flags on the file. */
  virtual void clear() override;
  /** Current offset within the file. */
  virtual taglib_offset_t tell() const override;
  /** Length of the file. */
  virtual taglib_offset_t length() override;
  /** Truncate the file to @a length. */
  virtual void truncate(taglib_offset_t length) override;

  /**
   * Create a TagLib file for a stream.
   * TagLib::FileRef::create() adapted for IOStream.
   * @param stream stream with name() of which the extension is used to deduce
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
   * Create a TagLib file for a stream.
   * @param stream stream with name() of which the extension is used to deduce
   * the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromExtension(IOStream* stream);

  /**
   * Create a TagLib file for a stream.
   * @param stream stream
   * @param ext uppercase extension used to deduce the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromExtension(TagLib::IOStream* stream,
                                           const TagLib::String& ext);

  /**
   * Create a TagLib file for a stream.
   * @param stream stream where the contents are used to deduce the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromContents(IOStream* stream);

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

FileIOStream::FileIOStream(const QString& fileName)
  : m_fileName(nullptr), m_fileStream(nullptr), m_offset(0)
{
  setName(fileName);
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
    auto self = const_cast<FileIOStream*>(this);
    self->m_fileStream =
        new TagLib::FileStream(TagLib::FileName(m_fileName));
    if (!self->m_fileStream->isOpen()) {
      delete self->m_fileStream;
      self->m_fileStream = nullptr;
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
    m_fileStream = nullptr;
    deregisterOpenFile(this);
  }
}

void FileIOStream::setName(const QString& fileName)
{
  delete m_fileName;
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
                          taglib_uoffset_t start, ulong replace)
{
  if (openFileHandle()) {
    m_fileStream->insert(data, start, replace);
  }
}

void FileIOStream::removeBlock(taglib_uoffset_t start, ulong length)
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

void FileIOStream::seek(taglib_offset_t offset, Position p)
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

taglib_offset_t FileIOStream::tell() const
{
  if (openFileHandle()) {
    return m_fileStream->tell();
  }
  return 0;
}

taglib_offset_t FileIOStream::length()
{
  if (openFileHandle()) {
    return m_fileStream->length();
  }
  return 0;
}

void FileIOStream::truncate(taglib_offset_t length)
{
  if (openFileHandle()) {
    m_fileStream->truncate(length);
  }
}

TagLib::File* FileIOStream::create(TagLib::IOStream* stream)
{
  TagLib::File* file = createFromExtension(stream);
  if (file && !file->isValid()) {
    delete file;
    file = nullptr;
  }
  if (!file) {
    file = createFromContents(stream);
  }
  return file;
}

TagLib::File* FileIOStream::createFromExtension(TagLib::IOStream* stream)
{
#ifdef Q_OS_WIN32
  TagLib::String fn = stream->name().toString();
#else
  TagLib::String fn = stream->name();
#endif
  const int extPos = fn.rfind(".");
  return extPos != -1
      ? createFromExtension(stream, fn.substr(extPos + 1).upper())
      : nullptr;
}

TagLib::File* FileIOStream::createFromExtension(TagLib::IOStream* stream,
                                                const TagLib::String& ext)
{
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
  if (ext == "OPUS")
    return new TagLib::Ogg::Opus::File(stream);
  if (ext == "TTA")
    return new TagLib::TrueAudio::File(stream);
  if (ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" ||
      ext == "M4R" || ext == "MP4" || ext == "3G2" || ext == "M4V" ||
      ext == "MP4V")
    return new TagLib::MP4::File(stream);
  if (ext == "WMA" || ext == "ASF" || ext == "WMV")
    return new TagLib::ASF::File(stream);
  if (ext == "AIF" || ext == "AIFF")
    return new TagLib::RIFF::AIFF::File(stream);
  if (ext == "WAV")
    return new WavFile(stream);
  if (ext == "APE")
    return new TagLib::APE::File(stream);
  if (ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
    return new TagLib::Mod::File(stream);
  if (ext == "S3M")
    return new TagLib::S3M::File(stream);
  if (ext == "IT")
    return new TagLib::IT::File(stream);
  if (ext == "XM")
    return new TagLib::XM::File(stream);
  if (ext == "DSF")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::DSF::File(stream);
#else
    return new DSFFile(stream, TagLib::ID3v2::FrameFactory::instance());
#endif
  if (ext == "DFF")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::DSDIFF::File(stream);
#else
    return new DSDIFFFile(stream, TagLib::ID3v2::FrameFactory::instance());
#endif
  return nullptr;
}

TagLib::File* FileIOStream::createFromContents(TagLib::IOStream* stream)
{
  static const struct ExtensionForMimeType {
    const char* mime;
    const char* ext;
  } extensionForMimeType[] = {
    { "application/ogg", "OGG" },
    { "application/vnd.ms-asf", "WMA" },
    { "audio/aac", "AAC" },
    { "audio/flac", "FLAC" },
    { "audio/mp4", "MP4" },
    { "audio/mpeg", "MP3" },
    { "audio/x-aiff", "AIFF" },
    { "audio/x-ape", "APE" },
    { "audio/x-flac+ogg", "OGG" },
    { "audio/x-it", "IT" },
    { "audio/x-musepack", "MPC" },
    { "audio/x-opus+ogg", "OPUS" },
    { "audio/x-s3m", "S3M" },
    { "audio/x-speex+ogg", "SPX" },
    { "audio/x-tta", "TTA" },
    { "audio/x-vorbis+ogg", "OGG" },
    { "audio/x-wav", "WAV" },
    { "audio/x-wavpack", "WV" },
    { "audio/x-xm", "XM" },
    { "video/mp4", "MP4" }
};

  static QMap<QString, TagLib::String> mimeExtMap;
  if (mimeExtMap.empty()) {
    // first time initialization
    for (const auto& efm : extensionForMimeType) {
      mimeExtMap.insert(QString::fromLatin1(efm.mime), efm.ext);
    }
  }

  stream->seek(0);
  TagLib::ByteVector bv = stream->readBlock(4096);
  stream->seek(0);
  QMimeDatabase mimeDb;
  auto mimeType =
      mimeDb.mimeTypeForData(QByteArray(bv.data(), static_cast<int>(bv.size())));
  TagLib::String ext = mimeExtMap.value(mimeType.name());
  if (!ext.isEmpty()) {
    return createFromExtension(stream, ext);
  }
  return nullptr;
}

void FileIOStream::registerOpenFile(FileIOStream* stream)
{
  if (s_openFiles.contains(stream))
    return;

  int numberOfFilesToClose = s_openFiles.size() - 15;
  if (numberOfFilesToClose > 5) {
    for (auto it = s_openFiles.begin(); it != s_openFiles.end(); ++it) { // clazy:exclude=detaching-member
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

namespace {

/**
 * Data encoding in ID3v1 tags.
 */
class TextCodecStringHandler : public TagLib::ID3v1::StringHandler {
public:
  /**
   * Constructor.
   */
  TextCodecStringHandler() = default;

  /**
   * Destructor.
   */
  virtual ~TextCodecStringHandler() = default;

  TextCodecStringHandler(const TextCodecStringHandler&) = delete;
  TextCodecStringHandler& operator=(const TextCodecStringHandler&) = delete;

  /**
   * Decode a string from data.
   *
   * @param data data to decode
   */
  virtual TagLib::String parse(const TagLib::ByteVector& data) const override;

  /**
   * Encode a byte vector with the data from a string.
   *
   * @param s string to encode
   */
  virtual TagLib::ByteVector render(const TagLib::String& s) const override;

#if QT_VERSION >= 0x060000
  /**
   * Set string decoder.
   * @param encodingName encoding, empty for default behavior (ISO 8859-1)
   */
  static void setStringDecoder(const QString& encodingName) {
    if (auto encoding = QStringConverter::encodingForName(encodingName.toLatin1())) {
      s_encoder = QStringEncoder(*encoding);
      s_decoder = QStringDecoder(*encoding);
    } else {
      s_encoder = QStringEncoder();
      s_decoder = QStringDecoder();
    }
  }
#else
  /**
   * Set text codec.
   * @param codec text codec, 0 for default behavior (ISO 8859-1)
   */
  static void setTextCodec(const QTextCodec* codec) { s_codec = codec; }
#endif

private:
#if QT_VERSION >= 0x060000
  static QStringDecoder s_decoder;
  static QStringEncoder s_encoder;
#else
  static const QTextCodec* s_codec;
#endif
};

#if QT_VERSION >= 0x060000
QStringDecoder TextCodecStringHandler::s_decoder;
QStringEncoder TextCodecStringHandler::s_encoder;
#else
const QTextCodec* TextCodecStringHandler::s_codec = nullptr;
#endif

/**
 * Decode a string from data.
 *
 * @param data data to decode
 */
TagLib::String TextCodecStringHandler::parse(const TagLib::ByteVector& data) const
{
#if QT_VERSION >= 0x060000
  return s_decoder.isValid()
      ? toTString(s_decoder(QByteArray(data.data(), data.size()))).stripWhiteSpace()
      : TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
#else
  return s_codec
      ? toTString(s_codec->toUnicode(data.data(), data.size())).stripWhiteSpace()
      : TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
#endif
}

/**
 * Encode a byte vector with the data from a string.
 *
 * @param s string to encode
 */
TagLib::ByteVector TextCodecStringHandler::render(const TagLib::String& s) const
{
#if QT_VERSION >= 0x060000
  if (s_encoder.isValid()) {
    QByteArray ba = s_encoder(toQString(s));
    return TagLib::ByteVector(ba.data(), ba.size());
  } else {
    return s.data(TagLib::String::Latin1);
  }
#else
  if (s_codec) {
    QByteArray ba(s_codec->fromUnicode(toQString(s)));
    return TagLib::ByteVector(ba.data(), ba.size());
  } else {
    return s.data(TagLib::String::Latin1);
  }
#endif
}

}

/** Default text encoding */
TagLib::String::Type TagLibFile::s_defaultTextEncoding = TagLib::String::Latin1;


/**
 * Constructor.
 *
 * @param idx index in tagged file system model
 */
TagLibFile::TagLibFile(const QPersistentModelIndex& idx)
  : TaggedFile(idx),
    m_tagInformationRead(false), m_fileRead(false),
    m_stream(nullptr),
    m_id3v2Version(0),
    m_activatedFeatures(0), m_duration(0)
{
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_isTagSupported[tagNr] = tagNr == Frame::Tag_2;
    m_tag[tagNr] = nullptr;
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
      TF_OggFlac |
      TF_OggPictures |
      TF_ID3v23 |
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
  m_pictures.clear();
  m_pictures.setRead(false);
  m_tagInformationRead = false;
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_tagFormat[tagNr].clear();
    m_tagType[tagNr] = TT_Unknown;
  }
  FOR_TAGLIB_TAGS(tagNr) {
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

  if (force || m_fileRef.isNull()) {
    delete m_stream;
    m_stream = new FileIOStream(fileName);
    m_fileRef = TagLib::FileRef(FileIOStream::create(m_stream));
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = nullptr;
    }
    FOR_TAGLIB_TAGS(tagNr) {
      markTagUnchanged(tagNr);
    }
    m_fileRead = true;

    m_pictures.clear();
    m_pictures.setRead(false);
  }

  TagLib::File* file;
  if (!m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
    TagLib::MPEG::File* mpegFile;
    TagLib::FLAC::File* flacFile;
#if TAGLIB_VERSION >= 0x010b00
    TagLib::MPC::File* mpcFile;
    TagLib::WavPack::File* wvFile;
#endif
    TagLib::TrueAudio::File* ttaFile;
    TagLib::RIFF::WAV::File* wavFile;
#if TAGLIB_VERSION >= 0x020000
    TagLib::DSF::File* dsfFile;
#else
    DSFFile* dsfFile;
#endif
#if TAGLIB_VERSION >= 0x020000
    TagLib::DSDIFF::File* dffFile;
#else
    DSDIFFFile* dffFile;
#endif
    TagLib::APE::File* apeFile;
    m_fileExtension = QLatin1String(".mp3");
    m_isTagSupported[Frame::Tag_1] = false;
    if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != nullptr) {
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
        setId3v2VersionFromTag(id3v2Tag);
        m_tag[Frame::Tag_2] = id3v2Tag;
        markTagUnchanged(Frame::Tag_2);
      }
      if (!m_tag[Frame::Tag_3]) {
        m_tag[Frame::Tag_3] = mpegFile->APETag();
        markTagUnchanged(Frame::Tag_3);
      }
    } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != nullptr) {
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
      if (!m_pictures.isRead()) {
        const TagLib::List<TagLib::FLAC::Picture*> pics(flacFile->pictureList());
        int i = 0;
        for (auto it = pics.begin(); it != pics.end(); ++it) {
          PictureFrame frame;
          flacPictureToFrame(*it, frame);
          frame.setIndex(Frame::toNegativeIndex(i++));
          m_pictures.append(frame);
        }
        m_pictures.setRead(true);
      }
#if TAGLIB_VERSION >= 0x010b00
    } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != nullptr) {
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
    } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != nullptr) {
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
    } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != nullptr) {
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
    } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != nullptr) {
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
    } else if ((wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) != nullptr) {
      m_fileExtension = QLatin1String(".wav");
      m_tag[Frame::Tag_1] = nullptr;
      markTagUnchanged(Frame::Tag_1);
#if TAGLIB_VERSION >= 0x010a00
      m_isTagSupported[Frame::Tag_3] = true;
      if (!m_tag[Frame::Tag_2]) {
        TagLib::ID3v2::Tag* id3v2Tag = wavFile->ID3v2Tag();
        setId3v2VersionFromTag(id3v2Tag);
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
#if TAGLIB_VERSION >= 0x020000
    } else if ((dsfFile = dynamic_cast<TagLib::DSF::File*>(file)) != nullptr) {
#else
    } else if ((dsfFile = dynamic_cast<DSFFile*>(file)) != nullptr) {
#endif
      m_fileExtension = QLatin1String(".dsf");
      m_tag[Frame::Tag_1] = nullptr;
      markTagUnchanged(Frame::Tag_1);
      if (!m_tag[Frame::Tag_2]) {
#if TAGLIB_VERSION >= 0x020000
        TagLib::ID3v2::Tag* id3v2Tag = dsfFile->tag();
#else
        TagLib::ID3v2::Tag* id3v2Tag = dsfFile->ID3v2Tag();
#endif
        setId3v2VersionFromTag(id3v2Tag);
        m_tag[Frame::Tag_2] = id3v2Tag;
        markTagUnchanged(Frame::Tag_2);
      }
#if TAGLIB_VERSION >= 0x020000
    } else if ((dffFile = dynamic_cast<TagLib::DSDIFF::File*>(file)) != nullptr) {
#else
    } else if ((dffFile = dynamic_cast<DSDIFFFile*>(file)) != nullptr) {
#endif
      m_fileExtension = QLatin1String(".dff");
      m_tag[Frame::Tag_1] = nullptr;
      markTagUnchanged(Frame::Tag_1);
      if (!m_tag[Frame::Tag_2]) {
        TagLib::ID3v2::Tag* id3v2Tag = dffFile->ID3v2Tag();
        setId3v2VersionFromTag(id3v2Tag);
        m_tag[Frame::Tag_2] = id3v2Tag;
        markTagUnchanged(Frame::Tag_2);
      }
    } else {
      if (dynamic_cast<TagLib::Vorbis::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".ogg");
      } else if (dynamic_cast<TagLib::Ogg::Speex::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".spx");
#if TAGLIB_VERSION < 0x010b00
      } else if (dynamic_cast<TagLib::MPC::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".mpc");
      } else if (dynamic_cast<TagLib::WavPack::File*>(file) != 0) {
        m_fileExtension = QLatin1String(".wv");
#endif
      } else if (dynamic_cast<TagLib::MP4::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".m4a");
      } else if (dynamic_cast<TagLib::ASF::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".wma");
      } else if (dynamic_cast<TagLib::RIFF::AIFF::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".aiff");
      } else if (dynamic_cast<TagLib::Mod::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".mod");
      } else if (dynamic_cast<TagLib::S3M::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".s3m");
      } else if (dynamic_cast<TagLib::IT::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".it");
      } else if (dynamic_cast<TagLib::XM::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".xm");
      } else if (dynamic_cast<TagLib::Ogg::Opus::File*>(file) != nullptr) {
        m_fileExtension = QLatin1String(".opus");
      }
      m_tag[Frame::Tag_1] = nullptr;
      markTagUnchanged(Frame::Tag_1);
      if (!m_tag[Frame::Tag_2]) {
        m_tag[Frame::Tag_2] = m_fileRef.tag();
        markTagUnchanged(Frame::Tag_2);
      }
      if (!m_pictures.isRead()) {
#if TAGLIB_VERSION >= 0x010b00
        if (auto xiphComment =
            dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[Frame::Tag_2])) {
          const TagLib::List<TagLib::FLAC::Picture*> pics(xiphComment->pictureList());
          int i = 0;
          for (auto it = pics.begin(); it != pics.end(); ++it) {
            PictureFrame frame;
            flacPictureToFrame(*it, frame);
            frame.setIndex(Frame::toNegativeIndex(i++));
            m_pictures.append(frame);
          }
          m_pictures.setRead(true);
        } else
#endif
        if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[Frame::Tag_2])) {
#if TAGLIB_VERSION >= 0x010a00
          const auto& itemMap = mp4Tag->itemMap();
          auto it = itemMap.find("covr");
          const TagLib::MP4::CoverArtList pics = it != itemMap.end()
              ? it->second.toCoverArtList() : TagLib::MP4::CoverArtList();
#else
          const TagLib::MP4::CoverArtList pics(mp4Tag->itemListMap()["covr"].toCoverArtList());
#endif
          int i = 0;
          for (auto it = pics.begin(); it != pics.end(); ++it) {
            const TagLib::MP4::CoverArt& coverArt = *it;
            TagLib::ByteVector bv = coverArt.data();
            QString mimeType, imgFormat;
            switch (coverArt.format()) {
            case TagLib::MP4::CoverArt::PNG:
              mimeType = QLatin1String("image/png");
              imgFormat = QLatin1String("PNG");
              break;
            case TagLib::MP4::CoverArt::BMP:
              mimeType = QLatin1String("image/bmp");
              imgFormat = QLatin1String("BMP");
              break;
            case TagLib::MP4::CoverArt::GIF:
              mimeType = QLatin1String("image/gif");
              imgFormat = QLatin1String("GIF");
              break;
            case TagLib::MP4::CoverArt::JPEG:
            case TagLib::MP4::CoverArt::Unknown:
            default:
              mimeType = QLatin1String("image/jpeg");
              imgFormat = QLatin1String("JPG");
            }
            PictureFrame frame(
                  QByteArray(bv.data(), static_cast<int>(bv.size())),
                  QLatin1String(""), PictureFrame::PT_CoverFront, mimeType,
                  Frame::TE_ISO8859_1, imgFormat);
            frame.setIndex(Frame::toNegativeIndex(i++));
            frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture,
                                                      QLatin1String("covr")));
            m_pictures.append(frame);
          }
          m_pictures.setRead(true);
        }
      }
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

  closeFile(false);

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
  if (force) {
    m_fileRef = TagLib::FileRef();
    delete m_stream;
    m_stream = nullptr;
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = nullptr;
    }
    m_fileRead = false;
  } else if (m_stream) {
    m_stream->closeFileHandle();
  }
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
    closeFile(false);
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
  if (!m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
    if (m_stream) {
#ifndef Q_OS_WIN32
      QString fileName = QFile::decodeName(m_stream->name());
#else
      QString fileName = toQString(m_stream->name().toString());
#endif
      if (fnStr != fileName) {
        qDebug("TagLibFile: Fix file name mismatch, should be '%s', not '%s'",
               qPrintable(fnStr), qPrintable(fileName));
        m_stream->setName(fnStr);
      }
    }
    auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file);
    if (mpegFile) {
      static const int tagTypes[NUM_TAGS] = {
        TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
        TagLib::MPEG::File::APE
      };
      int saveMask = 0;
      // We iterate through the tags in reverse order to work around
      // a TagLib bug: When stripping the APE tag after the ID3v1 tag,
      // the ID3v1 tag is not removed.
      FOR_TAGLIB_TAGS_REVERSE(tagNr) {
        if (m_tag[tagNr] && (force || isTagChanged(tagNr))) {
          if (m_tag[tagNr]->isEmpty()) {
            mpegFile->strip(tagTypes[tagNr]);
            fileChanged = true;
            m_tag[tagNr] = nullptr;
            markTagUnchanged(tagNr);
          } else {
            saveMask |= tagTypes[tagNr];
          }
        }
      }
      if (saveMask != 0) {
        setId3v2VersionOrDefault(id3v2Version);
        if (
#if TAGLIB_VERSION >= 0x010c00
            mpegFile->save(
              saveMask, TagLib::File::StripNone,
              m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3,
              TagLib::File::DoNotDuplicate)
#else
            mpegFile->save(saveMask, false, m_id3v2Version, false)
#endif
            ) {
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
        if (auto ttaFile =
            dynamic_cast<TagLib::TrueAudio::File*>(file)) {
          static const int tagTypes[NUM_TAGS] = {
            TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
            TagLib::MPEG::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) && m_tag[tagNr]->isEmpty()) {
              ttaFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              m_tag[tagNr] = nullptr;
              markTagUnchanged(tagNr);
            }
          }
        } else if (auto mpcFile =
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
              m_tag[tagNr] = nullptr;
              markTagUnchanged(tagNr);
            }
          }
#else
          // it does not work if there is also an ID3 tag (bug in TagLib)
          mpcFile->remove(TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2);
          fileChanged = true;
#endif
        } else if (auto wvFile =
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
              m_tag[tagNr] = nullptr;
              markTagUnchanged(tagNr);
            }
          }
#else
          // it does not work if there is also an ID3 tag (bug in TagLib)
          wvFile->strip(TagLib::WavPack::File::ID3v1);
          fileChanged = true;
#endif
        }
        else if (auto apeFile =
                 dynamic_cast<TagLib::APE::File*>(file)) {
          static const int tagTypes[NUM_TAGS] = {
            TagLib::MPEG::File::ID3v1, TagLib::APE::File::APE,
            TagLib::APE::File::NoTags
          };
          FOR_TAGLIB_TAGS(tagNr) {
            if (m_tag[tagNr] && (force || isTagChanged(tagNr)) && m_tag[tagNr]->isEmpty()) {
              apeFile->strip(tagTypes[tagNr]);
              fileChanged = true;
              m_tag[tagNr] = nullptr;
              markTagUnchanged(tagNr);
            }
          }
        }
        else if (auto flacFile =
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
              m_tag[tagNr] = nullptr;
              markTagUnchanged(tagNr);
            }
          }
#endif
          flacFile->removePictures();
          const auto frames = m_pictures;
          for (const Frame& frame : frames) {
            auto pic = new TagLib::FLAC::Picture;
            frameToFlacPicture(frame, pic);
            flacFile->addPicture(pic);
          }
        }
        else if (auto wavFile = dynamic_cast<WavFile*>(file)) {
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
              m_tag[tagNr] = nullptr;
            } else {
              saveTags |= tagTypes[tagNr];
            }
          }
          setId3v2VersionOrDefault(id3v2Version);
          if (
#if TAGLIB_VERSION >= 0x010c00
              wavFile->save(
                static_cast<TagLib::RIFF::WAV::File::TagTypes>(saveTags),
                TagLib::File::StripOthers,
                m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3)
#else
              wavFile->save(static_cast<TagLib::RIFF::WAV::File::TagTypes>(
                              saveTags), true, m_id3v2Version)
#endif
              ) {
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
#if TAGLIB_VERSION >= 0x020000
        else if (auto dsfFile = dynamic_cast<TagLib::DSF::File*>(file)) {
          setId3v2VersionOrDefault(id3v2Version);
          if (dsfFile->save(m_id3v2Version == 4 ? TagLib::ID3v2::v4
                                                : TagLib::ID3v2::v3)) {
#else
        else if (auto dsfFile = dynamic_cast<DSFFile*>(file)) {
          setId3v2VersionOrDefault(id3v2Version);
          if (dsfFile->save(m_id3v2Version)) {
#endif
            fileChanged = true;
            FOR_TAGLIB_TAGS(tagNr) {
              markTagUnchanged(tagNr);
            }
            needsSave = false;
          }
        }
#if TAGLIB_VERSION >= 0x020000
        else if (auto dffFile = dynamic_cast<TagLib::DSDIFF::File*>(file)) {
          int saveMask = 0;
          if (m_tag[Frame::Tag_2] && (force || isTagChanged(Frame::Tag_2))) {
            if (m_tag[Frame::Tag_2]->isEmpty()) {
              dffFile->strip(TagLib::DSDIFF::File::ID3v2);
              fileChanged = true;
              m_tag[Frame::Tag_2] = nullptr;
              markTagUnchanged(Frame::Tag_2);
              needsSave = false;
            } else {
              saveMask = TagLib::DSDIFF::File::ID3v2;
            }
          }
          setId3v2VersionOrDefault(id3v2Version);
          if (saveMask != 0 && dffFile->save(saveMask,
                            TagLib::File::StripNone,
                            m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3)) {
#else
        else if (auto dffFile = dynamic_cast<DSDIFFFile*>(file)) {
          setId3v2VersionOrDefault(id3v2Version);
          if (dffFile->save(m_id3v2Version)) {
#endif
            fileChanged = true;
            FOR_TAGLIB_TAGS(tagNr) {
              markTagUnchanged(tagNr);
            }
            needsSave = false;
          }
        }
#if TAGLIB_VERSION >= 0x010b00
        else if (auto xiphComment =
                 dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[Frame::Tag_2])) {
          xiphComment->removeAllPictures();
          const auto frames = m_pictures;
          for (const Frame& frame : frames) {
            auto pic = new TagLib::FLAC::Picture;
            frameToFlacPicture(frame, pic);
            xiphComment->addPicture(pic);
          }
        }
#endif
        else if (auto mp4Tag =
                 dynamic_cast<TagLib::MP4::Tag*>(m_tag[Frame::Tag_2])) {
          if (!m_pictures.isEmpty()) {
            TagLib::MP4::CoverArtList coverArtList;
            const auto frames = m_pictures;
            for (const Frame& frame : frames) {
              QByteArray ba;
              TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
              if (PictureFrame::getData(frame, ba)) {
                QString mimeType;
                if (PictureFrame::getMimeType(frame, mimeType)) {
                  if (mimeType == QLatin1String("image/png")) {
                    format = TagLib::MP4::CoverArt::PNG;
                  } else if (mimeType == QLatin1String("image/bmp")) {
                    format = TagLib::MP4::CoverArt::BMP;
                  } else if (mimeType == QLatin1String("image/gif")) {
                    format = TagLib::MP4::CoverArt::GIF;
                  }
                }
              }
              coverArtList.append(TagLib::MP4::CoverArt(
                          format,
                          TagLib::ByteVector(
                            ba.data(), static_cast<unsigned int>(ba.size()))));
            }
#if TAGLIB_VERSION >= 0x010a00
            mp4Tag->setItem("covr", coverArtList);
#else
            mp4Tag->itemListMap()["covr"] = coverArtList;
#endif
          } else {
#if TAGLIB_VERSION >= 0x010a00
            mp4Tag->removeItem("covr");
#else
            mp4Tag->itemListMap().erase("covr");
#endif
          }
#if TAGLIB_VERSION >= 0x010d00
          TagLib::MP4::File* mp4File;
          if ((force || isTagChanged(Frame::Tag_2)) && mp4Tag->isEmpty() &&
              (mp4File = dynamic_cast<TagLib::MP4::File*>(file)) != nullptr) {
            mp4File->strip();
            fileChanged = true;
            m_tag[Frame::Tag_2] = nullptr;
            markTagUnchanged(Frame::Tag_2);
            needsSave = false;
          }
#endif
        }
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
  closeFile(fileChanged);
#else
  closeFile(true);
#endif

  // restore time stamp
  if (actime || modtime) {
    setFileTimeStamps(fnStr, actime, modtime);
  }

  if (isFilenameChanged()) {
    if (!renameFile()) {
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

namespace {

/**
 * Get a genre string from a string which can contain the genre itself,
 * or only the genre number or the genre number in parenthesis.
 *
 * @param str genre string
 *
 * @return genre.
 */
QString getGenreString(const TagLib::String& str)
{
#if TAGLIB_VERSION < 0x010b01
  if (str.isNull()) {
    return QLatin1String("");
  }
#endif
  QString qs = toQString(str);
  int cpPos = 0, n = 0xff;
  bool ok = false;
  if (!qs.isEmpty() && qs[0] == QLatin1Char('(') &&
      (cpPos = qs.indexOf(QLatin1Char(')'), 2)) > 1) {
#if QT_VERSION >= 0x060000
    n = qs.mid(1, cpPos - 1).toInt(&ok);
#else
    n = qs.midRef(1, cpPos - 1).toInt(&ok);
#endif
    if (!ok || n > 0xff) {
      n = 0xff;
    }
    return QString::fromLatin1(Genres::getName(n));
  } else if ((n = qs.toInt(&ok)) >= 0 && n <= 0xff && ok) {
    return QString::fromLatin1(Genres::getName(n));
  } else {
    return qs;
  }
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
    if (!m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
      TagLib::MPEG::File* mpegFile;
      TagLib::FLAC::File* flacFile;
      TagLib::MPC::File* mpcFile;
      TagLib::WavPack::File* wvFile;
      TagLib::TrueAudio::File* ttaFile;
      TagLib::APE::File* apeFile;
      if (tagNr == Frame::Tag_1) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != nullptr) {
          m_tag[tagNr] = mpegFile->ID3v1Tag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != nullptr) {
          m_tag[tagNr] = flacFile->ID3v1Tag(true);
#if TAGLIB_VERSION >= 0x010b00
        } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != nullptr) {
          m_tag[tagNr] = mpcFile->ID3v1Tag(true);
        } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != nullptr) {
          m_tag[tagNr] = wvFile->ID3v1Tag(true);
#endif
        } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != nullptr) {
          m_tag[tagNr] = ttaFile->ID3v1Tag(true);
        } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != nullptr) {
          m_tag[tagNr] = apeFile->ID3v1Tag(true);
        }
      } else if (tagNr == Frame::Tag_2) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != nullptr) {
          m_tag[tagNr] = mpegFile->ID3v2Tag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != nullptr) {
          m_tag[tagNr] = flacFile->xiphComment(true);
        } else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != nullptr) {
          m_tag[tagNr] = mpcFile->APETag(true);
        } else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != nullptr) {
          m_tag[tagNr] = wvFile->APETag(true);
        } else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != nullptr) {
          m_tag[tagNr] = ttaFile->ID3v2Tag(true);
        } else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != nullptr) {
          m_tag[tagNr] = apeFile->APETag(true);
        } else if (auto wavFile =
                   dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
          m_tag[tagNr] = wavFile->ID3v2Tag();
        }
      } else if (tagNr == Frame::Tag_3) {
        if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != nullptr) {
          m_tag[tagNr] = mpegFile->APETag(true);
        } else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != nullptr) {
          m_tag[tagNr] = flacFile->ID3v2Tag(true);
#if TAGLIB_VERSION >= 0x010a00
        } else if (auto wavFile =
                   dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
          m_tag[tagNr] = wavFile->InfoTag();
#endif
        }
      }
    }
  }
  return m_tag[tagNr] != nullptr;
}

namespace {

/**
 * Check if string needs Unicode encoding.
 *
 * @return true if Unicode needed,
 *         false if Latin-1 sufficient.
 */
bool needsUnicode(const QString& qstr)
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
TagLib::String::Type getTextEncodingConfig(bool unicode)
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
void removeCommentFrame(TagLib::ID3v2::Tag* id3v2Tag)
{
  const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList("COMM");
  for (auto it = frameList.begin();
       it != frameList.end();
       ++it) {
    auto id3Frame =
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
bool setId3v2Unicode(TagLib::Tag* tag, const QString& qstr,
                     const TagLib::String& tstr, const char* frameId)
{
  TagLib::ID3v2::Tag* id3v2Tag;
  if (tag && (id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) != nullptr) {
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
          auto commFrame =
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
      (audioProperties = m_fileRef.audioProperties()) != nullptr) {
    TagLib::MPEG::Properties* mpegProperties;
    TagLib::Ogg::Speex::Properties* speexProperties;
    TagLib::TrueAudio::Properties* ttaProperties;
    TagLib::WavPack::Properties* wvProperties;
    TagLib::APE::Properties* apeProperties;
    TagLib::Mod::Properties* modProperties;
    TagLib::S3M::Properties* s3mProperties;
    TagLib::IT::Properties* itProperties;
    TagLib::XM::Properties* xmProperties;
    TagLib::Ogg::Opus::Properties* opusProperties;
#if TAGLIB_VERSION >= 0x020000
    TagLib::DSF::Properties* dsfProperties;
    TagLib::DSDIFF::Properties* dffProperties;
#else
    DSFProperties* dsfProperties;
    DSDIFFProperties* dffProperties;
#endif
    m_detailInfo.valid = true;
    if ((mpegProperties =
         dynamic_cast<TagLib::MPEG::Properties*>(audioProperties)) != nullptr) {
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
               nullptr) {
      m_detailInfo.format = QLatin1String("Ogg Vorbis");
    } else if (TagLib::FLAC::Properties* flacProperties =
                dynamic_cast<TagLib::FLAC::Properties*>(audioProperties)) {
      m_detailInfo.format = QLatin1String("FLAC");
#if TAGLIB_VERSION >= 0x010a00
      int bits = flacProperties->bitsPerSample();
      if (bits > 0) {
        m_detailInfo.format += QLatin1Char(' ');
        m_detailInfo.format += QString::number(bits);
        m_detailInfo.format += QLatin1String(" bit");
      }
#endif
    } else if (dynamic_cast<TagLib::MPC::Properties*>(audioProperties) != nullptr) {
      m_detailInfo.format = QLatin1String("MPC");
    } else if ((speexProperties =
                dynamic_cast<TagLib::Ogg::Speex::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("Speex %1")).arg(speexProperties->speexVersion());
    } else if ((ttaProperties =
                dynamic_cast<TagLib::TrueAudio::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QLatin1String("True Audio ");
      m_detailInfo.format += QString::number(ttaProperties->ttaVersion());
      m_detailInfo.format += QLatin1Char(' ');
      m_detailInfo.format += QString::number(ttaProperties->bitsPerSample());
      m_detailInfo.format += QLatin1String(" bit");
    } else if ((wvProperties =
                dynamic_cast<TagLib::WavPack::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QLatin1String("WavPack ");
      m_detailInfo.format += QString::number(wvProperties->version(), 16);
      m_detailInfo.format += QLatin1Char(' ');
      m_detailInfo.format += QString::number(wvProperties->bitsPerSample());
      m_detailInfo.format += QLatin1String(" bit");
    } else if (TagLib::MP4::Properties* mp4Properties =
                dynamic_cast<TagLib::MP4::Properties*>(audioProperties)) {
      m_detailInfo.format = QLatin1String("MP4");
#if TAGLIB_VERSION >= 0x010a00
      switch (mp4Properties->codec()) {
      case TagLib::MP4::Properties::AAC:
        m_detailInfo.format += QLatin1String(" AAC");
        break;
      case TagLib::MP4::Properties::ALAC:
        m_detailInfo.format += QLatin1String(" ALAC");
        break;
      case TagLib::MP4::Properties::Unknown:
        ;
      }
      int bits = mp4Properties->bitsPerSample();
      if (bits > 0) {
        m_detailInfo.format += QLatin1Char(' ');
        m_detailInfo.format += QString::number(bits);
        m_detailInfo.format += QLatin1String(" bit");
      }
#endif
    } else if (dynamic_cast<TagLib::ASF::Properties*>(audioProperties) != nullptr) {
      m_detailInfo.format = QLatin1String("ASF");
    } else if (TagLib::RIFF::AIFF::Properties* aiffProperties =
               dynamic_cast<TagLib::RIFF::AIFF::Properties*>(audioProperties)) {
      m_detailInfo.format = QLatin1String("AIFF");
#if TAGLIB_VERSION >= 0x010a00
      int bits = aiffProperties->bitsPerSample();
      if (bits > 0) {
        m_detailInfo.format += QLatin1Char(' ');
        m_detailInfo.format += QString::number(bits);
        m_detailInfo.format += QLatin1String(" bit");
      }
#endif
    } else if (TagLib::RIFF::WAV::Properties* wavProperties =
               dynamic_cast<TagLib::RIFF::WAV::Properties*>(audioProperties)) {
      m_detailInfo.format = QLatin1String("WAV");
#if TAGLIB_VERSION >= 0x010a00
      int format = wavProperties->format();
      if (format > 0) {
        // https://tools.ietf.org/html/rfc2361#appendix-A
        static const struct {
          int code;
          const char* name;
        } codeToName[] = {
        {0x0001, "PCM"}, {0x0002, "ADPCM"}, {0x003, "IEEE Float"},
        {0x0004, "VSELP"}, {0x0005, "IBM CVSD"}, {0x0006, "ALAW"},
        {0x0007, "MULAW"}, {0x0010, "OKI ADPCM"}, {0x0011, "DVI ADPCM"},
        {0x0012, "MediaSpace ADPCM"}, {0x0013, "Sierra ADPCM"},
        {0x0014, "G.723 ADPCM"}, {0x0015, "DIGISTD"}, {0x0016, "DIGIFIX"},
        {0x0017, "OKI ADPCM"}, {0x0018, "MediaVision ADPCM"}, {0x0019, "CU"},
        {0x0020, "Yamaha ADPCM"}, {0x0021, "Sonarc"}, {0x0022, "True Speech"},
        {0x0023, "EchoSC1"}, {0x0024, "AF36"}, {0x0025, "APTX"},
        {0x0026, "AF10"}, {0x0027, "Prosody 1612"}, {0x0028, "LRC"},
        {0x0030, "Dolby AC2"}, {0x0031, "GSM610"}, {0x0032, "MSNAudio"},
        {0x0033, "Antex ADPCME"}, {0x0034, "Control Res VQLPC"}, {0x0035, "Digireal"},
        {0x0036, "DigiADPCM"}, {0x0037, "Control Res CR10"}, {0x0038, "NMS VBXADPCM"},
        {0x0039, "Roland RDAC"}, {0x003a, "EchoSC3"}, {0x003b, "Rockwell ADPCM"},
        {0x003c, "Rockwell DIGITALK"}, {0x003d, "Xebec"}, {0x0040, "G.721 ADPCM"},
        {0x0041, "G.728 CELP"}, {0x0042, "MSG723"}, {0x0050, "MPEG"},
        {0x0052, "RT24"}, {0x0053, "PAC"}, {0x0055, "MPEG Layer 3"},
        {0x0059, "Lucent G.723"}, {0x0060, "Cirrus"}, {0x0061, "ESPCM"},
        {0x0062, "Voxware"}, {0x0063, "Canopus Atrac"}, {0x0064, "G.726 ADPCM"},
        {0x0065, "G.722 ADPCM"}, {0x0066, "DSAT"}, {0x0067, "DSAT Display"},
        {0x0069, "Voxware Byte Aligned"}, {0x0070, "Voxware AC8"}, {0x0071, "Voxware AC10"},
        {0x0072, "Voxware AC16"}, {0x0073, "Voxware AC20"}, {0x0074, "Voxware MetaVoice"},
        {0x0075, "Voxware MetaSound"}, {0x0076, "Voxware RT29HW"}, {0x0077, "Voxware VR12"},
        {0x0078, "Voxware VR18"}, {0x0079, "Voxware TQ40"}, {0x0080, "Softsound"},
        {0x0081, "Voxware TQ60"}, {0x0082, "MSRT24"}, {0x0083, "G.729A"},
        {0x0084, "MVI MV12"}, {0x0085, "DF G.726"}, {0x0086, "DF GSM610"},
        {0x0088, "ISIAudio"}, {0x0089, "Onlive"}, {0x0091, "SBC24"},
        {0x0092, "Dolby AC3 SPDIF"}, {0x0097, "ZyXEL ADPCM"}, {0x0098, "Philips LPCBB"},
        {0x0099, "Packed"}, {0x0100, "Rhetorex ADPCM"}, {0x0101, "IRAT"},
        {0x0111, "Vivo G.723"}, {0x0112, "Vivo Siren"}, {0x0123, "Digital G.723"},
        {0x0200, "Creative ADPCM"}, {0x0202, "Creative FastSpeech8"}, {0x0203, "Creative FastSpeech10"},
        {0x0220, "Quarterdeck"}, {0x0300, "FM Towns Snd"}, {0x0400, "BTV Digital"},
        {0x0680, "VME VMPCM"}, {0x1000, "OLIGSM"}, {0x1001, "OLIADPCM"},
        {0x1002, "OLICELP"}, {0x1003, "OLISBC"}, {0x1004, "OLIOPR"},
        {0x1100, "LH Codec"}, {0x1400, "Norris"}, {0x1401, "ISIAudio"},
        {0x1500, "Soundspace Music Compression"}, {0x2000, "DVM"}
        };
        for (const auto& c2n : codeToName) {
          if (format == c2n.code) {
            m_detailInfo.format += QLatin1Char(' ');
            m_detailInfo.format += QString::fromLatin1(c2n.name);
            break;
          }
        }
      }
      int bits = wavProperties->bitsPerSample();
      if (bits > 0) {
        m_detailInfo.format += QLatin1Char(' ');
        m_detailInfo.format += QString::number(bits);
        m_detailInfo.format += QLatin1String(" bit");
      }
#endif
    } else if ((apeProperties =
                dynamic_cast<TagLib::APE::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("APE %1.%2 %3 bit"))
        .arg(apeProperties->version() / 1000)
        .arg(apeProperties->version() % 1000)
        .arg(apeProperties->bitsPerSample());
    } else if ((modProperties =
                dynamic_cast<TagLib::Mod::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("Mod %1 %2 Instruments"))
          .arg(getTrackerName())
          .arg(modProperties->instrumentCount());
    } else if ((s3mProperties =
                dynamic_cast<TagLib::S3M::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("S3M %1 V%2 T%3"))
          .arg(getTrackerName())
          .arg(s3mProperties->fileFormatVersion())
          .arg(s3mProperties->trackerVersion(), 0, 16);
      m_detailInfo.channelMode = s3mProperties->stereo()
          ? DetailInfo::CM_Stereo : DetailInfo::CM_None;
    } else if ((itProperties =
                dynamic_cast<TagLib::IT::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("IT %1 V%2 %3 Instruments"))
          .arg(getTrackerName())
          .arg(itProperties->version(), 0, 16)
          .arg(itProperties->instrumentCount());
      m_detailInfo.channelMode = itProperties->stereo()
          ? DetailInfo::CM_Stereo : DetailInfo::CM_None;
    } else if ((xmProperties =
                dynamic_cast<TagLib::XM::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("XM %1 V%2 %3 Instruments"))
          .arg(getTrackerName())
          .arg(xmProperties->version(), 0, 16)
          .arg(xmProperties->instrumentCount());
    } else if ((opusProperties =
          dynamic_cast<TagLib::Ogg::Opus::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("Opus %1"))
          .arg(opusProperties->opusVersion());
#if TAGLIB_VERSION >= 0x020000
    } else if ((dsfProperties =
                dynamic_cast<TagLib::DSF::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("DSF %1"))
                                .arg(dsfProperties->formatVersion());
    } else if ((dffProperties =
                dynamic_cast<TagLib::DSDIFF::Properties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("DFF"));
#else
    } else if ((dsfProperties =
              dynamic_cast<DSFProperties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("DSF %1"))
                                .arg(dsfProperties->version());
    } else if ((dffProperties =
                dynamic_cast<DSDIFFProperties*>(audioProperties)) != nullptr) {
      m_detailInfo.format = QString(QLatin1String("DFF"));
#endif
    }

    m_detailInfo.bitrate = audioProperties->bitrate();
    m_detailInfo.sampleRate = audioProperties->sampleRate();
    if (audioProperties->channels() > 0) {
      m_detailInfo.channels = audioProperties->channels();
    }
#if TAGLIB_VERSION >= 0x020000
    m_detailInfo.duration = audioProperties->lengthInSeconds();
#else
    // lengthInSeconds() does not work for DSF with TagLib 1.x, because
    // it is not virtual.
    m_detailInfo.duration = audioProperties->length();
#endif
  } else {
    m_detailInfo.valid = false;
  }
}

/**
 * Get tracker name of a module file.
 *
 * @return tracker name, null if not found.
 */
QString TagLibFile::getTrackerName() const
{
  QString trackerName;
  if (auto modTag = dynamic_cast<TagLib::Mod::Tag*>(m_tag[Frame::Tag_2])) {
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
  if (id3v2Tag && (header = id3v2Tag->header()) != nullptr) {
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
    if (dynamic_cast<const TagLib::ID3v1::Tag*>(tag) != nullptr) {
      type = TT_Id3v1;
      return QLatin1String("ID3v1.1");
    } else if ((id3v2Tag = dynamic_cast<const TagLib::ID3v2::Tag*>(tag)) != nullptr) {
      type = TT_Id3v2;
      TagLib::ID3v2::Header* header = id3v2Tag->header();
      if (header) {
        uint majorVersion = header->majorVersion();
        uint revisionNumber = header->revisionNumber();
        return QString(QLatin1String("ID3v2.%1.%2"))
          .arg(majorVersion).arg(revisionNumber);
      } else {
        return QLatin1String("ID3v2");
      }
    } else if (dynamic_cast<const TagLib::Ogg::XiphComment*>(tag) != nullptr) {
      type = TT_Vorbis;
      return QLatin1String("Vorbis");
    } else if (dynamic_cast<const TagLib::APE::Tag*>(tag) != nullptr) {
      type = TT_Ape;
      return QLatin1String("APE");
    } else if (dynamic_cast<const TagLib::MP4::Tag*>(tag) != nullptr) {
      type = TT_Mp4;
      return QLatin1String("MP4");
    } else if (dynamic_cast<const TagLib::ASF::Tag*>(tag) != nullptr) {
      type = TT_Asf;
      return QLatin1String("ASF");
#if TAGLIB_VERSION >= 0x010a00
    } else if (dynamic_cast<const TagLib::RIFF::Info::Tag*>(tag) != nullptr) {
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


namespace TagLibFileInternal {

/**
 * Fix up the format of the value if needed for an ID3v2 frame.
 *
 * @param self this TagLibFile instance
 * @param frameType type of frame
 * @param value the value to be set for frame, will be modified if needed
 */
void fixUpTagLibFrameValue(const TagLibFile* self,
                           Frame::Type frameType, QString& value)
{
  if (frameType == Frame::FT_Genre) {
    const bool useId3v23 = self->m_id3v2Version == 3;
    if (!TagConfig::instance().genreNotNumeric() ||
        (useId3v23 && value.contains(Frame::stringListSeparator()))) {
      value = Genres::getNumberString(value, useId3v23);
    }
  } else if (frameType == Frame::FT_Track) {
    self->formatTrackNumberIfEnabled(value, true);
  } else if ((frameType == Frame::FT_Arranger ||
              frameType == Frame::FT_Performer) &&
             !value.isEmpty() &&
             !value.contains(Frame::stringListSeparator())) {
    // When using TIPL or TMCL and writing an ID3v2.3.0 tag, TagLib
    // needs in ID3v2::Tag::downgradeFrames() a string list with at
    // least two elements, otherwise it will not take the value over
    // to an IPLS frame. If there is a single value in such a case,
    // add a second element.
    value += Frame::stringListSeparator();
  }
}

}


using namespace TagLibFileInternal;

namespace {

/** Types and descriptions for id3lib frame IDs */
const struct TypeStrOfId {
  const char* str;
  Frame::Type type;
  bool supported;
} typeStrOfId[] = {
  { QT_TRANSLATE_NOOP("@default", "AENC - Audio encryption"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "APIC - Attached picture"), Frame::FT_Picture, true },
  { QT_TRANSLATE_NOOP("@default", "ASPI - Audio seek point index"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010a00
  { QT_TRANSLATE_NOOP("@default", "CHAP - Chapter"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "COMM - Comments"), Frame::FT_Comment, true },
  { QT_TRANSLATE_NOOP("@default", "COMR - Commercial"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010a00
  { QT_TRANSLATE_NOOP("@default", "CTOC - Table of contents"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "ENCR - Encryption method registration"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "EQU2 - Equalisation (2)"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "ETCO - Event timing codes"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "GEOB - General encapsulated object"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "GRID - Group identification registration"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010c00
  { QT_TRANSLATE_NOOP("@default", "GRP1 - Grouping"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "LINK - Linked information"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "MCDI - Music CD identifier"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "MLLT - MPEG location lookup table"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010c00
  { QT_TRANSLATE_NOOP("@default", "MVIN - Movement Number"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "MVNM - Movement Name"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "OWNE - Ownership frame"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "PRIV - Private frame"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "PCNT - Play counter"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "PCST - Podcast"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "POPM - Popularimeter"), Frame::FT_Rating, true },
  { QT_TRANSLATE_NOOP("@default", "POSS - Position synchronisation frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "RBUF - Recommended buffer size"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "RVA2 - Relative volume adjustment (2)"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "RVRB - Reverb"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SEEK - Seek frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SIGN - Signature frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SYLT - Synchronized lyric/text"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "SYTC - Synchronized tempo codes"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "TALB - Album/Movie/Show title"), Frame::FT_Album, true },
  { QT_TRANSLATE_NOOP("@default", "TBPM - BPM (beats per minute)"),  Frame::FT_Bpm, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TCAT - Podcast category"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TCMP - iTunes compilation flag"), Frame::FT_Compilation, true },
  { QT_TRANSLATE_NOOP("@default", "TCOM - Composer"), Frame::FT_Composer, true },
  { QT_TRANSLATE_NOOP("@default", "TCON - Content type"), Frame::FT_Genre, true },
  { QT_TRANSLATE_NOOP("@default", "TCOP - Copyright message"), Frame::FT_Copyright, true },
  { QT_TRANSLATE_NOOP("@default", "TDEN - Encoding time"), Frame::FT_EncodingTime, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TDES - Podcast description"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TDLY - Playlist delay"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TDOR - Original release time"), Frame::FT_OriginalDate, true },
  { QT_TRANSLATE_NOOP("@default", "TDRC - Recording time"), Frame::FT_Date, true },
  { QT_TRANSLATE_NOOP("@default", "TDRL - Release time"), Frame::FT_ReleaseDate, true },
  { QT_TRANSLATE_NOOP("@default", "TDTG - Tagging time"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TENC - Encoded by"), Frame::FT_EncodedBy, true },
  { QT_TRANSLATE_NOOP("@default", "TEXT - Lyricist/Text writer"), Frame::FT_Lyricist, true },
  { QT_TRANSLATE_NOOP("@default", "TFLT - File type"), Frame::FT_Other, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TGID - Podcast identifier"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TIPL - Involved people list"), Frame::FT_Arranger, true },
  { QT_TRANSLATE_NOOP("@default", "TIT1 - Content group description"), Frame::FT_Work, true },
  { QT_TRANSLATE_NOOP("@default", "TIT2 - Title/songname/content description"), Frame::FT_Title, true },
  { QT_TRANSLATE_NOOP("@default", "TIT3 - Subtitle/Description refinement"), Frame::FT_Description, true },
  { QT_TRANSLATE_NOOP("@default", "TKEY - Initial key"), Frame::FT_InitialKey, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TKWD - Podcast keywords"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TLAN - Language(s)"), Frame::FT_Language, true },
  { QT_TRANSLATE_NOOP("@default", "TLEN - Length"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TMCL - Musician credits list"), Frame::FT_Performer, true },
  { QT_TRANSLATE_NOOP("@default", "TMED - Media type"), Frame::FT_Media, true },
  { QT_TRANSLATE_NOOP("@default", "TMOO - Mood"), Frame::FT_Mood, true },
  { QT_TRANSLATE_NOOP("@default", "TOAL - Original album/movie/show title"), Frame::FT_OriginalAlbum, true },
  { QT_TRANSLATE_NOOP("@default", "TOFN - Original filename"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TOLY - Original lyricist(s)/text writer(s)"), Frame::FT_Author, true },
  { QT_TRANSLATE_NOOP("@default", "TOPE - Original artist(s)/performer(s)"), Frame::FT_OriginalArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TOWN - File owner/licensee"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TPE1 - Lead performer(s)/Soloist(s)"), Frame::FT_Artist, true },
  { QT_TRANSLATE_NOOP("@default", "TPE2 - Band/orchestra/accompaniment"), Frame::FT_AlbumArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TPE3 - Conductor/performer refinement"), Frame::FT_Conductor, true },
  { QT_TRANSLATE_NOOP("@default", "TPE4 - Interpreted, remixed, or otherwise modified by"), Frame::FT_Remixer, true },
  { QT_TRANSLATE_NOOP("@default", "TPOS - Part of a set"), Frame::FT_Disc, true },
  { QT_TRANSLATE_NOOP("@default", "TPRO - Produced notice"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TPUB - Publisher"), Frame::FT_Publisher, true },
  { QT_TRANSLATE_NOOP("@default", "TRCK - Track number/Position in set"), Frame::FT_Track, true },
  { QT_TRANSLATE_NOOP("@default", "TRSN - Internet radio station name"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TRSO - Internet radio station owner"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TSO2 - Album artist sort order"), Frame::FT_SortAlbumArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TSOA - Album sort order"), Frame::FT_SortAlbum, true },
  { QT_TRANSLATE_NOOP("@default", "TSOC - Composer sort order"), Frame::FT_SortComposer, true },
  { QT_TRANSLATE_NOOP("@default", "TSOP - Performer sort order"), Frame::FT_SortArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TSOT - Title sort order"), Frame::FT_SortName, true },
  { QT_TRANSLATE_NOOP("@default", "TSRC - ISRC (international standard recording code)"), Frame::FT_Isrc, true },
  { QT_TRANSLATE_NOOP("@default", "TSSE - Software/Hardware and settings used for encoding"), Frame::FT_EncoderSettings, true },
  { QT_TRANSLATE_NOOP("@default", "TSST - Set subtitle"), Frame::FT_Subtitle, true },
  { QT_TRANSLATE_NOOP("@default", "TXXX - User defined text information"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "UFID - Unique file identifier"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "USER - Terms of use"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "USLT - Unsynchronized lyric/text transcription"), Frame::FT_Lyrics, true },
  { QT_TRANSLATE_NOOP("@default", "WCOM - Commercial information"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WCOP - Copyright/Legal information"), Frame::FT_Other, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "WFED - Podcast feed"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "WOAF - Official audio file webpage"), Frame::FT_WWWAudioFile, true },
  { QT_TRANSLATE_NOOP("@default", "WOAR - Official artist/performer webpage"), Frame::FT_Website, true },
  { QT_TRANSLATE_NOOP("@default", "WOAS - Official audio source webpage"), Frame::FT_WWWAudioSource, true },
  { QT_TRANSLATE_NOOP("@default", "WORS - Official internet radio station homepage"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WPAY - Payment"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WPUB - Official publisher webpage"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WXXX - User defined URL link"), Frame::FT_Other, true }
};

/**
 * Get type and description of frame.
 *
 * @param id   ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here
 */
void getTypeStringForFrameId(const TagLib::ByteVector& id, Frame::Type& type,
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
    if (type == Frame::FT_Other) {
      type = Frame::getTypeFromCustomFrameName(
            QByteArray(id.data(), id.size()));
    }
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
const char* getStringForType(Frame::Type type)
{
  if (type != Frame::FT_Other) {
    for (const auto& ts : typeStrOfId) {
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
QString getFieldsFromTextFrame(
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
      != nullptr) {
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
QString getFieldsFromApicFrame(
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
QString getFieldsFromCommFrame(
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
QString getFieldsFromUfidFrame(
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
        AttributeData::isHexString(text, 'Z', QLatin1String("-"))) {
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
QString getFieldsFromGeobFrame(
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
QString getFieldsFromUrlFrame(
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
QString getFieldsFromUserUrlFrame(
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
QString getFieldsFromUsltFrame(
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
QString getFieldsFromSyltFrame(
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
  const TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList stl =
      syltFrame->synchedText();
  for (auto it = stl.begin(); it != stl.end(); ++it) {
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
QString getFieldsFromEtcoFrame(
  const TagLib::ID3v2::EventTimingCodesFrame* etcoFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = etcoFrame->timestampFormat();
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList synchedData;
  const TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList sel =
      etcoFrame->synchedEvents();
  for (auto it = sel.begin(); it != sel.end(); ++it) {
    synchedData.append(static_cast<quint32>(it->time));
    synchedData.append(static_cast<int>(it->type));
  }
  field.m_value = synchedData;
  fields.push_back(field);

  return QString();
}

/**
 * Get the fields from a private frame.
 *
 * @param privFrame private frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromPrivFrame(
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
QString getFieldsFromPopmFrame(
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

/**
 * Get the fields from an ownership frame.
 *
 * @param owneFrame ownership frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromOwneFrame(
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

/**
 * Get a string representation of the data in an RVA2 frame.
 * @param rva2Frame RVA2 frame
 * @return string containing lines with space separated values for
 * type of channel, volume adjustment, bits representing peak,
 * peak volume. The peak volume is a hex byte array, the other values
 * are integers, the volume adjustment is signed. Bits representing peak
 * and peak volume are omitted if they have zero bits.
 */
QString rva2FrameToString(
    const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame)
{
  QString text;
  const TagLib::List<TagLib::ID3v2::RelativeVolumeFrame::ChannelType> channels =
      rva2Frame->channels();
  for (auto it = channels.begin(); it != channels.end(); ++it) {
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
void rva2FrameFromString(TagLib::ID3v2::RelativeVolumeFrame* rva2Frame,
                         const TagLib::String& text)
{
  // Unfortunately, it is not possible to remove data for a specific channel.
  // Only the whole frame could be deleted and a new one created.
  const auto lines = toQString(text).split(QLatin1Char('\n'));
  for (const QString& line : lines) {
    QStringList strs = line.split(QLatin1Char(' '));
    if (strs.size() > 1) {
      bool ok;
      int typeInt = strs.at(0).toInt(&ok);
      if (ok && typeInt >= 0 && typeInt <= 8) {
        short adj = strs.at(1).toShort(&ok);
        if (ok) {
          auto type =
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
QString getFieldsFromRva2Frame(
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

#if TAGLIB_VERSION >= 0x010a00
Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index);

/**
 * Get the fields from a chapter frame.
 *
 * @param chapFrame chapter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromChapFrame(
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
  for (auto it = frameList.begin();
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
QString getFieldsFromCtocFrame(
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
  const TagLib::ByteVectorList childElements = ctocFrame->childElements();
  for (auto it = childElements.begin(); it != childElements.end(); ++it) {
    elements.append(toQString(TagLib::String(*it, TagLib::String::Latin1)));
  }
  data.append(elements);
  field.m_value = data;
  fields.push_back(field);

  field.m_id = Frame::ID_Subframe;
  const TagLib::ID3v2::FrameList& frameList = ctocFrame->embeddedFrameList();
  for (auto it = frameList.begin();
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
QString getFieldsFromUnknownFrame(
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
QString getFieldsFromId3Frame(const TagLib::ID3v2::Frame* frame,
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
    const TagLib::ID3v2::PrivateFrame* privFrame;
    const TagLib::ID3v2::PopularimeterFrame* popmFrame;
    const TagLib::ID3v2::OwnershipFrame* owneFrame;
    const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame;
#if TAGLIB_VERSION >= 0x010a00
    const TagLib::ID3v2::ChapterFrame* chapFrame;
    const TagLib::ID3v2::TableOfContentsFrame* ctocFrame;
#endif
    if ((tFrame =
         dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(frame)) !=
        nullptr) {
      return getFieldsFromTextFrame(tFrame, fields, type);
    } else if ((apicFrame =
                dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame))
               != nullptr) {
      return getFieldsFromApicFrame(apicFrame, fields);
    } else if ((commFrame = dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(
                  frame)) != nullptr) {
      return getFieldsFromCommFrame(commFrame, fields);
    } else if ((ufidFrame =
                dynamic_cast<const TagLib::ID3v2::UniqueFileIdentifierFrame*>(
                  frame)) != nullptr) {
      return getFieldsFromUfidFrame(ufidFrame, fields);
    } else if ((geobFrame =
                dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
                  frame)) != nullptr) {
      return getFieldsFromGeobFrame(geobFrame, fields);
    } else if ((wxxxFrame = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(
                  frame)) != nullptr) {
      return getFieldsFromUserUrlFrame(wxxxFrame, fields);
    } else if ((wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(
                  frame)) != nullptr) {
      return getFieldsFromUrlFrame(wFrame, fields);
    } else if ((usltFrame = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
                frame)) != nullptr) {
      return getFieldsFromUsltFrame(usltFrame, fields);
    } else if ((syltFrame = dynamic_cast<const TagLib::ID3v2::SynchronizedLyricsFrame*>(
                frame)) != nullptr) {
      return getFieldsFromSyltFrame(syltFrame, fields);
    } else if ((etcoFrame = dynamic_cast<const TagLib::ID3v2::EventTimingCodesFrame*>(
                frame)) != nullptr) {
      return getFieldsFromEtcoFrame(etcoFrame, fields);
    } else if ((privFrame = dynamic_cast<const TagLib::ID3v2::PrivateFrame*>(
                frame)) != nullptr) {
      return getFieldsFromPrivFrame(privFrame, fields);
    } else if ((popmFrame = dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>(
                frame)) != nullptr) {
      return getFieldsFromPopmFrame(popmFrame, fields);
    } else if ((owneFrame = dynamic_cast<const TagLib::ID3v2::OwnershipFrame*>(
                frame)) != nullptr) {
      return getFieldsFromOwneFrame(owneFrame, fields);
    } else if ((rva2Frame = dynamic_cast<const TagLib::ID3v2::RelativeVolumeFrame*>(
                frame)) != nullptr) {
      return getFieldsFromRva2Frame(rva2Frame, fields);
#if TAGLIB_VERSION >= 0x010a00
    } else if ((chapFrame = dynamic_cast<const TagLib::ID3v2::ChapterFrame*>(
                frame)) != nullptr) {
      return getFieldsFromChapFrame(chapFrame, fields);
    } else if ((ctocFrame = dynamic_cast<const TagLib::ID3v2::TableOfContentsFrame*>(
                frame)) != nullptr) {
      return getFieldsFromCtocFrame(ctocFrame, fields);
#endif
    } else {
      TagLib::ByteVector id = frame->frameID();
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
TagLib::ByteVector languageCodeByteVector(QString str)
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
void setTextEncoding(TagLib::ID3v2::UserTextIdentificationFrame* f,
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

    auto type =
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

void setStringOrList(TagLib::ID3v2::TextIdentificationFrame* f,
                     const TagLib::String& text)
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
  if (AttributeData::isHexString(toQString(text), 'Z', QLatin1String("-"))) {
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

template <>
void setValue(TagLib::ID3v2::PrivateFrame* f, const TagLib::String& text)
{
  QByteArray newData;
  TagLib::String owner = f->owner();
  if (!owner.isEmpty() &&
      AttributeData(toQString(owner))
      .toByteArray(toQString(text), newData)) {
    f->setData(TagLib::ByteVector(newData.data(), newData.size()));
  }
}

template <>
void setValue(TagLib::ID3v2::PopularimeterFrame* f, const TagLib::String& text)
{
  f->setRating(text.toInt());
}

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

#if TAGLIB_VERSION >= 0x010a00
TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
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
void setValue(TagLib::ID3v2::ChapterFrame* f, const TagLib::String& text)
{
  f->setElementID(text.data(TagLib::String::Latin1));
}

template <>
void setValue(TagLib::ID3v2::TableOfContentsFrame* f, const TagLib::String& text)
{
  f->setElementID(text.data(TagLib::String::Latin1));
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
  while (!f->embeddedFrameList().isEmpty()) {
    f->removeEmbeddedFrame(f->embeddedFrameList()[0]);
  }
  // f->removeEmbeddedFrame() calls erase() thereby invalidating an iterator
  // on f->embeddedFrameList(). The uncommented code below will therefore crash.
  // const TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  // for (auto it = l.begin(); it != l.end(); ++it) {
  //   f->removeEmbeddedFrame(*it, true);
  // }
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
    for (auto it = elementStrings.constBegin();
         it != elementStrings.constEnd();
         ++it) {
      QByteArray id = it->toLatin1();
      elements.append(TagLib::ByteVector(id.constData(), id.size()));
    }
    f->setChildElements(elements);
  }
  // The embedded frames are deleted here because frames without subframes
  // do not have an ID_Subframe field and setSubframes() is not called.
  while (!f->embeddedFrameList().isEmpty()) {
    f->removeEmbeddedFrame(f->embeddedFrameList()[0]);
  }
  // f->removeEmbeddedFrame() calls erase() thereby invalidating an iterator
  // on f->embeddedFrameList(). The uncommented code below will therefore crash.
  // const TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  // for (auto it = l.begin(); it != l.end(); ++it) {
  //   f->removeEmbeddedFrame(*it, true);
  // }
}

template <class T>
void setSubframes(const TagLibFile*, T*, Frame::FieldList::const_iterator, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator) {}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::ChapterFrame* f,
                  Frame::FieldList::const_iterator begin, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator end) // clazy:exclude=function-args-by-ref
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (auto it = frames.begin(); it != frames.end(); ++it) {
    f->addEmbeddedFrame(createId3FrameFromFrame(self, const_cast<Frame&>(*it)));
  }
}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::TableOfContentsFrame* f,
                  Frame::FieldList::const_iterator begin, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator end) // clazy:exclude=function-args-by-ref
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (auto it = frames.begin(); it != frames.end(); ++it) {
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
    fixUpTagLibFrameValue(self, frame.getType(), text);
    setValue(tFrame, toTString(text));
    setTextEncoding(tFrame, getTextEncodingConfig(needsUnicode(text)));
  } else {
    for (auto fldIt = fieldList.constBegin(); fldIt != fieldList.constEnd(); ++fldIt) {
      const Frame::Field& fld = *fldIt;
      switch (fld.m_id) {
        case Frame::ID_Text:
        {
          QString value(fld.m_value.toString());
          fixUpTagLibFrameValue(self, frame.getType(), value);
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
        case Frame::ID_Email:
          setEmail(tFrame, fld);
          break;
        case Frame::ID_Rating:
          setRating(tFrame, fld);
          break;
        case Frame::ID_Counter:
          setCounter(tFrame, fld);
          break;
        case Frame::ID_Price:
          setPrice(tFrame, fld);
          break;
        case Frame::ID_Date:
          setDate(tFrame, fld);
          break;
        case Frame::ID_Seller:
          setSeller(tFrame, fld);
          break;
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
void setId3v2Frame(const TagLibFile* self,
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
    TagLib::ID3v2::PrivateFrame* privFrame;
    TagLib::ID3v2::PopularimeterFrame* popmFrame;
    TagLib::ID3v2::OwnershipFrame* owneFrame;
    TagLib::ID3v2::RelativeVolumeFrame* rva2Frame;
#if TAGLIB_VERSION >= 0x010a00
    TagLib::ID3v2::ChapterFrame* chapFrame;
    TagLib::ID3v2::TableOfContentsFrame* ctocFrame;
#endif
    if ((tFrame =
         dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3Frame))
        != nullptr) {
      auto txxxFrame =
        dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(id3Frame);
      if (txxxFrame) {
        setTagLibFrame(self, txxxFrame, frame);
      } else {
        setTagLibFrame(self, tFrame, frame);
      }
    } else if ((apicFrame =
                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame))
               != nullptr) {
      setTagLibFrame(self, apicFrame, frame);
    } else if ((commFrame = dynamic_cast<TagLib::ID3v2::CommentsFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, commFrame, frame);
    } else if ((ufidFrame =
                dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, ufidFrame, frame);
    } else if ((geobFrame =
                dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, geobFrame, frame);
    } else if ((wxxxFrame = dynamic_cast<TagLib::ID3v2::UserUrlLinkFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, wxxxFrame, frame);
    } else if ((wFrame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, wFrame, frame);
    } else if ((usltFrame =
                dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, usltFrame, frame);
    } else if ((syltFrame =
                dynamic_cast<TagLib::ID3v2::SynchronizedLyricsFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, syltFrame, frame);
    } else if ((etcoFrame =
                dynamic_cast<TagLib::ID3v2::EventTimingCodesFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, etcoFrame, frame);
    } else if ((privFrame = dynamic_cast<TagLib::ID3v2::PrivateFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, privFrame, frame);
    } else if ((popmFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, popmFrame, frame);
    } else if ((owneFrame = dynamic_cast<TagLib::ID3v2::OwnershipFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, owneFrame, frame);
    } else if ((rva2Frame = dynamic_cast<TagLib::ID3v2::RelativeVolumeFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, rva2Frame, frame);
#if TAGLIB_VERSION >= 0x010a00
    } else if ((chapFrame = dynamic_cast<TagLib::ID3v2::ChapterFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, chapFrame, frame);
    } else if ((ctocFrame = dynamic_cast<TagLib::ID3v2::TableOfContentsFrame*>(
                  id3Frame)) != nullptr) {
      setTagLibFrame(self, ctocFrame, frame);
#endif
    } else {
      TagLib::ByteVector id(id3Frame->frameID());
      // create temporary objects for frames not known by TagLib,
      // an UnknownFrame copy will be created by the edit method.
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
const char* getVorbisNameFromType(Frame::Type type)
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
    "DESCRIPTION",     // FT_Description,
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
    "RATING",          // FT_Rating,
    "WORK"             // FT_Work,
                       // FT_Custom1
  };
  Q_STATIC_ASSERT(sizeof(names) / sizeof(names[0]) == Frame::FT_Custom1);
  if (type == Frame::FT_Picture &&
      TagConfig::instance().pictureNameIndex() == TagConfig::VP_COVERART) {
    return "COVERART";
  }
  if (Frame::isCustomFrameType(type)) {
    return Frame::getNameForCustomFrame(type);
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
Frame::Type getTypeFromVorbisName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i < Frame::FT_Custom1; ++i) {
      auto type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getVorbisNameFromType(type)), type);
    }
    strNumMap.insert(QLatin1String("COVERART"), Frame::FT_Picture);
    strNumMap.insert(QLatin1String("METADATA_BLOCK_PICTURE"), Frame::FT_Picture);
  }
  auto it = strNumMap.constFind(name.remove(QLatin1Char('=')).toUpper());
  if (it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::getTypeFromCustomFrameName(name.toLatin1());
}

/**
 * Get the frame type for an APE name.
 *
 * @param name APE tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromApeName(const QString& name)
{
  Frame::Type type = getTypeFromVorbisName(name);
  if (type == Frame::FT_Other) {
    if (name == QLatin1String("YEAR")) {
      type = Frame::FT_Date;
    } else if (name == QLatin1String("TRACK")) {
      type = Frame::FT_Track;
    } else if (name == QLatin1String("ENCODED BY")) {
      type = Frame::FT_EncodedBy;
    } else if (name.startsWith(QLatin1String("COVER ART"))) {
      type = Frame::FT_Picture;
    }
  }
  return type;
}

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
    return fixUpTagKey(frame.getName(), TT_Vorbis).toUpper();
  }
}

namespace {

/**
 * Get internal name of an APE picture frame.
 *
 * @param pictureType picture type
 *
 * @return APE key.
 */
TagLib::String getApePictureName(PictureFrame::PictureType pictureType)
{
  TagLib::String name("COVER ART (");
  name += TagLib::String(PictureFrame::getPictureTypeString(pictureType))
      .upper();
  name += ')';
  return name;
}

/**
 * Get internal name of an APE frame.
 *
 * @param frame frame
 *
 * @return APE key.
 */
QString getApeName(const Frame& frame)
{
  Frame::Type type = frame.getType();
  if (type == Frame::FT_Date) {
    return QLatin1String("YEAR");
  } else if (type == Frame::FT_Track) {
    return QLatin1String("TRACK");
  } else if (type == Frame::FT_Picture) {
    PictureFrame::PictureType pictureType;
    if (!PictureFrame::getPictureType(frame, pictureType)) {
      pictureType = Frame::PT_CoverFront;
    }
    return toQString(getApePictureName(pictureType));
  } else if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  } else {
    return TaggedFile::fixUpTagKey(frame.getName(),
                                   TaggedFile::TT_Ape).toUpper();
  }
}

/** Type of data in MP4 frame. */
enum Mp4ValueType {
  MVT_ByteArray,
  MVT_CoverArt,
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
const Mp4NameTypeValue mp4NameTypeValues[] = {
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
  { "desc", Frame::FT_Description, MVT_String },
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
#if TAGLIB_VERSION >= 0x010c00
  { "purl", Frame::FT_Other, MVT_String },
  { "egid", Frame::FT_Other, MVT_String },
  { "cmID", Frame::FT_Other, MVT_UInt },
#endif
  { "xid ", Frame::FT_Other, MVT_String },
  { "covr", Frame::FT_Picture, MVT_CoverArt },
#if TAGLIB_VERSION >= 0x010c00
  { "\251wrk", Frame::FT_Work, MVT_String },
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
  { "PERFORMER", Frame::FT_Performer, MVT_String },
  { "PUBLISHER", Frame::FT_Publisher, MVT_String },
  { "RELEASECOUNTRY", Frame::FT_ReleaseCountry, MVT_String },
  { "REMIXER", Frame::FT_Remixer, MVT_String },
  { "SUBTITLE", Frame::FT_Subtitle, MVT_String },
  { "WEBSITE", Frame::FT_Website, MVT_String },
  { "WWWAUDIOFILE", Frame::FT_WWWAudioFile, MVT_String },
  { "WWWAUDIOSOURCE", Frame::FT_WWWAudioSource, MVT_String },
  { "RELEASEDATE", Frame::FT_ReleaseDate, MVT_String },
  { "rate", Frame::FT_Rating, MVT_String }
};

/**
 * Get MP4 name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
void getMp4NameForType(Frame::Type type, TagLib::String& name,
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
    auto it = typeNameMap.constFind(type);
    if (it != typeNameMap.constEnd()) {
      name = mp4NameTypeValues[*it].name;
      value = mp4NameTypeValues[*it].value;
    } else {
      auto customFrameName = Frame::getNameForCustomFrame(type);
      if (!customFrameName.isEmpty()) {
        name = TagLib::String(customFrameName.constData());
      }
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
bool getMp4TypeForName(const TagLib::String& name, Frame::Type& type,
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
  auto it = nameTypeMap.constFind(name);
  if (it != nameTypeMap.constEnd()) {
    type = mp4NameTypeValues[*it].type;
    value = mp4NameTypeValues[*it].value;
    if (type == Frame::FT_Other) {
      type = Frame::getTypeFromCustomFrameName(name.toCString());
    }
    return name[0] >= 'A' && name[0] <= 'Z';
  } else {
    type = Frame::getTypeFromCustomFrameName(name.toCString());
    value = MVT_String;
    return true;
  }
}

/**
 * Strip free form prefix from MP4 frame name.
 *
 * @param name MP4 frame name to be stripped
 */
void stripMp4FreeFormName(TagLib::String& name)
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
void prefixMp4FreeFormName(TagLib::String& name, const TagLib::MP4::Tag* mp4Tag)
{
  if (
#if TAGLIB_VERSION >= 0x010a00
      !mp4Tag->contains(name)
#else
      !const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(name)
#endif
      && ((!name.startsWith("----") &&
           !(name.length() == 4 &&
             (static_cast<char>(name[0]) == '\251' ||
              (name[0] >= 'a' && name[0] <= 'z')))) ||
#if TAGLIB_VERSION >= 0x010a00
          mp4Tag->contains("----:com.apple.iTunes:" + name)
#else
          const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(
            "----:com.apple.iTunes:" + name)
#endif
          )
      ) {
    Frame::Type type;
    Mp4ValueType valueType;
    if (getMp4TypeForName(name, type, valueType)) {
      // free form
      if (name[0] == ':') name = name.substr(1);
      TagLib::String freeFormName = "----:com.apple.iTunes:" + name;
      unsigned int nameLen;
      if (
#if TAGLIB_VERSION >= 0x010a00
          !mp4Tag->contains(freeFormName)
#else
          !const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(
            freeFormName)
#endif
          && (nameLen = name.length()) > 0) {
        // Not an iTunes free form name, maybe using another prefix
        // (such as "----:com.nullsoft.winamp:").
        // Search for a frame which ends with this name.
#if TAGLIB_VERSION >= 0x010a00
        const TagLib::MP4::ItemMap& items = mp4Tag->itemMap();
#else
        const TagLib::MP4::ItemListMap& items =
            const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap();
#endif
        for (auto it = items.begin(); it != items.end(); ++it) {
          const TagLib::String& key = it->first;
          if (key.length() >= nameLen &&
              key.substr(key.length() - nameLen, nameLen) == name) {
            freeFormName = key;
            break;
          }
        }
      }
      name = freeFormName;
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
void getMp4TypeForFrame(const Frame& frame, TagLib::String& name,
                        Mp4ValueType& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getMp4NameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(TaggedFile::fixUpTagKey(frame.getInternalName(),
                                             TaggedFile::TT_Mp4));
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
TagLib::MP4::Item getMp4ItemForFrame(const Frame& frame, TagLib::String& name)
{
  Mp4ValueType valueType;
  getMp4TypeForFrame(frame, name, valueType);
  switch (valueType) {
    case MVT_String:
      return TagLib::MP4::Item(
            TagLib::StringList::split(toTString(frame.getValue()),
                                      Frame::stringListSeparator().toLatin1()));
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
    case MVT_Byte:
      return TagLib::MP4::Item(static_cast<uchar>(frame.getValue().toInt()));
    case MVT_UInt:
      return TagLib::MP4::Item(frame.getValue().toUInt());
    case MVT_LongLong:
      return TagLib::MP4::Item(frame.getValue().toLongLong());
    case MVT_ByteArray:
    default:
      // binary data and album art are not handled by TagLib
      return TagLib::MP4::Item();
  }
}

}

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
#if TAGLIB_VERSION >= 0x010b01
    mp4Tag->setItem(name, item);
#else
    mp4Tag->itemListMap()[name] = item;
#endif
    markTagChanged(Frame::Tag_2, frame.getExtendedType());
  }
}

namespace {

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
const AsfNameTypeValue asfNameTypeValues[] = {
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
  { "WM/ContentGroupDescription", Frame::FT_Work, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ISRC", Frame::FT_Isrc, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Language", Frame::FT_Language, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Writer", Frame::FT_Lyricist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Lyrics", Frame::FT_Lyrics, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AudioSourceURL", Frame::FT_WWWAudioSource, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalAlbumTitle", Frame::FT_OriginalAlbum, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalArtist", Frame::FT_OriginalArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalReleaseYear", Frame::FT_OriginalDate, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SubTitleDescription", Frame::FT_Description, TagLib::ASF::Attribute::UnicodeType },
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
void getAsfNameForType(Frame::Type type, TagLib::String& name,
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
    auto it = typeNameMap.constFind(type);
    if (it != typeNameMap.constEnd()) {
      name = asfNameTypeValues[*it].name;
      value = asfNameTypeValues[*it].value;
    } else {
      auto customFrameName = Frame::getNameForCustomFrame(type);
      if (!customFrameName.isEmpty()) {
        name = TagLib::String(customFrameName.constData());
      }
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
void getAsfTypeForName(const TagLib::String& name, Frame::Type& type,
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
  auto it = nameTypeMap.constFind(name);
  if (it != nameTypeMap.constEnd()) {
    type = asfNameTypeValues[*it].type;
    value = asfNameTypeValues[*it].value;
  } else {
    type = Frame::getTypeFromCustomFrameName(name.toCString());
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
void getAsfTypeForFrame(const Frame& frame, TagLib::String& name,
                        TagLib::ASF::Attribute::AttributeTypes& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getAsfNameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(TaggedFile::fixUpTagKey(frame.getInternalName(),
                                             TaggedFile::TT_Asf));
    getAsfTypeForName(name, type, value);
  }
}

/**
 * Get a picture frame from a WM/Picture.
 *
 * @param picture ASF picture
 * @param frame   the picture frame is returned here
 *
 * @return true if ok.
 */
bool parseAsfPicture(const TagLib::ASF::Picture& picture, Frame& frame)
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
void renderAsfPicture(const Frame& frame, TagLib::ASF::Picture& picture)
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

/**
 * Get an ASF attribute for a frame.
 *
 * @param frame     frame
 * @param valueType ASF value type
 *
 * @return ASF attribute, an empty attribute is returned if not supported.
 */
TagLib::ASF::Attribute getAsfAttributeForFrame(
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
      else {
        TagLib::ASF::Picture picture;
        renderAsfPicture(frame, picture);
        return TagLib::ASF::Attribute(picture);
      }
  }
  return TagLib::ASF::Attribute();
}

/**
 * Get a picture frame from the bytes in an APE cover art frame.
 * The cover art frame has the following data:
 * zero terminated description string (UTF-8), picture data.
 *
 * @param name key of APE item
 * @param data bytes in APE cover art frame
 * @param frame the picture frame is returned here
 */
void parseApePicture(const QString& name,
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
void renderApePicture(const Frame& frame, TagLib::ByteVector& data)
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

#if TAGLIB_VERSION >= 0x010a00
/**
 * Get name of INFO tag from type.
 *
 * @param type type
 *
 * @return name, NULL if not supported.
 */
TagLib::ByteVector getInfoNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    "INAM",  // FT_Title,
    "IART",  // FT_Artist,
    "IPRD",  // FT_Album,
    "ICMT",  // FT_Comment,
    "ICRD",  // FT_Date,
    "IPRT",  // FT_Track
    "IGNR",  // FT_Genre,
             // FT_LastV1Frame = FT_Track,
    nullptr, // FT_AlbumArtist,
    "IENG",  // FT_Arranger,
    nullptr, // FT_Author,
    "IBPM",  // FT_Bpm,
    nullptr, // FT_CatalogNumber,
    nullptr, // FT_Compilation,
    "IMUS",  // FT_Composer,
    nullptr, // FT_Conductor,
    "ICOP",  // FT_Copyright,
    nullptr, // FT_Disc,
    "ITCH",  // FT_EncodedBy,
    "ISFT",  // FT_EncoderSettings,
    "IDIT",  // FT_EncodingTime,
    nullptr, // FT_Grouping,
    nullptr, // FT_InitialKey,
    "ISRC",  // FT_Isrc,
    "ILNG",  // FT_Language,
    "IWRI",  // FT_Lyricist,
    nullptr, // FT_Lyrics,
    "IMED",  // FT_Media,
    nullptr, // FT_Mood,
    nullptr, // FT_OriginalAlbum,
    nullptr, // FT_OriginalArtist,
    nullptr, // FT_OriginalDate,
    nullptr, // FT_Description,
    "ISTR",  // FT_Performer,
    nullptr, // FT_Picture,
    "IPUB",  // FT_Publisher,
    "ICNT",  // FT_ReleaseCountry,
    "IEDT",  // FT_Remixer,
    nullptr, // FT_SortAlbum,
    nullptr, // FT_SortAlbumArtist,
    nullptr, // FT_SortArtist,
    nullptr, // FT_SortComposer,
    nullptr, // FT_SortName,
    "PRT1",  // FT_Subtitle,
    "IBSU",  // FT_Website,
    nullptr, // FT_WWWAudioFile,
    nullptr, // FT_WWWAudioSource,
    nullptr, // FT_ReleaseDate,
    "IRTD",  // FT_Rating,
    nullptr, // FT_Work,
             // FT_Custom1
  };
  Q_STATIC_ASSERT(sizeof(names) / sizeof(names[0]) == Frame::FT_Custom1);
  if (type == Frame::FT_Track) {
    QByteArray ba = TagConfig::instance().riffTrackName().toLatin1();
    return TagLib::ByteVector(ba.constData(), ba.size());
  }
  if (Frame::isCustomFrameType(type)) {
    return TagLib::ByteVector(Frame::getNameForCustomFrame(type).constData());
  }
  const char* name = type <= Frame::FT_LastFrame ? names[type] : nullptr;
  return name ? TagLib::ByteVector(name, 4) : TagLib::ByteVector();
}

/**
 * Get the frame type for an INFO name.
 *
 * @param id INFO tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromInfoName(const TagLib::ByteVector& id)
{
  static QMap<TagLib::ByteVector, int> strNumMap;
  if (strNumMap.isEmpty()) {
    // first time initialization
    for (int i = 0; i < Frame::FT_Custom1; ++i) {
      auto type = static_cast<Frame::Type>(i);
      TagLib::ByteVector str = getInfoNameFromType(type);
      if (!str.isEmpty()) {
        strNumMap.insert(str, type);
      }
    }
    QStringList riffTrackNames = TagConfig::getRiffTrackNames();
    riffTrackNames.append(TagConfig::instance().riffTrackName());
    const auto constRiffTrackNames = riffTrackNames;
    for (const QString& str : constRiffTrackNames) {
      QByteArray ba = str.toLatin1();
      strNumMap.insert(TagLib::ByteVector(ba.constData(), ba.size()),
                       Frame::FT_Track);
    }
  }
  auto it = strNumMap.constFind(id);
  if (it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::getTypeFromCustomFrameName(
        QByteArray(id.data(), id.size()));
}

/**
 * Get internal name of an INFO frame.
 *
 * @param frame frame
 *
 * @return INFO id, "IKEY" if not found.
 */
TagLib::ByteVector getInfoName(const Frame& frame)
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

}

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
      if (tagNr == Frame::Tag_Id3v1
#if TAGLIB_VERSION < 0x010b01
          && !tstr.isNull()
#endif
          ) {
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
#if TAGLIB_VERSION >= 0x010b01
    QString str = tagNr != Frame::Tag_Id3v1 && type == Frame::FT_Genre
        ? getGenreString(tstr) : toQString(tstr);
#else
    QString str = tagNr != Frame::Tag_Id3v1 && type == Frame::FT_Genre
        ? getGenreString(tstr)
        : tstr.isNull() ? QLatin1String("") : toQString(tstr);
#endif
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
      TagLib::MP4::Tag* mp4Tag;
      TagLib::ASF::Tag* asfTag;
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        if (index >= 0 && index < static_cast<int>(frameList.size())) {
          // This is a hack. The frameList should not be modified directly.
          // However when removing the old frame and adding a new frame,
          // the indices of all frames get invalid.
          setId3v2Frame(this, frameList[index], frame);
          markTagChanged(tagNr, frame.getExtendedType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != nullptr) {
        QString frameValue(frame.getValue());
        Frame::ExtendedType extendedType = frame.getExtendedType();
        if (extendedType.getType() == Frame::FT_Picture) {
          if (m_pictures.isRead()) {
            int idx = Frame::fromNegativeIndex(frame.getIndex());
            if (idx >= 0 && idx < m_pictures.size()) {
              Frame newFrame(frame);
              PictureFrame::setDescription(newFrame, frameValue);
              if (PictureFrame::areFieldsEqual(m_pictures[idx], newFrame)) {
                m_pictures[idx].setValueChanged(false);
              } else {
                m_pictures[idx] = newFrame;
                markTagChanged(tagNr, extendedType);
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
        }
        TagLib::String key = toTString(getVorbisName(frame));
        TagLib::String value = toTString(frameValue);
        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        if (fieldListMap.contains(key) && fieldListMap[key].size() > 1) {
          int i = 0;
          bool found = false;
          for (auto it = fieldListMap.begin();
               it != fieldListMap.end();
               ++it) {
            TagLib::StringList stringList = (*it).second;
            for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
              if (i++ == index) {
                *slit = value;
                found = true;
                break;
              }
            }
            if (found) {
              // Replace all fields with this key to preserve the order.
#if TAGLIB_VERSION >= 0x010b01
              oggTag->removeFields(key);
#else
              oggTag->removeField(key);
#endif
              for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
                oggTag->addField(key, *slit, false);
              }
              break;
            }
          }
        } else {
          oggTag->addField(key, value, true);
        }
        if (frame.getType() == Frame::FT_Track) {
          int numTracks = getTotalNumberOfTracksIfEnabled();
          if (numTracks > 0) {
            oggTag->addField("TRACKTOTAL", TagLib::String::number(numTracks), true);
          }
        }
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
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
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
        Frame::ExtendedType extendedType = frame.getExtendedType();
        if (extendedType.getType() == Frame::FT_Picture) {
          if (m_pictures.isRead()) {
            int idx = Frame::fromNegativeIndex(frame.getIndex());
            if (idx >= 0 && idx < m_pictures.size()) {
              Frame newFrame(frame);
              if (PictureFrame::areFieldsEqual(m_pictures[idx], newFrame)) {
                m_pictures[idx].setValueChanged(false);
              } else {
                m_pictures[idx] = newFrame;
                markTagChanged(tagNr, extendedType);
              }
              return true;
            } else {
              return false;
            }
          }
        }
        setMp4Frame(frame, mp4Tag);
        return true;
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
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
              for (auto it = attrListMap.begin();
                   it != attrListMap.end();
                   ++it) {
                TagLib::ASF::AttributeList& attrList = (*it).second;
                for (auto ait = attrList.begin();
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
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
#if TAGLIB_VERSION >= 0x010a00
      } else if (auto infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        infoTag->setFieldText(getInfoName(frame), toTString(frame.getValue()));
        markTagChanged(tagNr, frame.getExtendedType());
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
#if TAGLIB_VERSION >= 0x010b01
    TagLib::String tstr = toTString(str);
#else
    TagLib::String tstr = str.isEmpty() ? TagLib::String::null : toTString(str);
#endif
    TagLib::String oldTstr;
    uint oldNum;
    const char* frameId = nullptr;
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
          markTagChanged(tagNr, Frame::ExtendedType(type));
        }
      } else {
        if (num > 0 && num != static_cast<int>(oldNum) &&
            getDefaultTextEncoding() == TagLib::String::Latin1) {
          tag->setYear(num);
          markTagChanged(tagNr, Frame::ExtendedType(type));
        } else if (num == 0 || num != static_cast<int>(oldNum)){
          QString yearStr;
          if (num != 0) {
            yearStr.setNum(num);
          } else {
            yearStr = frame.getValue();
          }
#if TAGLIB_VERSION >= 0x010b01
          TagLib::String tstr = toTString(yearStr);
#else
          TagLib::String tstr =
              yearStr.isEmpty() ? TagLib::String::null : toTString(yearStr);
#endif
          bool ok = false;
          if (dynamic_cast<TagLib::ID3v2::Tag*>(tag) != nullptr) {
            ok = setId3v2Unicode(tag, yearStr, tstr, frameId);
          } else if (auto mp4Tag =
                     dynamic_cast<TagLib::MP4::Tag*>(tag)) {
            TagLib::String name;
            Mp4ValueType valueType;
            getMp4NameForType(type, name, valueType);
            TagLib::MP4::Item item = TagLib::MP4::Item(tstr);
            ok = valueType == MVT_String && item.isValid();
            if (ok) {
#if TAGLIB_VERSION >= 0x010b01
              mp4Tag->setItem(name, item);
#else
              mp4Tag->itemListMap()[name] = item;
#endif
            }
          } else if (auto oggTag =
                     dynamic_cast<TagLib::Ogg::XiphComment*>(tag)) {
            oggTag->addField(getVorbisNameFromType(type), tstr, true);
            ok = true;
          }
          if (!ok) {
            tag->setYear(num);
          }
          markTagChanged(tagNr, Frame::ExtendedType(type));
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
            auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag);
            TagLib::MP4::Tag* mp4Tag;
            if (id3v2Tag) {
#if TAGLIB_VERSION >= 0x010b01
              TagLib::String tstr = toTString(trackStr);
#else
              TagLib::String tstr =
                trackStr.isEmpty() ? TagLib::String::null : toTString(trackStr);
#endif
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
            } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(tag)) != nullptr) {
              // Set a frame in order to store the total number too.
              Frame frame(Frame::FT_Track, str, QLatin1String(""), -1);
              setMp4Frame(frame, mp4Tag);
#if TAGLIB_VERSION >= 0x010a00
            } else if (auto infoTag =
                       dynamic_cast<TagLib::RIFF::Info::Tag*>(tag)) {
              infoTag->setFieldText(getInfoNameFromType(Frame::FT_Track),
                                    toTString(trackStr));
#endif
            } else {
              tag->setTrack(num);
            }
          }
        }
        markTagChanged(tagNr, Frame::ExtendedType(type));
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
              const auto genres =
                  tstr.split(Frame::stringListSeparator().toLatin1());
              for (const auto& genre : genres) {
                if (TagLib::ID3v1::genreIndex(genre) != 0xff) {
                  tstr = genre;
                  break;
                } else {
                  static const struct {
                    const char* newName;
                    const char* oldName;
                  } alternativeGenreNames[] = {
                    { "Avant-Garde", "Avantgarde" },
                    { "Beat Music", "Beat" },
                    { "Bebop", "Bebob" },
                    { "Britpop", "BritPop" },
                    { "Dancehall", "Dance Hall" },
                    { "Dark Wave", "Darkwave" },
                    { "Euro House", "Euro-House" },
                    { "Eurotechno", "Euro-Techno" },
                    { "Fast Fusion", "Fusion" },
                    { "Folk Rock", "Folk/Rock" },
                    { "Hip Hop", "Hip-Hop" },
                    { "Jazz-Funk", "Jazz+Funk" },
                    { "Pop-Funk", "Pop/Funk" },
                    { "Synth-Pop", "Synthpop" },
                    { "Worldbeat", "Negerpunk" }
                  };
                  static TagLib::Map<TagLib::String, TagLib::String> genreNameMap;
                  if (genreNameMap.isEmpty()) {
                    // first time initialization
                    for (const auto& agn : alternativeGenreNames) {
                      genreNameMap.insert(agn.newName, agn.oldName);
                    }
                  }
                  auto it = genreNameMap.find(tstr);
                  if (it != genreNameMap.end()) {
                    tstr = it->second;
                    break;
                  }
                }
              }
              tag->setGenre(tstr);
              // if the string cannot be converted to a number, set the truncation flag
              checkTruncation(tagNr, !tstr.isEmpty() &&
                              TagLib::ID3v1::genreIndex(tstr) == 0xff
                              ? 1 : 0, 1ULL << type, 0);
            } else {
              TagLib::ID3v2::TextIdentificationFrame* frame;
              auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag);
              if (id3v2Tag && TagConfig::instance().genreNotNumeric() &&
                  (frame = new TagLib::ID3v2::TextIdentificationFrame(
                    frameId, getDefaultTextEncoding())) != nullptr) {
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
        markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    }
  }
  return true;
}

namespace {

/**
 * Check if an ID3v2.4.0 frame ID is valid.
 *
 * @param frameId frame ID (4 characters)
 *
 * @return true if frame ID is valid.
 */
bool isFrameIdValid(const QString& frameId)
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
TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
                                              Frame& frame)
{
  TagLib::String::Type enc = TagLibFile::getDefaultTextEncoding();
  QString name = !Frame::isCustomFrameTypeOrOther(frame.getType())
      ? QString::fromLatin1(getStringForType(frame.getType()))
      : frame.getName();
  QString frameId = name;
  frameId.truncate(4);
  TagLib::ID3v2::Frame* id3Frame = nullptr;

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
#if TAGLIB_VERSION >= 0x010c00
      || frameId == QLatin1String("MVIN") || frameId == QLatin1String("MVNM")
      || frameId == QLatin1String("GRP1")
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
    auto commFrame =
        new TagLib::ID3v2::CommentsFrame(enc);
    id3Frame = commFrame;
    commFrame->setLanguage("eng"); // for compatibility with iTunes
    if (frame.getType() == Frame::FT_Other) {
      commFrame->setDescription(toTString(frame.getName()));
    }
  } else if (frameId == QLatin1String("APIC")) {
    id3Frame = new TagLib::ID3v2::AttachedPictureFrame;
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setTextEncoding(enc);
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setMimeType(
      "image/jpeg");
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setType(
      TagLib::ID3v2::AttachedPictureFrame::FrontCover);
  } else if (frameId == QLatin1String("UFID")) {
    // the bytevector must not be empty
    TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame =
        new TagLib::ID3v2::UniqueFileIdentifierFrame(
                  TagLib::String("http://www.id3.org/dummy/ufid.html"),
                  TagLib::ByteVector(" "));
    id3Frame = ufidFrame;
    QByteArray data;
    if (AttributeData::isHexString(frame.getValue(), 'Z', QLatin1String("-"))) {
      data = (frame.getValue() + QLatin1Char('\0')).toLatin1();
      ufidFrame->setIdentifier(TagLib::ByteVector(data.constData(),
                                                  data.size()));
    }
  } else if (frameId == QLatin1String("GEOB")) {
    id3Frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
    static_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(id3Frame)->setTextEncoding(enc);
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
    static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("SYLT")) {
    id3Frame = new TagLib::ID3v2::SynchronizedLyricsFrame(enc);
    static_cast<TagLib::ID3v2::SynchronizedLyricsFrame*>(id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("ETCO")) {
    id3Frame = new TagLib::ID3v2::EventTimingCodesFrame;
  } else if (frameId == QLatin1String("POPM")) {
    auto popmFrame =
        new TagLib::ID3v2::PopularimeterFrame;
    id3Frame = popmFrame;
    popmFrame->setEmail(toTString(TagConfig::instance().defaultPopmEmail()));
  } else if (frameId == QLatin1String("PRIV")) {
    auto privFrame =
        new TagLib::ID3v2::PrivateFrame;
    id3Frame = privFrame;
    if (!frame.getName().startsWith(QLatin1String("PRIV"))) {
      privFrame->setOwner(toTString(frame.getName()));
      QByteArray data;
      if (AttributeData(frame.getName()).toByteArray(frame.getValue(), data)) {
        privFrame->setData(TagLib::ByteVector(data.constData(), data.size()));
      }
    }
  } else if (frameId == QLatin1String("OWNE")) {
    id3Frame = new TagLib::ID3v2::OwnershipFrame(enc);
  } else if (frameId == QLatin1String("RVA2")) {
    id3Frame = new TagLib::ID3v2::RelativeVolumeFrame;
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
    auto txxxFrame =
      new TagLib::ID3v2::UserTextIdentificationFrame(enc);
    TagLib::String description;
    if (frame.getType() == Frame::FT_CatalogNumber) {
      description = "CATALOGNUMBER";
    } else if (frame.getType() == Frame::FT_ReleaseCountry) {
      description = "RELEASECOUNTRY";
    } else if (frame.getType() == Frame::FT_Grouping) {
      description = "GROUPING";
    } else if (frame.getType() == Frame::FT_Subtitle) {
      description = "SUBTITLE";
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
      TagLib::MP4::Tag* mp4Tag;
      TagLib::ASF::Tag* asfTag;
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
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
          markTagChanged(tagNr, frame.getExtendedType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != nullptr) {
        QString name(getVorbisName(frame));
        QString value(frame.getValue());
        if (frame.getType() == Frame::FT_Picture) {
          if (frame.getFieldList().empty()) {
            PictureFrame::setFields(
              frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), QLatin1String("image/jpeg"),
              PictureFrame::PT_CoverFront, QLatin1String(""), QByteArray());
          }
          if (m_pictures.isRead()) {
            PictureFrame::setDescription(frame, value);
            frame.setIndex(Frame::toNegativeIndex(m_pictures.size()));
            m_pictures.append(frame);
            markTagChanged(tagNr, frame.getExtendedType());
            return true;
          } else {
            PictureFrame::getFieldsToBase64(frame, value);
          }
        }
        TagLib::String tname = toTString(name);
        TagLib::String tvalue = toTString(value);
        if (tvalue.isEmpty()) {
          tvalue = " "; // empty values are not added by TagLib
        }
        oggTag->addField(tname, tvalue, false);
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        int index = 0;
        bool found = false;
        for (auto it = fieldListMap.begin();
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
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
        if (frame.getType() == Frame::FT_Picture &&
            frame.getFieldList().isEmpty()) {
          // Do not replace an already existing picture.
          Frame::PictureType pictureType = Frame::PT_CoverFront;
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (int i = Frame::PT_CoverFront; i <= Frame::PT_PublisherLogo; ++i) {
            auto pt = static_cast<Frame::PictureType>(i);
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
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

        const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
        int index = 0;
        bool found = false;
        for (auto it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          if ((*it).first == tname) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
        if (frame.getType() == Frame::FT_Picture) {
          if (frame.getFieldList().empty()) {
            PictureFrame::setFields(frame);
          }
          if (m_pictures.isRead()) {
            frame.setIndex(Frame::toNegativeIndex(m_pictures.size()));
            m_pictures.append(frame);
            markTagChanged(tagNr, frame.getExtendedType());
            return true;
          }
        }
        TagLib::String name;
        TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
        if (!item.isValid()) {
          return false;
        }
        frame.setExtendedType(Frame::ExtendedType(frame.getType(),
                                                  toQString(name)));
        prefixMp4FreeFormName(name, mp4Tag);
#if TAGLIB_VERSION >= 0x010b01
        mp4Tag->setItem(name, item);
        const TagLib::MP4::ItemMap& itemListMap = mp4Tag->itemMap();
#else
        mp4Tag->itemListMap()[name] = item;
        const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
#endif
        int index = 0;
        bool found = false;
        for (auto it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          if ((*it).first == name) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
        if (frame.getType() == Frame::FT_Picture &&
            frame.getFieldList().empty()) {
          PictureFrame::setFields(frame);
        }
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
        for (auto it = attrListMap.begin();
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
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
#if TAGLIB_VERSION >= 0x010a00
      } else if (auto infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        TagLib::ByteVector id = getInfoName(frame);
        TagLib::String tvalue = toTString(frame.getValue());
        if (tvalue.isEmpty()) {
          tvalue = " "; // empty values are not added by TagLib
        }
        infoTag->setFieldText(id, tvalue);
        QString name = QString::fromLatin1(id.data(), id.size());
        frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
        const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
        int index = 0;
        bool found = false;
        for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
          if ((*it).first == id) {
            found = true;
            break;
          }
          ++index;
        }
        frame.setIndex(found ? index : -1);
        markTagChanged(tagNr, frame.getExtendedType());
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
      TagLib::MP4::Tag* mp4Tag;
      TagLib::ASF::Tag* asfTag;
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        if (index >= 0 && index < static_cast<int>(frameList.size())) {
          id3v2Tag->removeFrame(frameList[index]);
          markTagChanged(tagNr, frame.getExtendedType());
          return true;
        }
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != nullptr) {
        QString frameValue(frame.getValue());
        if (frame.getType() == Frame::FT_Picture) {
          if (m_pictures.isRead()) {
            int idx = Frame::fromNegativeIndex(frame.getIndex());
            if (idx >= 0 && idx < m_pictures.size()) {
              m_pictures.removeAt(idx);
              while (idx < m_pictures.size()) {
                m_pictures[idx].setIndex(Frame::toNegativeIndex(idx));
                ++idx;
              }
              markTagChanged(tagNr, frame.getExtendedType());
              return true;
            }
          } else {
            PictureFrame::getFieldsToBase64(frame, frameValue);
          }
        }
        TagLib::String key =
          toTString(frame.getInternalName());
#if TAGLIB_VERSION >= 0x010b01
        oggTag->removeFields(key, toTString(frameValue));
#else
        oggTag->removeField(key, toTString(frameValue));
#endif
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
        TagLib::String key = toTString(frame.getInternalName());
        apeTag->removeItem(key);
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
        if (frame.getType() == Frame::FT_Picture) {
          if (m_pictures.isRead()) {
            int idx = Frame::fromNegativeIndex(frame.getIndex());
            if (idx >= 0 && idx < m_pictures.size()) {
              m_pictures.removeAt(idx);
              while (idx < m_pictures.size()) {
                m_pictures[idx].setIndex(Frame::toNegativeIndex(idx));
                ++idx;
              }
              markTagChanged(tagNr, frame.getExtendedType());
              return true;
            }
          }
        }
        TagLib::String name = toTString(frame.getInternalName());
        prefixMp4FreeFormName(name, mp4Tag);
#if TAGLIB_VERSION >= 0x010b01
        mp4Tag->removeItem(name);
#else
        mp4Tag->itemListMap().erase(name);
#endif
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
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
              for (auto it = attrListMap.begin();
                   it != attrListMap.end();
                   ++it) {
                TagLib::ASF::AttributeList& attrList = (*it).second;
                for (auto ait = attrList.begin();
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
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
#if TAGLIB_VERSION >= 0x010a00
      } else if (auto infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        QByteArray ba = frame.getInternalName().toLatin1();
        TagLib::ByteVector id(ba.constData(), ba.size());
        infoTag->removeField(id);
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
#endif
      }
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

namespace {

/**
 * Create a frame from a TagLib ID3 frame.
 * @param id3Frame TagLib ID3 frame
 * @param index, -1 if not used
 * @return frame.
 */
Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index)
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
        } else if (description == QLatin1String("GROUPING")) {
          frame.setType(Frame::FT_Grouping);
        } else if (description == QLatin1String("SUBTITLE")) {
          frame.setType(Frame::FT_Subtitle);
        } else {
          if (description.startsWith(QLatin1String("QuodLibet::"))) {
            // remove ExFalso/QuodLibet "namespace"
            description = description.mid(11);
          }
          frame.setExtendedType(Frame::ExtendedType(
            Frame::getTypeFromCustomFrameName(description.toLatin1()),
            frame.getInternalName() + QLatin1Char('\n') + description));
        }
      }
    }
  } else if (id3Frame->frameID().startsWith("PRIV")) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Owner);
    if (fieldValue.isValid()) {
      QString owner = fieldValue.toString();
      if (!owner.isEmpty()) {
        frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                  frame.getInternalName() + QLatin1Char('\n') + owner));
      }
    }
  }
  return frame;
}

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
      TagLib::MP4::Tag* mp4Tag;
      TagLib::ASF::Tag* asfTag;
      if (flt.areAllEnabled()) {
        if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
          const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          for (auto it = frameList.begin();
               it != frameList.end();) {
            id3v2Tag->removeFrame(*it++, true);
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) !=
                   nullptr) {
          const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
          for (auto it = fieldListMap.begin();
               it != fieldListMap.end();) {
#if TAGLIB_VERSION >= 0x010b01
            oggTag->removeFields((*it++).first);
#else
            oggTag->removeField((*it++).first);
#endif
          }
          m_pictures.clear();
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (auto it = itemListMap.begin();
               it != itemListMap.end();) {
            apeTag->removeItem((*it++).first);
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
#if TAGLIB_VERSION >= 0x010b01
          const auto& itemMap = mp4Tag->itemMap();
          for (auto it = itemMap.begin(); it != itemMap.end();) {
            mp4Tag->removeItem((*it++).first);
          }
#else
          mp4Tag->itemListMap().clear();
#endif
          m_pictures.clear();
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
          asfTag->setTitle("");
          asfTag->setArtist("");
          asfTag->setComment("");
          asfTag->setCopyright("");
          asfTag->setRating("");
          asfTag->attributeListMap().clear();
          markTagChanged(tagNr, Frame::ExtendedType());
#if TAGLIB_VERSION >= 0x010a00
        } else if (auto infoTag =
                   dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
          const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
          for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
            infoTag->removeField((*it).first);
          }
          markTagChanged(tagNr, Frame::ExtendedType());
#endif
        } else {
          TaggedFile::deleteFrames(tagNr, flt);
        }
      } else {
        if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
          const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          for (auto it = frameList.begin();
               it != frameList.end();) {
            Frame frame(createFrameFromId3Frame(*it, -1));
            if (flt.isEnabled(frame.getType(), frame.getName())) {
              id3v2Tag->removeFrame(*it++, true);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) !=
                   nullptr) {
          const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
          for (auto it = fieldListMap.begin();
               it != fieldListMap.end();) {
            QString name(toQString((*it).first));
            if (flt.isEnabled(getTypeFromVorbisName(name), name)) {
#if TAGLIB_VERSION >= 0x010b01
              oggTag->removeFields((*it++).first);
#else
              oggTag->removeField((*it++).first);
#endif
            } else {
              ++it;
            }
          }
          if (flt.isEnabled(Frame::FT_Picture)) {
            m_pictures.clear();
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
          const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
          for (auto it = itemListMap.begin();
               it != itemListMap.end();) {
            QString name(toQString((*it).first));
            if (flt.isEnabled(getTypeFromApeName(name), name)) {
              apeTag->removeItem((*it++).first);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
#if TAGLIB_VERSION >= 0x010b01
          const auto& itemMap = mp4Tag->itemMap();
          for (auto it = itemMap.begin(); it != itemMap.end();) {
            TagLib::String name = it->first;
            stripMp4FreeFormName(name);
            Frame::Type type;
            Mp4ValueType valueType;
            getMp4TypeForName(name, type, valueType);
            if (flt.isEnabled(type, toQString(name))) {
              mp4Tag->removeItem((*it++).first);
            } else {
              ++it;
            }
          }
#else
          TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
          Frame::Type type;
          Mp4ValueType valueType;
          for (auto it = itemListMap.begin();
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
#endif
          if (flt.isEnabled(Frame::FT_Picture)) {
            m_pictures.clear();
          }
          markTagChanged(tagNr, Frame::ExtendedType());
        } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
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
          for (auto it = attrListMap.begin();
               it != attrListMap.end();) {
            getAsfTypeForName((*it).first, type, valueType);
            QString name(toQString((*it).first));
            if (flt.isEnabled(type, name)) {
              attrListMap.erase(it++);
            } else {
              ++it;
            }
          }
          markTagChanged(tagNr, Frame::ExtendedType());
#if TAGLIB_VERSION >= 0x010a00
        } else if (auto infoTag =
                   dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
          const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
          for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
            TagLib::ByteVector id = (*it).first;
            QString name = QString::fromLatin1(id.data(), id.size());
            if (flt.isEnabled(getTypeFromInfoName(id), name)) {
              infoTag->removeField(id);
            }
          }
          markTagChanged(tagNr, Frame::ExtendedType());
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
      TagLib::MP4::Tag* mp4Tag;
      TagLib::ASF::Tag* asfTag;
      if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr])) != nullptr) {
        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
        int i = 0;
        for (auto it = frameList.begin();
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
      } else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag[tagNr])) != nullptr) {
        const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
        int i = 0;
        for (auto it = fieldListMap.begin();
             it != fieldListMap.end();
             ++it) {
          QString name = toQString((*it).first);
          Frame::Type type = getTypeFromVorbisName(name);
          const TagLib::StringList stringList = (*it).second;
          for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
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
        if (m_pictures.isRead()) {
          for (auto it = m_pictures.constBegin(); it != m_pictures.constEnd(); ++it) {
            frames.insert(*it);
          }
        }
      } else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag[tagNr])) != nullptr) {
        const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
        int i = 0;
        for (auto it = itemListMap.begin();
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
          if (type == Frame::FT_Picture) {
            TagLib::ByteVector data = (*it).second.binaryData();
            parseApePicture(name, data, frame);
          }
          frames.insert(frame);
        }
      } else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tag[tagNr])) != nullptr) {
#if TAGLIB_VERSION >= 0x010b01
        const TagLib::MP4::ItemMap& itemListMap = mp4Tag->itemMap();
#else
        const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
#endif
        int i = 0;
        for (auto it = itemListMap.begin();
             it != itemListMap.end();
             ++it) {
          TagLib::String name = (*it).first;
          stripMp4FreeFormName(name);
          Frame::Type type;
          Mp4ValueType valueType;
          getMp4TypeForName(name, type, valueType);
          QString value;
          switch (valueType) {
            case MVT_String:
            {
              TagLib::StringList strings = (*it).second.toStringList();
              value = strings.size() > 0
                  ? toQString(strings.toString(
                                Frame::stringListSeparator().toLatin1()))
                  : QLatin1String("");
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
            case MVT_CoverArt:
              // handled by m_pictures
              break;
            case MVT_Byte:
              value.setNum((*it).second.toByte());
              break;
            case MVT_UInt:
              value.setNum((*it).second.toUInt());
              break;
            case MVT_LongLong:
              value.setNum((*it).second.toLongLong());
              break;
            case MVT_ByteArray:
            default:
              // binary data and album art are not handled by TagLib
              value = QLatin1String("");
          }
          if (type != Frame::FT_Picture) {
            frames.insert(
              Frame(type, value, toQString(name), i++));
          }
        }
        if (m_pictures.isRead()) {
          for (auto it = m_pictures.constBegin(); it != m_pictures.constEnd(); ++it) {
            frames.insert(*it);
          }
        }
      } else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tag[tagNr])) != nullptr) {
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
        for (auto it = attrListMap.begin();
             it != attrListMap.end();
             ++it) {
          name = (*it).first;
          getAsfTypeForName(name, type, valueType);
          for (auto ait = (*it).second.begin();
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
            ++i;
            if (type == Frame::FT_Picture) {
              parseAsfPicture((*ait).toPicture(), frame);
            }
            frames.insert(frame);
          }
        }
#if TAGLIB_VERSION >= 0x010a00
      } else if (auto infoTag =
                 dynamic_cast<TagLib::RIFF::Info::Tag*>(m_tag[tagNr])) {
        const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
        int i = 0;
        for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
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
  if (dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr]) != nullptr &&
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
  if (m_tagType[tagNr] == TT_Id3v2 ||
      (m_tagType[tagNr] == TT_Unknown &&
       dynamic_cast<TagLib::ID3v2::Tag*>(m_tag[tagNr]))) {
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      auto name = Frame::ExtendedType(static_cast<Frame::Type>(k), QLatin1String("")). // clazy:exclude=reserve-candidates
          getName();
      if (!name.isEmpty()) {
        lst.append(name);
      }
    }
    for (const auto& ts : typeStrOfId) {
      if (ts.type == Frame::FT_Other && ts.supported && ts.str) {
        lst.append(QString::fromLatin1(ts.str));
      }
    }
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
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (const auto& mp4NameTypeValue : mp4NameTypeValues) {
      if (mp4NameTypeValue.type == Frame::FT_Other &&
          mp4NameTypeValue.value != MVT_ByteArray &&
          !(mp4NameTypeValue.name[0] >= 'A' &&
            mp4NameTypeValue.name[0] <= 'Z')) {
        lst.append(QString::fromLatin1(mp4NameTypeValue.name));
      }
    }
  } else if (m_tagType[tagNr] == TT_Asf) {
    TagLib::String name;
    TagLib::ASF::Attribute::AttributeTypes valueType;
    Frame::Type type;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      name = "";
      type = static_cast<Frame::Type>(k);
      getAsfNameForType(type, name, valueType);
      if (!name.isEmpty()) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (const auto& asfNameTypeValue : asfNameTypeValues) {
      if (asfNameTypeValue.type == Frame::FT_Other) {
        lst.append(QString::fromLatin1(asfNameTypeValue.name));
      }
    }
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
      auto type = static_cast<Frame::Type>(k);
      if (!getInfoNameFromType(type).isEmpty()) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (auto fieldName : fieldNames) {
      lst.append(QString::fromLatin1(fieldName)); // clazy:exclude=reserve-candidates
    }
#endif
  } else {
    static const char* const fieldNames[] = {
      "CONTACT",
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
    const bool picturesSupported = m_pictures.isRead() ||
        m_tagType[tagNr] == TT_Vorbis || m_tagType[tagNr] == TT_Ape;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      if (k != Frame::FT_Picture || picturesSupported) {
        auto name = Frame::ExtendedType(static_cast<Frame::Type>(k),
                                        QLatin1String("")).getName();
        if (!name.isEmpty()) {
          lst.append(name);
        }
      }
    }
    for (auto fieldName : fieldNames) {
      lst.append(QString::fromLatin1(fieldName)); // clazy:exclude=reserve-candidates
    }
  }
  return lst;
}

/**
 * Set the encoding to be used for tag 1.
 *
 * @param name of encoding, default is ISO 8859-1
 */
void TagLibFile::setTextEncodingV1(const QString& name)
{
#if QT_VERSION >= 0x060000
  TextCodecStringHandler::setStringDecoder(name);
#else
  TextCodecStringHandler::setTextCodec(name != QLatin1String("ISO-8859-1")
      ? QTextCodec::codecForName(name.toLatin1().data()) : nullptr);
#endif
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
  setDefaultTextEncoding(
    static_cast<TagConfig::TextEncoding>(TagConfig::instance().textEncoding()));
  setTextEncodingV1(TagConfig::instance().textEncodingV1());
}

namespace {

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

  QScopedPointer<AACFileTypeResolver> m_aacFileTypeResolver;
  QScopedPointer<MP2FileTypeResolver> m_mp2FileTypeResolver;
  QScopedPointer<TextCodecStringHandler> m_textCodecStringHandler;
};


TagLibInitializer::TagLibInitializer()
  : m_aacFileTypeResolver(new AACFileTypeResolver),
    m_mp2FileTypeResolver(new MP2FileTypeResolver),
    m_textCodecStringHandler(new TextCodecStringHandler)
{
}

void TagLibInitializer::init()
{
  TagLib::FileRef::addFileTypeResolver(m_aacFileTypeResolver.data());
  TagLib::FileRef::addFileTypeResolver(m_mp2FileTypeResolver.data());
  TagLib::ID3v1::Tag::setStringHandler(m_textCodecStringHandler.data());
}

TagLibInitializer::~TagLibInitializer() {
  // Must not be inline because of forwared declared QScopedPointer.
}

TagLibInitializer tagLibInitializer;

}

/**
 * Static initialization.
 * Registers file types.
 */
void TagLibFile::staticInit()
{
  tagLibInitializer.init();
}
