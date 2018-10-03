/**
 * \file pictureframe.h
 * Frame containing picture.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03 Mar 2008
 *
 * Copyright (C) 2008-2013  Urs Fleisch
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

#ifndef PICTUREFRAME_H
#define PICTUREFRAME_H

#include "frame.h"

class QImage;

/** Frame containing picture. */
class KID3_CORE_EXPORT PictureFrame : public Frame {
public:
  /**
   * Additional properties for METADATA_BLOCK_PICTURE.
   */
  class KID3_CORE_EXPORT ImageProperties {
  public:
    /** Default constructor. */
    ImageProperties() :
      m_width(0), m_height(0), m_depth(0), m_numColors(0), m_imageHash(0) {}

    /**
     * Construct from properties in METADATA_BLOCK_PICTURE.
     * @param width width of picture in pixels
     * @param height height of picture in pixels
     * @param depth color depth of picture in bits-per-pixel
     * @param numColors number of colors used for indexed-color pictures
     *                  (e.g. GIF), or 0 for non-indexed pictures
     * @param data image data
     */
    ImageProperties(uint width, uint height, uint depth, uint numColors,
                    const QByteArray& data) :
      m_width(width), m_height(height), m_depth(depth), m_numColors(numColors),
      m_imageHash(qHash(data)) {}

    /**
     * Construct properties from a new image.
     * @param data image data
     */
    explicit ImageProperties(const QByteArray& data);

    /**
     * Check if the image properties are not set.
     * @return true if not set.
     */
    bool isNull() const {
      return m_width == 0 && m_height == 0 && m_depth == 0 &&
          m_numColors == 0 && m_imageHash == 0;
    }

    /**
     * Check if image properties are valid for an image.
     * @param data image data
     * @return true if valid.
     */
    bool isValidForImage(const QByteArray& data) const {
      return !isNull() && qHash(data) == m_imageHash;
    }

    /** Width of picture in pixels. */
    uint width() const { return m_width; }

    /** Height of picture in pixels. */
    uint height() const { return m_height; }

    /** Color depth of picture in bits-per-pixel.*/
    uint depth() const { return m_depth; }

    /**
     * Number of colors used for indexed-color pictures (e.g. GIF),
     * or 0 for non-indexed pictures.
     */
    uint numColors() const { return m_numColors; }

  private:
    uint m_width;
    uint m_height;
    uint m_depth;
    uint m_numColors;
    uint m_imageHash;
  };


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
  explicit PictureFrame(
    const QByteArray& data = QByteArray(),
    const QString& description = QLatin1String(""),
    PictureType pictureType = PT_CoverFront,
    const QString& mimeType = QLatin1String("image/jpeg"),
    TextEncoding enc = TE_ISO8859_1,
    const QString& imgFormat = QLatin1String("JPG"));

  /**
   * Constructor.
   *
   * @param frame general frame
   */
  explicit PictureFrame(const Frame& frame);

  /**
   * Destructor.
   */
  ~PictureFrame() = default;

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
  static void setFields(
    Frame& frame,
    TextEncoding enc = TE_ISO8859_1, const QString& imgFormat = QLatin1String("JPG"),
    const QString& mimeType = QLatin1String("image/jpeg"), PictureType pictureType = PT_CoverFront,
    const QString& description = QLatin1String(""), const QByteArray& data = QByteArray(),
    const ImageProperties* imgProps = nullptr);

  /**
   * Set all fields of a GEOB frame.
   *
   * @param frame       frame to set
   * @param enc         text encoding
   * @param mimeType    MIME type
   * @param fileName    file name
   * @param description description
   * @param data        binary data
   */
  static void setGeobFields(
      Frame& frame, TextEncoding enc = TE_ISO8859_1,
      const QString& mimeType = QLatin1String(""),
      const QString& fileName = QLatin1String(""),
      const QString& description = QLatin1String(""),
      const QByteArray& data = QByteArray());

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
  static void getFields(const Frame& frame,
                        TextEncoding& enc, QString& imgFormat,
                        QString& mimeType, PictureType& pictureType,
                        QString& description, QByteArray& data,
                        ImageProperties* imgProps = nullptr);

  /**
   * Check if all the fields of two picture frames are equal.
   * @param f1 first picture frame
   * @param f2 second picture frame
   * @return true if equal.
   */
  static bool areFieldsEqual(const Frame& f1, const Frame& f2);

  /**
   * Set text encoding.
   *
   * @param frame frame to set
   * @param enc   text encoding
   *
   * @return true if field found and set.
   */
  static bool setTextEncoding(Frame& frame, TextEncoding enc);

  /**
   * Get text encoding.
   *
   * @param frame frame to get
   * @param enc   the text encoding is returned here
   *
   * @return true if field found.
   */
  static bool getTextEncoding(const Frame& frame, TextEncoding& enc);

  /**
   * Set image format.
   *
   * @param frame     frame to set
   * @param imgFormat image format
   *
   * @return true if field found and set.
   */
  static bool setImageFormat(Frame& frame, const QString& imgFormat);

  /**
   * Get image format.
   *
   * @param frame     frame to get
   * @param imgFormat the image format is returned here
   *
   * @return true if field found.
   */
  static bool getImageFormat(const Frame& frame, QString& imgFormat);

  /**
   * Set MIME type.
   *
   * @param frame    frame to set
   * @param mimeType MIME type
   *
   * @return true if field found and set.
   */
  static bool setMimeType(Frame& frame, const QString& mimeType);

  /**
   * Get MIME type.
   *
   * @param frame    frame to get
   * @param mimeType the MIME type is returned here
   *
   * @return true if field found.
   */
  static bool getMimeType(const Frame& frame, QString& mimeType);

  /**
   * Set picture type.
   *
   * @param frame       frame to set
   * @param pictureType picture type
   *
   * @return true if field found and set.
   */
  static bool setPictureType(Frame& frame, PictureType pictureType);

  /**
   * Get picture type.
   *
   * @param frame       frame to get
   * @param pictureType the picture type is returned here
   *
   * @return true if field found.
   */
  static bool getPictureType(const Frame& frame, PictureType& pictureType);

  /**
   * Set description.
   *
   * @param frame       frame to set
   * @param description description
   *
   * @return true if field found and set.
   */
  static bool setDescription(Frame& frame, const QString& description);

  /**
   * Get description.
   *
   * @param frame       frame to get
   * @param description the description is returned here
   *
   * @return true if field found.
   */
  static bool getDescription(const Frame& frame, QString& description);

  /**
   * Set binary data.
   *
   * @param frame frame to set
   * @param data  binary data
   *
   * @return true if field found and set.
   */
  static bool setData(Frame& frame, const QByteArray& data);

  /**
   * Get binary data.
   *
   * @param frame frame to get
   * @param data  the binary data is returned here
   *
   * @return true if field found.
   */
  static bool getData(const Frame& frame, QByteArray& data);

  /**
   * Read binary data from file.
   *
   * @param frame frame to set
   * @param fileName name of data file
   *
   * @return true if file read, field found and set.
   */
  static bool setDataFromFile(Frame& frame, const QString& fileName);

  /**
   * Get binary data from image.
   *
   * @param frame frame to set
   * @param image image
   *
   * @return true if field found and set.
   */
  static bool setDataFromImage(Frame& frame, const QImage& image);

  /**
   * Save binary data to a file.
   *
   * @param frame    frame
   * @param fileName name of data file to save
   *
   * @return true if field found and saved.
   */
  static bool writeDataToFile(const Frame& frame, const QString& fileName);

  /**
   * Get the MIME type and image format from a file.
   *
   * @param fileName name of data file
   * @param imgFormat if not null, the ID3v2.2 PIC image format ("JGP" or "PNG")
   * is set here
   *
   * @return mime type of file, null if not recognized.
   */
  static QString getMimeTypeForFile(const QString& fileName,
                                    QString* imgFormat = nullptr);

  /**
   * Set the MIME type and image format from a file.
   *
   * @param frame frame to set
   * @param fileName name of data file
   *
   * @return true if field found and set.
   */
  static bool setMimeTypeFromFileName(Frame& frame, const QString& fileName);

  /**
   * Set picture from a base64 string.
   *
   * @param frame       frame to set
   * @param base64Value base64 string
   */
  static void setFieldsFromBase64(Frame& frame, const QString& base64Value);

  /**
   * Get picture to a base64 string.
   *
   * @param frame       frame to get
   * @param base64Value base64 string to set
   */
  static void getFieldsToBase64(const Frame& frame, QString& base64Value);

  /**
   * Get a translated string for a picture type.
   *
   * @param type picture type
   *
   * @return picture type, null string if unknown.
   */
  static QString getPictureTypeName(PictureType type);

  /**
   * Get list of picture type strings.
   * @return list of picture type names, NULL terminated.
   */
  static const char* const* getPictureTypeNames();

  /**
   * Get an untranslated string for a picture type.
   *
   * @param type picture type
   *
   * @return picture type, 0 if unknown.
   */
  static const char* getPictureTypeString(PictureType type);

  /**
   * List of untranslated picture type strings, NULL terminated.
   */
  static const char* const* getPictureTypeStrings();

  /**
   * Get picture type from an untranslated string.
   *
   * @param str untranslated picture type string
   *
   * @return picture type, PT_Other if unknown.
   */
  static PictureType getPictureTypeFromString(const char* str);
};

Q_DECLARE_METATYPE(PictureFrame::ImageProperties)

#endif // PICTUREFRAME_H
