/**
 * \file pictureframe.h
 * Frame containing picture.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03 Mar 2008
 *
 * Copyright (C) 2008-2009  Urs Fleisch
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
#include "config.h"
#if QT_VERSION >= 0x040000 || defined CONFIG_USE_KDE
#define HAVE_BASE64_ENCODING
#endif

class QImage;

/** Frame containing picture. */
class PictureFrame : public Frame {
public:
	/** Picture type, compatible with ID3v2 and FLAC. */
	enum PictureType {
		PT_Other = 0,
		PT_Png32Icon = 1,
		PT_OtherIcon = 2,
		PT_CoverFront = 3,
		PT_CoverBack = 4,
		PT_LeafletPage = 5,
		PT_Media = 6,
		PT_LeadArtist = 7,
		PT_Artist = 8,
		PT_Conductor = 9,
		PT_Band = 10,
		PT_Composer = 11,
		PT_Lyricist = 12,
		PT_RecordingLocation = 13,
		PT_DuringRecording = 14,
		PT_DuringPerformance = 15,
		PT_Video = 16,
		PT_Fish = 17,
		PT_Illustration = 18,
		PT_ArtistLogo = 19,
		PT_PublisherLogo = 20
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
		const QString& description = "",
		PictureType pictureType = PT_CoverFront,
		const QString& mimeType = "image/jpeg",
		Field::TextEncoding enc = Field::TE_ISO8859_1,
		const QString& imgFormat = "JPG");

	/**
	 * Constructor.
	 *
	 * @param frame general frame
	 */
	PictureFrame(const Frame& frame);

	/**
	 * Destructor.
	 */
	~PictureFrame();

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
	 */
	static void setFields(
		Frame& frame,
		Field::TextEncoding enc = Field::TE_ISO8859_1, const QString& imgFormat = "JPG",
		const QString& mimeType = "image/jpeg", PictureType pictureType = PT_CoverFront,
		const QString& description = "", const QByteArray& data = QByteArray());

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
	 */
	static void getFields(const Frame& frame,
												Field::TextEncoding& enc, QString& imgFormat,
												QString& mimeType, PictureType& pictureType,
												QString& description, QByteArray& data);

	/**
	 * Set text encoding.
	 *
	 * @param frame frame to set
	 * @param enc   text encoding
	 *
	 * @return true if field found and set.
	 */
	static bool setTextEncoding(Frame& frame, Field::TextEncoding enc);

	/**
	 * Get text encoding.
	 *
	 * @param frame frame to get
	 * @param enc   the text encoding is returned here
	 *
	 * @return true if field found.
	 */
	static bool getTextEncoding(const Frame& frame, Field::TextEncoding& enc);

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
	 * Set the MIME type and image format from the file name extension.
	 *
	 * @param frame frame to set
	 * @param fileName name of data file
	 *
	 * @return true if field found and set.
	 */
	static bool setMimeTypeFromFileName(Frame& frame, const QString& fileName);

#ifdef HAVE_BASE64_ENCODING
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
#endif // HAVE_BASE64_ENCODING

private:
	/**
	 * Set value of a field.
	 *
	 * @param frame frame to set
	 * @param id    field ID
	 * @param value field value
	 *
	 * @return true if field found and set.
	 */
	static bool setField(Frame& frame, Field::Id id, const QVariant& value);

	/**
	 * Get value of a field.
	 *
	 * @param frame frame to get
	 * @param id    field ID
	 *
	 * @return field value, invalid if not found.
	 */
	static QVariant getField(const Frame& frame, Field::Id id);
};

#endif // PICTUREFRAME_H
