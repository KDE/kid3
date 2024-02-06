/**
 * \file isettings.h
 * Interface for application settings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Apr 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#include "isettings.h"
#include <QVariant>
#include <QStringList>

/**
 * Destructor.
 */
ISettings::~ISettings()
{
}

/**
 * Migrate from an old settings version.
 * Can be called from the constructor of derived classes to automatically
 * convert old settings.
 */
void ISettings::migrateOldSettings()
{
  beginGroup(QLatin1String("Tags"));
  bool isOld = !contains(QLatin1String("MarkTruncations"));
  endGroup();
  if (isOld) {
    bool migrated = false;
#if QT_VERSION >= 0x060000
    static const struct {
      const char* oldKey;
      const char* newKey;
      QMetaType::Type type;
    } mappings[] = {
      { "Id3Format/FormatWhileEditing", "TagFormat/FormatWhileEditing", QMetaType::Bool },
      { "Id3Format/CaseConversion", "TagFormat/CaseConversion", QMetaType::Int },
      { "Id3Format/LocaleName", "TagFormat/LocaleName", QMetaType::QString },
      { "Id3Format/StrRepEnabled", "TagFormat/StrRepEnabled", QMetaType::Bool },
      { "Id3Format/StrRepMapKeys", "TagFormat/StrRepMapKeys", QMetaType::QStringList },
      { "Id3Format/StrRepMapValues", "TagFormat/StrRepMapValues", QMetaType::QStringList },
      { "General Options/HideToolBar", "MainWindow/HideToolBar", QMetaType::Bool },
      { "General Options/HideStatusBar", "MainWindow/HideStatusBar", QMetaType::Bool },
      { "General Options/Geometry", "MainWindow/Geometry", QMetaType::QByteArray },
      { "General Options/WindowState", "MainWindow/WindowState", QMetaType::QByteArray },
      { "General Options/UseFont", "MainWindow/UseFont", QMetaType::Bool },
      { "General Options/FontFamily", "MainWindow/FontFamily", QMetaType::QString },
      { "General Options/FontSize", "MainWindow/FontSize", QMetaType::Int },
      { "General Options/Style", "MainWindow/Style", QMetaType::QString },
      { "General Options/DontUseNativeDialogs", "MainWindow/DontUseNativeDialogs", QMetaType::Bool },
      { "General Options/MarkTruncations", "Tags/MarkTruncations", QMetaType::Bool },
      { "General Options/EnableTotalNumberOfTracks", "Tags/EnableTotalNumberOfTracks", QMetaType::Bool },
      { "General Options/GenreNotNumeric", "Tags/GenreNotNumeric", QMetaType::Bool },
      { "General Options/CommentName", "Tags/CommentName", QMetaType::QString },
      { "General Options/PictureNameItem", "Tags/PictureNameItem", QMetaType::Int },
      { "General Options/CustomGenres", "Tags/CustomGenres", QMetaType::QStringList },
      { "General Options/ID3v2Version", "Tags/ID3v2Version", QMetaType::Int },
      { "General Options/TextEncodingV1", "Tags/TextEncodingV1", QMetaType::QString },
      { "General Options/TextEncoding", "Tags/TextEncoding", QMetaType::Int },
      { "General Options/QuickAccessFrames", "Tags/QuickAccessFrames", QMetaType::UInt },
      { "General Options/TrackNumberDigits", "Tags/TrackNumberDigits", QMetaType::Int },
      { "General Options/OnlyCustomGenres", "Tags/OnlyCustomGenres", QMetaType::Bool },
      { "General Options/NameFilter3", "Files/NameFilter", QMetaType::QString },
      { "General Options/FormatItem", "Files/FormatItem", QMetaType::Int },
      { "General Options/FormatItems", "Files/FormatItems", QMetaType::QStringList },
      { "General Options/FormatText2", "Files/FormatText", QMetaType::QString },
      { "General Options/FormatFromFilenameItem", "Files/FormatFromFilenameItem", QMetaType::Int },
      { "General Options/FormatFromFilenameItems", "Files/FormatFromFilenameItems", QMetaType::QStringList },
      { "General Options/FormatFromFilenameText", "Files/FormatFromFilenameText", QMetaType::QString },
      { "General Options/PreserveTime", "Files/PreserveTime", QMetaType::Bool },
      { "General Options/MarkChanges", "Files/MarkChanges", QMetaType::Bool },
      { "General Options/LoadLastOpenedFile", "Files/LoadLastOpenedFile", QMetaType::Bool },
      { "General Options/LastOpenedFile", "Files/LastOpenedFile", QMetaType::QString },
      { "General Options/DefaultCoverFileName", "Files/DefaultCoverFileName", QMetaType::QString },
      { "General Options/DirFormatItem", "RenameDirectory/DirFormatItem", QMetaType::Int },
      { "General Options/DirFormatText", "RenameDirectory/DirFormatText", QMetaType::QString },
      { "General Options/RenameDirectorySource", "RenameDirectory/RenameDirectorySource", QMetaType::Int },
      { "General Options/NumberTracksDestination", "NumberTracks/NumberTracksDestination", QMetaType::Int },
      { "General Options/NumberTracksStartNumber", "NumberTracks/NumberTracksStartNumber", QMetaType::Int },
      { "General Options/AutoHideTags", "GUI/AutoHideTags", QMetaType::Bool },
      { "General Options/HideFile", "GUI/HideFile", QMetaType::Bool },
      { "General Options/HideV1", "GUI/HideV1", QMetaType::Bool },
      { "General Options/HideV2", "GUI/HideV2", QMetaType::Bool },
      { "General Options/HidePicture", "GUI/HidePicture", QMetaType::Bool },
      { "General Options/PlayOnDoubleClick", "GUI/PlayOnDoubleClick", QMetaType::Bool },
      { "General Options/SplitterSize0", "GUI/SplitterSize0", QMetaType::Int },
      { "General Options/SplitterSize1", "GUI/SplitterSize1", QMetaType::Int },
      { "General Options/VSplitterSize0", "GUI/VSplitterSize0", QMetaType::Int },
      { "General Options/VSplitterSize1", "GUI/VSplitterSize1", QMetaType::Int },
      { "General Options/UseProxy", "Network/UseProxy", QMetaType::Bool },
      { "General Options/Proxy", "Network/Proxy", QMetaType::QString },
      { "General Options/UseProxyAuthentication", "Network/UseProxyAuthentication", QMetaType::Bool },
      { "General Options/ProxyUserName", "Network/ProxyUserName", QMetaType::QString },
      { "General Options/ProxyPassword", "Network/ProxyPassword", QMetaType::QString },
      { "General Options/Browser", "Network/Browser", QMetaType::QString },
      { "General Options/ImportServer", "Import/ImportServer", QMetaType::Int },
      { "General Options/ImportDestination", "Import/ImportDestination", QMetaType::Int },
      { "General Options/ImportFormatNames", "Import/ImportFormatNames", QMetaType::QStringList },
      { "General Options/ImportFormatHeaders", "Import/ImportFormatHeaders", QMetaType::QStringList },
      { "General Options/ImportFormatTracks", "Import/ImportFormatTracks", QMetaType::QStringList },
      { "General Options/ImportFormatIdx", "Import/ImportFormatIdx", QMetaType::Int },
      { "General Options/EnableTimeDifferenceCheck", "Import/EnableTimeDifferenceCheck", QMetaType::Bool },
      { "General Options/MaxTimeDifference", "Import/MaxTimeDifference", QMetaType::Int },
      { "General Options/ImportVisibleColumns", "Import/ImportVisibleColumns", QMetaType::ULongLong },
      { "General Options/ImportWindowGeometry", "Import/ImportWindowGeometry", QMetaType::QByteArray },
      { "General Options/ImportTagsNames", "Import/ImportTagsNames", QMetaType::QStringList },
      { "General Options/ImportTagsSources", "Import/ImportTagsSources", QMetaType::QStringList },
      { "General Options/ImportTagsExtractions", "Import/ImportTagsExtractions", QMetaType::QStringList },
      { "General Options/ImportTagsIdx", "Import/ImportTagsIdx", QMetaType::Int },
      { "General Options/PictureSourceNames", "Import/PictureSourceNames", QMetaType::QStringList },
      { "General Options/PictureSourceUrls", "Import/PictureSourceUrls", QMetaType::QStringList },
      { "General Options/PictureSourceIdx", "Import/PictureSourceIdx", QMetaType::Int },
      { "General Options/MatchPictureUrlMapKeys", "Import/MatchPictureUrlMapKeys", QMetaType::QStringList },
      { "General Options/MatchPictureUrlMapValues", "Import/MatchPictureUrlMapValues", QMetaType::QStringList },
      { "General Options/BrowseCoverArtWindowGeometry", "Import/BrowseCoverArtWindowGeometry", QMetaType::QByteArray },
      { "General Options/ExportSourceV1", "Export/ExportSourceV1", QMetaType::Bool },
      { "General Options/ExportFormatNames", "Export/ExportFormatNames", QMetaType::QStringList },
      { "General Options/ExportFormatHeaders", "Export/ExportFormatHeaders", QMetaType::QStringList },
      { "General Options/ExportFormatTracks", "Export/ExportFormatTracks", QMetaType::QStringList },
      { "General Options/ExportFormatTrailers", "Export/ExportFormatTrailers", QMetaType::QStringList },
      { "General Options/ExportFormatIdx", "Export/ExportFormatIdx", QMetaType::Int },
      { "General Options/ExportWindowGeometry", "Export/ExportWindowGeometry", QMetaType::QByteArray }
    };
    for (const auto& mapping : mappings) {
      QStringList groupKey = QString::fromLatin1(mapping.oldKey)
          .split(QLatin1Char('/'));
      beginGroup(groupKey.at(0));
      if (contains(groupKey.at(1))) {
        QVariant val = value(groupKey.at(1), QVariant(QMetaType(mapping.type)));
        remove(groupKey.at(1));
        endGroup();
        groupKey = QString::fromLatin1(mapping.newKey)
            .split(QLatin1Char('/'));
        beginGroup(groupKey.at(0));
        setValue(groupKey.at(1), val);
        migrated = true;
      }
      endGroup();
    }
#else
    static const struct {
      const char* oldKey;
      const char* newKey;
      QVariant::Type type;
    } mappings[] = {
      { "Id3Format/FormatWhileEditing", "TagFormat/FormatWhileEditing", QVariant::Bool },
      { "Id3Format/CaseConversion", "TagFormat/CaseConversion", QVariant::Int },
      { "Id3Format/LocaleName", "TagFormat/LocaleName", QVariant::String },
      { "Id3Format/StrRepEnabled", "TagFormat/StrRepEnabled", QVariant::Bool },
      { "Id3Format/StrRepMapKeys", "TagFormat/StrRepMapKeys", QVariant::StringList },
      { "Id3Format/StrRepMapValues", "TagFormat/StrRepMapValues", QVariant::StringList },
      { "General Options/HideToolBar", "MainWindow/HideToolBar", QVariant::Bool },
      { "General Options/HideStatusBar", "MainWindow/HideStatusBar", QVariant::Bool },
      { "General Options/Geometry", "MainWindow/Geometry", QVariant::ByteArray },
      { "General Options/WindowState", "MainWindow/WindowState", QVariant::ByteArray },
      { "General Options/UseFont", "MainWindow/UseFont", QVariant::Bool },
      { "General Options/FontFamily", "MainWindow/FontFamily", QVariant::String },
      { "General Options/FontSize", "MainWindow/FontSize", QVariant::Int },
      { "General Options/Style", "MainWindow/Style", QVariant::String },
      { "General Options/DontUseNativeDialogs", "MainWindow/DontUseNativeDialogs", QVariant::Bool },
      { "General Options/MarkTruncations", "Tags/MarkTruncations", QVariant::Bool },
      { "General Options/EnableTotalNumberOfTracks", "Tags/EnableTotalNumberOfTracks", QVariant::Bool },
      { "General Options/GenreNotNumeric", "Tags/GenreNotNumeric", QVariant::Bool },
      { "General Options/CommentName", "Tags/CommentName", QVariant::String },
      { "General Options/PictureNameItem", "Tags/PictureNameItem", QVariant::Int },
      { "General Options/CustomGenres", "Tags/CustomGenres", QVariant::StringList },
      { "General Options/ID3v2Version", "Tags/ID3v2Version", QVariant::Int },
      { "General Options/TextEncodingV1", "Tags/TextEncodingV1", QVariant::String },
      { "General Options/TextEncoding", "Tags/TextEncoding", QVariant::Int },
      { "General Options/QuickAccessFrames", "Tags/QuickAccessFrames", QVariant::UInt },
      { "General Options/TrackNumberDigits", "Tags/TrackNumberDigits", QVariant::Int },
      { "General Options/OnlyCustomGenres", "Tags/OnlyCustomGenres", QVariant::Bool },
      { "General Options/NameFilter3", "Files/NameFilter", QVariant::String },
      { "General Options/FormatItem", "Files/FormatItem", QVariant::Int },
      { "General Options/FormatItems", "Files/FormatItems", QVariant::StringList },
      { "General Options/FormatText2", "Files/FormatText", QVariant::String },
      { "General Options/FormatFromFilenameItem", "Files/FormatFromFilenameItem", QVariant::Int },
      { "General Options/FormatFromFilenameItems", "Files/FormatFromFilenameItems", QVariant::StringList },
      { "General Options/FormatFromFilenameText", "Files/FormatFromFilenameText", QVariant::String },
      { "General Options/PreserveTime", "Files/PreserveTime", QVariant::Bool },
      { "General Options/MarkChanges", "Files/MarkChanges", QVariant::Bool },
      { "General Options/LoadLastOpenedFile", "Files/LoadLastOpenedFile", QVariant::Bool },
      { "General Options/LastOpenedFile", "Files/LastOpenedFile", QVariant::String },
      { "General Options/DefaultCoverFileName", "Files/DefaultCoverFileName", QVariant::String },
      { "General Options/DirFormatItem", "RenameDirectory/DirFormatItem", QVariant::Int },
      { "General Options/DirFormatText", "RenameDirectory/DirFormatText", QVariant::String },
      { "General Options/RenameDirectorySource", "RenameDirectory/RenameDirectorySource", QVariant::Int },
      { "General Options/NumberTracksDestination", "NumberTracks/NumberTracksDestination", QVariant::Int },
      { "General Options/NumberTracksStartNumber", "NumberTracks/NumberTracksStartNumber", QVariant::Int },
      { "General Options/AutoHideTags", "GUI/AutoHideTags", QVariant::Bool },
      { "General Options/HideFile", "GUI/HideFile", QVariant::Bool },
      { "General Options/HideV1", "GUI/HideV1", QVariant::Bool },
      { "General Options/HideV2", "GUI/HideV2", QVariant::Bool },
      { "General Options/HidePicture", "GUI/HidePicture", QVariant::Bool },
      { "General Options/PlayOnDoubleClick", "GUI/PlayOnDoubleClick", QVariant::Bool },
      { "General Options/SplitterSize0", "GUI/SplitterSize0", QVariant::Int },
      { "General Options/SplitterSize1", "GUI/SplitterSize1", QVariant::Int },
      { "General Options/VSplitterSize0", "GUI/VSplitterSize0", QVariant::Int },
      { "General Options/VSplitterSize1", "GUI/VSplitterSize1", QVariant::Int },
      { "General Options/UseProxy", "Network/UseProxy", QVariant::Bool },
      { "General Options/Proxy", "Network/Proxy", QVariant::String },
      { "General Options/UseProxyAuthentication", "Network/UseProxyAuthentication", QVariant::Bool },
      { "General Options/ProxyUserName", "Network/ProxyUserName", QVariant::String },
      { "General Options/ProxyPassword", "Network/ProxyPassword", QVariant::String },
      { "General Options/Browser", "Network/Browser", QVariant::String },
      { "General Options/ImportServer", "Import/ImportServer", QVariant::Int },
      { "General Options/ImportDestination", "Import/ImportDestination", QVariant::Int },
      { "General Options/ImportFormatNames", "Import/ImportFormatNames", QVariant::StringList },
      { "General Options/ImportFormatHeaders", "Import/ImportFormatHeaders", QVariant::StringList },
      { "General Options/ImportFormatTracks", "Import/ImportFormatTracks", QVariant::StringList },
      { "General Options/ImportFormatIdx", "Import/ImportFormatIdx", QVariant::Int },
      { "General Options/EnableTimeDifferenceCheck", "Import/EnableTimeDifferenceCheck", QVariant::Bool },
      { "General Options/MaxTimeDifference", "Import/MaxTimeDifference", QVariant::Int },
      { "General Options/ImportVisibleColumns", "Import/ImportVisibleColumns", QVariant::ULongLong },
      { "General Options/ImportWindowGeometry", "Import/ImportWindowGeometry", QVariant::ByteArray },
      { "General Options/ImportTagsNames", "Import/ImportTagsNames", QVariant::StringList },
      { "General Options/ImportTagsSources", "Import/ImportTagsSources", QVariant::StringList },
      { "General Options/ImportTagsExtractions", "Import/ImportTagsExtractions", QVariant::StringList },
      { "General Options/ImportTagsIdx", "Import/ImportTagsIdx", QVariant::Int },
      { "General Options/PictureSourceNames", "Import/PictureSourceNames", QVariant::StringList },
      { "General Options/PictureSourceUrls", "Import/PictureSourceUrls", QVariant::StringList },
      { "General Options/PictureSourceIdx", "Import/PictureSourceIdx", QVariant::Int },
      { "General Options/MatchPictureUrlMapKeys", "Import/MatchPictureUrlMapKeys", QVariant::StringList },
      { "General Options/MatchPictureUrlMapValues", "Import/MatchPictureUrlMapValues", QVariant::StringList },
      { "General Options/BrowseCoverArtWindowGeometry", "Import/BrowseCoverArtWindowGeometry", QVariant::ByteArray },
      { "General Options/ExportSourceV1", "Export/ExportSourceV1", QVariant::Bool },
      { "General Options/ExportFormatNames", "Export/ExportFormatNames", QVariant::StringList },
      { "General Options/ExportFormatHeaders", "Export/ExportFormatHeaders", QVariant::StringList },
      { "General Options/ExportFormatTracks", "Export/ExportFormatTracks", QVariant::StringList },
      { "General Options/ExportFormatTrailers", "Export/ExportFormatTrailers", QVariant::StringList },
      { "General Options/ExportFormatIdx", "Export/ExportFormatIdx", QVariant::Int },
      { "General Options/ExportWindowGeometry", "Export/ExportWindowGeometry", QVariant::ByteArray }
    };
    for (const auto& [oldKey, newKey, type] : mappings) {
      QStringList groupKey = QString::fromLatin1(oldKey)
          .split(QLatin1Char('/'));
      beginGroup(groupKey.at(0));
      if (contains(groupKey.at(1))) {
        QVariant val = value(groupKey.at(1), QVariant(type));
        remove(groupKey.at(1));
        endGroup();
        groupKey = QString::fromLatin1(newKey)
            .split(QLatin1Char('/'));
        beginGroup(groupKey.at(0));
        setValue(groupKey.at(1), val);
        migrated = true;
      }
      endGroup();
    }
#endif
    if (migrated) {
      qDebug("Migrated old settings");
    }
  }
}
