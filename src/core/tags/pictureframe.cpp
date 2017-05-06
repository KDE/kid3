/**
 * \file pictureframe.cpp
 * Frame containing picture.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03 Mar 2008
 *
 * Copyright (C) 2008-2017  Urs Fleisch
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

#include "pictureframe.h"
#include <QFile>
#include <QImage>
#include <QBuffer>
#include <QCoreApplication>
#if QT_VERSION >= 0x050000
#include <QMimeDatabase>
#include <QMimeType>
#endif

namespace {

/**
 * List of picture type strings, NULL terminated.
 */
static const char* const pictureTypeNames[] = {
  QT_TRANSLATE_NOOP("@default", "Other"),
  QT_TRANSLATE_NOOP("@default", "32x32 pixels PNG file icon"),
  QT_TRANSLATE_NOOP("@default", "Other file icon"),
  QT_TRANSLATE_NOOP("@default", "Cover (front)"),
  QT_TRANSLATE_NOOP("@default", "Cover (back)"),
  QT_TRANSLATE_NOOP("@default", "Leaflet page"),
  QT_TRANSLATE_NOOP("@default", "Media"),
  QT_TRANSLATE_NOOP("@default", "Lead artist/lead performer/soloist"),
  QT_TRANSLATE_NOOP("@default", "Artist/performer"),
  QT_TRANSLATE_NOOP("@default", "Conductor"),
  QT_TRANSLATE_NOOP("@default", "Band/Orchestra"),
  QT_TRANSLATE_NOOP("@default", "Composer"),
  QT_TRANSLATE_NOOP("@default", "Lyricist/text writer"),
  QT_TRANSLATE_NOOP("@default", "Recording Location"),
  QT_TRANSLATE_NOOP("@default", "During recording"),
  QT_TRANSLATE_NOOP("@default", "During performance"),
  QT_TRANSLATE_NOOP("@default", "Movie/video screen capture"),
  QT_TRANSLATE_NOOP("@default", "A bright coloured fish"),
  QT_TRANSLATE_NOOP("@default", "Illustration"),
  QT_TRANSLATE_NOOP("@default", "Band/artist logotype"),
  QT_TRANSLATE_NOOP("@default", "Publisher/Studio logotype"),
  NULL
};

/**
 * List of untranslated picture type strings, NULL terminated.
 */
static const char* const pictureTypeStrings[] = {
  "Other",
  "Png Icon",
  "Icon",
  "Front",
  "Back",
  "Leaflet",
  "Media",
  "Lead Artist",
  "Artist",
  "Conductor",
  "Band",
  "Composer",
  "Lyricist",
  "Recording Location",
  "During Recording",
  "During Performance",
  "Video Capture",
  "Fish",
  "Illustration",
  "Band Logotype",
  "Publisher Logotype",
  NULL
};

}


/**
 * Construct properties from a new image.
 * @param data image data
 */
PictureFrame::ImageProperties::ImageProperties(const QByteArray& data)
{
  QImage image;
  if (image.loadFromData(data)) {
    m_width = image.width();
    m_height = image.height();
    m_depth = image.bitPlaneCount();
    m_numColors = image.colorCount();
    m_imageHash = qHash(data);
  } else {
    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_numColors = 0;
    m_imageHash = 0;
  }
}


/**
 * Constructor.
 *
 * @param data        binary picture data
 * @param description description
 * @param pictureType picture type
 * @param mimeType    MIME type
 * @param enc         text encoding
 * @param imgFormat   image format
 */
PictureFrame::PictureFrame(
  const QByteArray& data,
  const QString& description,
  PictureType pictureType,
  const QString& mimeType,
  TextEncoding enc,
  const QString& imgFormat)
{
  setType(FT_Picture);
  setFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
}

/**
 * Constructor.
 *
 * @param frame general frame
 */
PictureFrame::PictureFrame(const Frame& frame)
{
  *(static_cast<Frame*>(this)) = frame;
  setType(FT_Picture);

  // Make sure all fields are available in the correct order
  TextEncoding enc = TE_ISO8859_1;
  PictureType pictureType = PT_CoverFront;
  QString imgFormat(QLatin1String("JPG")), mimeType(QLatin1String("image/jpeg")), description;
  QByteArray data;
  getFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
  setFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
}

/**
 * Destructor.
 */
PictureFrame::~PictureFrame()
{
}

/**
 * Set all properties.
 *
 * @param frame       frame to set
 * @param enc         text encoding
 * @param imgFormat   image format
 * @param mimeType    MIME type
 * @param pictureType picture type
 * @param description description
 * @param data        binary picture data
 * @param imgProps    optional METADATA_BLOCK_PICTURE image properties
 */
void PictureFrame::setFields(Frame& frame,
                             TextEncoding enc, const QString& imgFormat,
                             const QString& mimeType, PictureType pictureType,
                             const QString& description, const QByteArray& data,
                             const ImageProperties* imgProps)
{
  Field field;
  FieldList& fields = frame.fieldList();
  fields.clear();

  field.m_id = ID_TextEnc;
  field.m_value = enc;
  fields.push_back(field);

  field.m_id = ID_ImageFormat;
  field.m_value = imgFormat;
  fields.push_back(field);

  field.m_id = ID_MimeType;
  field.m_value = mimeType;
  fields.push_back(field);

  field.m_id = ID_PictureType;
  field.m_value = pictureType;
  fields.push_back(field);

  field.m_id = ID_Description;
  field.m_value = description;
  fields.push_back(field);

  field.m_id = ID_Data;
  field.m_value = data;
  fields.push_back(field);

  if (imgProps && !imgProps->isNull()) {
    field.m_id = ID_ImageProperties;
    field.m_value.setValue(*imgProps);
    fields.push_back(field);
  }

  frame.setValue(description);
}

/**
 * Set all properties of a GEOB frame.
 *
 * @param frame       frame to set
 * @param enc         text encoding
 * @param mimeType    MIME type
 * @param fileName    file name
 * @param description description
 * @param data        binary data
 */
void PictureFrame::setGeobFields(
    Frame& frame, TextEncoding enc, const QString& mimeType,
    const QString& fileName, const QString& description, const QByteArray& data)
{
  Field field;
  FieldList& fields = frame.fieldList();
  fields.clear();

  field.m_id = ID_TextEnc;
  field.m_value = enc;
  fields.push_back(field);

  field.m_id = ID_MimeType;
  field.m_value = mimeType;
  fields.push_back(field);

  field.m_id = ID_Filename;
  field.m_value = fileName;
  fields.push_back(field);

  field.m_id = ID_Description;
  field.m_value = description;
  fields.push_back(field);

  field.m_id = ID_Data;
  field.m_value = data;
  fields.push_back(field);

  frame.setValue(description);
}

/**
 * Get all properties.
 * Unavailable fields are not set.
 *
 * @param frame       frame to get
 * @param enc         text encoding
 * @param imgFormat   image format
 * @param mimeType    MIME type
 * @param pictureType picture type
 * @param description description
 * @param data        binary picture data
 * @param imgProps    optional METADATA_BLOCK_PICTURE image properties
 */
void PictureFrame::getFields(const Frame& frame,
                             TextEncoding& enc, QString& imgFormat,
                             QString& mimeType, PictureType& pictureType,
                             QString& description, QByteArray& data,
                             ImageProperties* imgProps)
{
  for (Frame::FieldList::const_iterator it = frame.getFieldList().begin();
       it != frame.getFieldList().end();
       ++it) {
    switch ((*it).m_id) {
      case ID_TextEnc:
        enc = static_cast<TextEncoding>((*it).m_value.toInt());
        break;
      case ID_ImageFormat:
        imgFormat = (*it).m_value.toString();
        break;
      case ID_MimeType:
        mimeType = (*it).m_value.toString();
        break;
      case ID_PictureType:
        pictureType = static_cast<PictureType>((*it).m_value.toInt());
        break;
      case ID_Description:
        description = (*it).m_value.toString();
        break;
      case ID_Data:
        data = (*it).m_value.toByteArray();
        break;
      case ID_ImageProperties:
        if (imgProps) {
          *imgProps = (*it).m_value.value<ImageProperties>();
        }
        break;
      default:
        qDebug("Unknown picture field ID");
    }
  }
}

/**
 * Check if all the fields of two picture frames are equal.
 * @param f1 first picture frame
 * @param f2 second picture frame
 * @return true if equal.
 */
bool PictureFrame::areFieldsEqual(const Frame& f1, const Frame& f2)
{
  TextEncoding enc1, enc2;
  QString imgFormat1, imgFormat2;
  QString mimeType1, mimeType2;
  PictureType pictureType1, pictureType2;
  QString description1, description2;
  QByteArray data1, data2;
  getFields(f1, enc1, imgFormat1, mimeType1, pictureType1, description1, data1);
  getFields(f2, enc2, imgFormat2, mimeType2, pictureType2, description2, data2);
  return (data1 == data2 && description1 == description2 &&
          mimeType1 == mimeType2 && pictureType1 == pictureType2 &&
          imgFormat1 == imgFormat2 && enc1 == enc2);
}

/**
 * Set text encoding.
 *
 * @param frame frame to set
 * @param enc   text encoding
 *
 * @return true if field found and set.
 */
bool PictureFrame::setTextEncoding(Frame& frame, TextEncoding enc)
{
  return setField(frame, ID_TextEnc, enc);
}

/**
 * Get text encoding.
 *
 * @param frame frame to get
 * @param enc   the text encoding is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getTextEncoding(const Frame& frame, TextEncoding& enc)
{
  QVariant var(getField(frame, ID_TextEnc));
  if (var.isValid()) {
    enc = static_cast<TextEncoding>(var.toInt());
    return true;
  }
  return false;
}

/**
 * Set image format.
 *
 * @param frame     frame to set
 * @param imgFormat image format
 *
 * @return true if field found and set.
 */
bool PictureFrame::setImageFormat(Frame& frame, const QString& imgFormat)
{
  return setField(frame, ID_ImageFormat, imgFormat);
}

/**
 * Get image format.
 *
 * @param frame     frame to get
 * @param imgFormat the image format is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getImageFormat(const Frame& frame, QString& imgFormat)
{
  QVariant var(getField(frame, ID_ImageFormat));
  if (var.isValid()) {
    imgFormat = var.toString();
    return true;
  }
  return false;
}

/**
 * Set MIME type.
 *
 * @param frame   frame to set
 * @param mimeType MIME type
 *
 * @return true if field found and set.
 */
bool PictureFrame::setMimeType(Frame& frame, const QString& mimeType)
{
  return setField(frame, ID_MimeType, mimeType);
}

/**
 * Get MIME type.
 *
 * @param frame    frame to get
 * @param mimeType the MIME type is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getMimeType(const Frame& frame, QString& mimeType)
{
  QVariant var(getField(frame, ID_MimeType));
  if (var.isValid()) {
    mimeType = var.toString();
    return true;
  }
  return false;
}

/**
 * Set picture type.
 *
 * @param frame       frame to set
 * @param pictureType picture type
 *
 * @return true if field found and set.
 */
bool PictureFrame::setPictureType(Frame& frame, PictureType pictureType)
{
  return setField(frame, ID_PictureType, pictureType);
}

/**
 * Get picture type.
 *
 * @param frame       frame to get
 * @param pictureType the picture type is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getPictureType(const Frame& frame, PictureType& pictureType)
{
  QVariant var(getField(frame, ID_PictureType));
  if (var.isValid()) {
    pictureType = static_cast<PictureType>(var.toInt());
    return true;
  }
  return false;
}

/**
 * Set description.
 *
 * @param frame       frame to set
 * @param description description
 *
 * @return true if field found and set.
 */
bool PictureFrame::setDescription(Frame& frame, const QString& description)
{
  return setField(frame, ID_Description, description);
}

/**
 * Get description.
 *
 * @param frame       frame to get
 * @param description the description is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getDescription(const Frame& frame, QString& description)
{
  QVariant var(getField(frame, ID_Description));
  if (var.isValid()) {
    description = var.toString();
    return true;
  }
  return false;
}

/**
 * Set binary data.
 *
 * @param frame frame to set
 * @param data  binary data
 *
 * @return true if field found and set.
 */
bool PictureFrame::setData(Frame& frame, const QByteArray& data)
{
  return setField(frame, ID_Data, data);
}

/**
 * Get binary data.
 *
 * @param frame frame to get
 * @param data  the binary data is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getData(const Frame& frame, QByteArray& data)
{
  QVariant var(getField(frame, ID_Data));
  if (var.isValid()) {
    data = var.toByteArray();
    return true;
  }
  return false;
}

/**
 * Read binary data from file.
 *
 * @param frame frame to set
 * @param fileName name of data file
 *
 * @return true if file read, field found and set.
 */
bool PictureFrame::setDataFromFile(Frame& frame, const QString& fileName)
{
  bool result = false;
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
      int size = file.size();
      char* data = new char[size];
      QDataStream stream(&file);
      stream.readRawData(data, size);
      QByteArray ba;
      ba = QByteArray(data, size);
      result = setData(frame, ba);
      delete [] data;
      file.close();
    }
  }
  return result;
}

/**
 * Get binary data from image.
 *
 * @param frame frame to set
 * @param image image
 *
 * @return true if field found and set.
 */
bool PictureFrame::setDataFromImage(Frame& frame, const QImage& image)
{
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "JPG");
  return setData(frame, ba);
}

/**
 * Save binary data to a file.
 *
 * @param frame    frame
 * @param fileName name of data file to save
 *
 * @return true if field found and saved.
 */
bool PictureFrame::writeDataToFile(const Frame& frame, const QString& fileName)
{
  QByteArray ba;
  if (getData(frame, ba)) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
      QDataStream stream(&file);
      stream.writeRawData(ba.data(), ba.size());
      file.close();
      return true;
    }
  }
  return false;
}

/**
 * Get the MIME type and image format from a file.
 *
 * @param fileName name of data file
 * @param imgFormat if not null, the ID3v2.2 PIC image format ("JGP" or "PNG")
 * is set here
 *
 * @return mime type of file, null if not recognized.
 */
QString PictureFrame::getMimeTypeForFile(const QString& fileName,
                                         QString* imgFormat)
{
#if QT_VERSION >= 0x050000
  QMimeDatabase mimeDb;
  QString mimeType = mimeDb.mimeTypeForFile(fileName).name();
#else
  static const struct {
    const char* ext;
    const char* type;
  } extType[] = {
  { ".jpg",  "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".png",  "image/png" },
  { ".gif",  "image/gif" },
  { ".ico",  "image/vnd.microsoft.icon" },
  { ".svg",  "image/svg+xml" },
  { ".txt",  "text/plain" },
  { ".html", "text/html" },
  { ".htm",  "text/html" },
  { ".css",  "text/css" },
  { ".pdf",  "application/pdf" },
  { ".bin",  "application/octet-stream" },
  { ".js",   "application/javascript" },
  { ".json", "application/json" },
  { ".woff", "application/font-woff" },
  { ".ttf",  "application/font-sfnt" },
  { ".eot",  "application/vnd.ms-fontobject" }
  };
  QString mimeType;
  for (unsigned int i = 0; i < sizeof extType / sizeof extType[0]; ++i) {
    if (fileName.endsWith(QLatin1String(extType[i].ext), Qt::CaseInsensitive)) {
      mimeType = QString::fromLatin1(extType[i].type);
      break;
    }
  }
#endif
  if (imgFormat) {
    if (mimeType == QLatin1String("image/jpeg")) {
      *imgFormat = QLatin1String("JPG");
    } else if (mimeType == QLatin1String("image/png")) {
      *imgFormat = QLatin1String("PNG");
    }
  }
  return mimeType;
}

/**
 * Set the MIME type and image format from a file.
 *
 * @param frame frame to set
 * @param fileName name of data file
 *
 * @return true if field found and set.
 */
bool PictureFrame::setMimeTypeFromFileName(Frame& frame, const QString& fileName)
{
  QString imgFormat;
  QString mimeType = getMimeTypeForFile(fileName, &imgFormat);
  if (!mimeType.isEmpty()) {
    return setMimeType(frame, mimeType) && setImageFormat(frame, imgFormat);
  }
  return false;
}

/**
 * Get a 32-bit number from a byte array stored in big-endian order.
 *
 * @param data byte array
 * @param index index of first byte in data
 *
 * @return big endian 32-bit value.
 */
static unsigned long getBigEndianULongFromByteArray(const QByteArray& data,
                                                    int index)
{
  return
     ((unsigned char)data[index + 3] & 0xff)        |
    (((unsigned char)data[index + 2] & 0xff) << 8)  |
    (((unsigned char)data[index + 1] & 0xff) << 16) |
    (((unsigned char)data[index + 0] & 0xff) << 24);
}

/**
 * Render a 32-bit number to a byte array in big-endian order.
 *
 * @param value 32-bit value
 * @param data  byte array
 * @param index index of first byte in data
 */
static void renderBigEndianULongToByteArray(unsigned long value,
                                            QByteArray& data, int index)
{
  data[index + 3] = value & 0xff;
  value >>= 8;
  data[index + 2] = value & 0xff;
  value >>= 8;
  data[index + 1] = value & 0xff;
  value >>= 8;
  data[index + 0] = value & 0xff;
}

/**
 * Copy characters into a byte array.
 *
 * @param str   source string
 * @param data  destination byte array
 * @param index index of first byte in data
 * @param len   number of bytes to copy
 */
static void renderCharsToByteArray(const char* str, QByteArray& data,
                                   int index, int len)
{
  for (int i = 0; i < len; ++i) {
    data[index++] = *str++;
  }
}

/**
 * Set picture from a base64 string.
 *
 * @param frame       frame to set
 * @param base64Value base64 string
 */
void PictureFrame::setFieldsFromBase64(Frame& frame, const QString& base64Value)
{
  QByteArray ba = QByteArray::fromBase64(base64Value.toLatin1());
  PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
  QString mimeType(QLatin1String("image/jpeg"));
  QString description(QLatin1String(""));
  ImageProperties imgProps;
  if (frame.getInternalName() == QLatin1String("METADATA_BLOCK_PICTURE")) {
    unsigned long baSize = static_cast<unsigned long>(ba.size());
    if (baSize < 32) return;
    int index = 0;
    pictureType = static_cast<PictureFrame::PictureType>(
      getBigEndianULongFromByteArray(ba, index));
    index += 4;
    unsigned long mimeLen = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    if (baSize < index + mimeLen + 24) return;
    mimeType = QString::fromLatin1(ba.data() + index, mimeLen);
    index += mimeLen;
    unsigned long descLen = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    if (baSize < index + descLen + 20) return;
    description = QString::fromUtf8(ba.data() + index, descLen);
    index += descLen;
    uint width, height, depth, numColors;
    width = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    height = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    depth = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    numColors = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    unsigned long picLen = getBigEndianULongFromByteArray(ba, index);
    index += 4;
    if (baSize < index + picLen) return;
    ba = ba.mid(index);
    imgProps = ImageProperties(width, height, depth, numColors, ba);
  }
  PictureFrame::setFields(
    frame, TE_UTF8, QLatin1String(""), mimeType,
    pictureType, description, ba, &imgProps);
}

/**
 * Get picture to a base64 string.
 *
 * @param frame       frame to get
 * @param base64Value base64 string to set
 */
void PictureFrame::getFieldsToBase64(const Frame& frame, QString& base64Value)
{
  TextEncoding enc;
  PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
  QString imgFormat, mimeType, description;
  QByteArray pic;
  ImageProperties imgProps;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType,
                          pictureType, description, pic, &imgProps);
  if (frame.getInternalName() == QLatin1String("METADATA_BLOCK_PICTURE")) {
    QByteArray mimeStr = mimeType.toLatin1();
    QByteArray descStr = description.toUtf8();
    int mimeLen = mimeStr.length();
    int descLen = descStr.length();
    int picLen = pic.size();
    QByteArray ba(32 + mimeLen + descLen + picLen, '\0');
    int index = 0;
    renderBigEndianULongToByteArray(pictureType, ba, index);
    index += 4;
    renderBigEndianULongToByteArray(mimeLen, ba, index);
    index += 4;
    renderCharsToByteArray(mimeStr, ba, index, mimeLen);
    index += mimeLen;
    renderBigEndianULongToByteArray(descLen, ba, index);
    index += 4;
    renderCharsToByteArray(descStr, ba, index, descLen);
    index += descLen;

    if (!imgProps.isValidForImage(pic)) {
      imgProps = ImageProperties(pic);
    }

    renderBigEndianULongToByteArray(imgProps.width(), ba, index);
    index += 4;
    renderBigEndianULongToByteArray(imgProps.height(), ba, index);
    index += 4;
    renderBigEndianULongToByteArray(imgProps.depth(), ba, index);
    index += 4;
    renderBigEndianULongToByteArray(imgProps.numColors(), ba, index);
    index += 4;

    renderBigEndianULongToByteArray(picLen, ba, index);
    index += 4;
    renderCharsToByteArray(pic.data(), ba, index, picLen);
    pic = ba;
  }
  base64Value = QString::fromLatin1(pic.toBase64());
}

/**
 * Get a translated string for a picture type.
 *
 * @param type picture type
 *
 * @return picture type, null string if unknown.
 */
QString PictureFrame::getPictureTypeName(PictureType type)
{
  if (type >= 0 &&
      type < static_cast<int>(
        sizeof(pictureTypeNames) / sizeof(pictureTypeNames[0]) - 1)) {
    return QCoreApplication::translate("@default", pictureTypeNames[type]);
  }
  return QString();
}

/**
 * List of picture type strings, NULL terminated.
 */
const char* const* PictureFrame::getPictureTypeNames()
{
  return pictureTypeNames;
}

/**
 * Get an untranslated string for a picture type.
 *
 * @param type picture type
 *
 * @return picture type, 0 if unknown.
 */
const char* PictureFrame::getPictureTypeString(PictureType type)
{
  return type >= 0 && type < static_cast<int>(
        sizeof(pictureTypeStrings) / sizeof(pictureTypeStrings[0]) - 1)
      ? pictureTypeStrings[type] : 0;
}

/**
 * List of untranslated picture type strings, NULL terminated.
 */
const char* const* PictureFrame::getPictureTypeStrings()
{
  return pictureTypeStrings;
}

/**
 * Get picture type from an untranslated string.
 *
 * @param str untranslated picture type string
 *
 * @return picture type, PT_Other if unknown.
 */
PictureFrame::PictureType PictureFrame::getPictureTypeFromString(const char* str)
{
  for (unsigned int i = 0;
       i < sizeof(pictureTypeStrings) / sizeof(pictureTypeStrings[0]) - 1;
       ++i) {
    if (qstricmp(str, pictureTypeStrings[i]) == 0) {
      return static_cast<PictureType>(i);
    }
  }
  return PT_Other;
}
