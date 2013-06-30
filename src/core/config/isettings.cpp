/**
 * \file isettings.h
 * Interface for application settings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Apr 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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
    static const struct {
      const char* oldKey;
      const char* newKey;
    } mappings[] = {
      { "Id3Format/FormatWhileEditing", "TagFormat/FormatWhileEditing" },
      { "Id3Format/CaseConversion", "TagFormat/CaseConversion" },
      { "Id3Format/LocaleName", "TagFormat/LocaleName" },
      { "Id3Format/StrRepEnabled", "TagFormat/StrRepEnabled" },
      { "Id3Format/StrRepMapKeys", "TagFormat/StrRepMapKeys" },
      { "Id3Format/StrRepMapValues", "TagFormat/StrRepMapValues" },
      { "General Options/HideToolBar", "MainWindow/HideToolBar" },
      { "General Options/HideStatusBar", "MainWindow/HideStatusBar" },
      { "General Options/Geometry", "MainWindow/Geometry" },
      { "General Options/WindowState", "MainWindow/WindowState" },
      { "General Options/UseFont", "MainWindow/UseFont" },
      { "General Options/FontFamily", "MainWindow/FontFamily" },
      { "General Options/FontSize", "MainWindow/FontSize" },
      { "General Options/Style", "MainWindow/Style" },
      { "General Options/DontUseNativeDialogs", "MainWindow/DontUseNativeDialogs" },
      { "General Options/MarkTruncations", "Tags/MarkTruncations" },
      { "General Options/EnableTotalNumberOfTracks", "Tags/EnableTotalNumberOfTracks" },
      { "General Options/GenreNotNumeric", "Tags/GenreNotNumeric" },
      { "General Options/CommentName", "Tags/CommentName" },
      { "General Options/PictureNameItem", "Tags/PictureNameItem" },
      { "General Options/CustomGenres", "Tags/CustomGenres" },
      { "General Options/ID3v2Version", "Tags/ID3v2Version" },
      { "General Options/TextEncodingV1", "Tags/TextEncodingV1" },
      { "General Options/TextEncoding", "Tags/TextEncoding" },
      { "General Options/QuickAccessFrames", "Tags/QuickAccessFrames" },
      { "General Options/TrackNumberDigits", "Tags/TrackNumberDigits" },
      { "General Options/OnlyCustomGenres", "Tags/OnlyCustomGenres" },
      { "General Options/NameFilter3", "Files/NameFilter" },
      { "General Options/FormatItem", "Files/FormatItem" },
      { "General Options/FormatItems", "Files/FormatItems" },
      { "General Options/FormatText2", "Files/FormatText" },
      { "General Options/FormatFromFilenameItem", "Files/FormatFromFilenameItem" },
      { "General Options/FormatFromFilenameItems", "Files/FormatFromFilenameItems" },
      { "General Options/FormatFromFilenameText", "Files/FormatFromFilenameText" },
      { "General Options/PreserveTime", "Files/PreserveTime" },
      { "General Options/MarkChanges", "Files/MarkChanges" },
      { "General Options/LoadLastOpenedFile", "Files/LoadLastOpenedFile" },
      { "General Options/LastOpenedFile", "Files/LastOpenedFile" },
      { "General Options/DefaultCoverFileName", "Files/DefaultCoverFileName" },
      { "General Options/DirFormatItem", "RenameDirectory/DirFormatItem" },
      { "General Options/DirFormatText", "RenameDirectory/DirFormatText" },
      { "General Options/RenameDirectorySource", "RenameDirectory/RenameDirectorySource" },
      { "General Options/NumberTracksDestination", "NumberTracks/NumberTracksDestination" },
      { "General Options/NumberTracksStartNumber", "NumberTracks/NumberTracksStartNumber" },
      { "General Options/AutoHideTags", "GUI/AutoHideTags" },
      { "General Options/HideFile", "GUI/HideFile" },
      { "General Options/HideV1", "GUI/HideV1" },
      { "General Options/HideV2", "GUI/HideV2" },
      { "General Options/HidePicture", "GUI/HidePicture" },
      { "General Options/PlayOnDoubleClick", "GUI/PlayOnDoubleClick" },
      { "General Options/SplitterSize0", "GUI/SplitterSize0" },
      { "General Options/SplitterSize1", "GUI/SplitterSize1" },
      { "General Options/VSplitterSize0", "GUI/VSplitterSize0" },
      { "General Options/VSplitterSize1", "GUI/VSplitterSize1" },
      { "General Options/UseProxy", "Network/UseProxy" },
      { "General Options/Proxy", "Network/Proxy" },
      { "General Options/UseProxyAuthentication", "Network/UseProxyAuthentication" },
      { "General Options/ProxyUserName", "Network/ProxyUserName" },
      { "General Options/ProxyPassword", "Network/ProxyPassword" },
      { "General Options/Browser", "Network/Browser" },
      { "General Options/ImportServer", "Import/ImportServer" },
      { "General Options/ImportDestination", "Import/ImportDestination" },
      { "General Options/ImportFormatNames", "Import/ImportFormatNames" },
      { "General Options/ImportFormatHeaders", "Import/ImportFormatHeaders" },
      { "General Options/ImportFormatTracks", "Import/ImportFormatTracks" },
      { "General Options/ImportFormatIdx", "Import/ImportFormatIdx" },
      { "General Options/EnableTimeDifferenceCheck", "Import/EnableTimeDifferenceCheck" },
      { "General Options/MaxTimeDifference", "Import/MaxTimeDifference" },
      { "General Options/ImportVisibleColumns", "Import/ImportVisibleColumns" },
      { "General Options/ImportWindowGeometry", "Import/ImportWindowGeometry" },
      { "General Options/ImportTagsNames", "Import/ImportTagsNames" },
      { "General Options/ImportTagsSources", "Import/ImportTagsSources" },
      { "General Options/ImportTagsExtractions", "Import/ImportTagsExtractions" },
      { "General Options/ImportTagsIdx", "Import/ImportTagsIdx" },
      { "General Options/PictureSourceNames", "Import/PictureSourceNames" },
      { "General Options/PictureSourceUrls", "Import/PictureSourceUrls" },
      { "General Options/PictureSourceIdx", "Import/PictureSourceIdx" },
      { "General Options/MatchPictureUrlMapKeys", "Import/MatchPictureUrlMapKeys" },
      { "General Options/MatchPictureUrlMapValues", "Import/MatchPictureUrlMapValues" },
      { "General Options/BrowseCoverArtWindowGeometry", "Import/BrowseCoverArtWindowGeometry" },
      { "General Options/ExportSourceV1", "Export/ExportSourceV1" },
      { "General Options/ExportFormatNames", "Export/ExportFormatNames" },
      { "General Options/ExportFormatHeaders", "Export/ExportFormatHeaders" },
      { "General Options/ExportFormatTracks", "Export/ExportFormatTracks" },
      { "General Options/ExportFormatTrailers", "Export/ExportFormatTrailers" },
      { "General Options/ExportFormatIdx", "Export/ExportFormatIdx" },
      { "General Options/ExportWindowGeometry", "Export/ExportWindowGeometry" }
    };
    for (unsigned int i = 0; i < sizeof(mappings) / sizeof(mappings[0]); ++i) {
      QStringList groupKey = QString::fromLatin1(mappings[i].oldKey).
          split(QLatin1Char('/'));
      beginGroup(groupKey.at(0));
      if (contains(groupKey.at(1))) {
        QVariant val = value(groupKey.at(1), QVariant());
        remove(groupKey.at(1));
        endGroup();
        groupKey = QString::fromLatin1(mappings[i].newKey).
            split(QLatin1Char('/'));
        beginGroup(groupKey.at(0));
        setValue(groupKey.at(1), val);
        migrated = true;
      }
      endGroup();
    }
    if (migrated) {
      qDebug("Migrated old settings");
    }
  }
}
