/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
/*
 * By committing to this project, you allow VideoLAN and VideoLabs to relicense
 * the code to a different OSI approved license, in case it is required for
 * compatibility with the Store
 *****************************************************************************/

#include "common.h"

#include <ctime>

#include "media_library_private.hpp"
#include "IVideoTrack.h"
#include "IAlbum.h"
#include "IAlbumTrack.h"
#include "IArtist.h"

media_item*
fileToMediaItem( MediaPtr file )
{
    auto type = MEDIA_ITEM_TYPE_UNKNOWN;
    switch ( file->type() )
    {
    case IMedia::Type::VideoType:
        type = MEDIA_ITEM_TYPE_VIDEO;
        break;
    case IMedia::Type::AudioType:
        type = MEDIA_ITEM_TYPE_AUDIO;
        break;
    default:
        LOGW( "Unknown file type: %d", file->type() );
        return nullptr;
    }
    auto mi = media_item_create( file->mrl().c_str(), type );
    if ( mi == nullptr )
    {
        //FIXME: What should we do? This won't be run again until the next time
        //we restore the media library. Also, do we care? This is likely E_NOMEM, so we
        //might have bigger problems than a missing file...
        LOGE( "Failed to create media_item for file %s", file->mrl().c_str() );
        return nullptr;
    }
    mi->i_id = file->id();
    media_item_set_meta(mi, MEDIA_ITEM_META_TITLE, file->title().c_str());

    mi->i_duration = file->duration();
    if ( file->type() == IMedia::Type::VideoType )
    {
        auto vtracks = file->videoTracks();
        if ( vtracks.size() != 0 )
        {
            if ( vtracks.size() > 1 )
                LOGW( "Ignoring file [%s] extra video tracks for file description", file->mrl().c_str() );
            auto vtrack = vtracks[0];
            mi->i_w = vtrack->width();
            mi->i_h = vtrack->height();
        }
        if (file->snapshot().length() > 0)
            mi->psz_snapshot = strdup(file->snapshot().c_str());
    }
    else if ( file->type() == IMedia::Type::AudioType )
    {
        auto albumTrack = file->albumTrack();
        if (albumTrack != nullptr)
        {
            auto album = albumTrack->album();
            if (album != nullptr)
            {
                media_item_set_meta(mi, MEDIA_ITEM_META_ALBUM, album->title().c_str());
                auto date = album->releaseDate();
                if (date != 0)
                {
                    auto t = tm{};
                    if (gmtime_r(&date, &t) != NULL)
                    {
                        media_item_set_meta(mi, MEDIA_ITEM_META_YEAR, std::to_string(t.tm_year).c_str());
                    }
                }
                auto artwork = album->artworkUrl();
                if ( artwork.size() > 0 && artwork.compare( 0, strlen("file://"), "file://" ) == 0 )
                {
                    mi->psz_snapshot = strdup( artwork.c_str() + strlen( "file://" ) );
                }
            }
        }
        auto artist = file->artist();
        if (artist.length() > 0)
            media_item_set_meta(mi, MEDIA_ITEM_META_ARTIST, artist.c_str());
    }
    return mi;
}

album_item*
albumToAlbumItem( AlbumPtr album )
{
    auto p_item = album_item_create(album->title().c_str());
    if (p_item == nullptr)
        return nullptr;
    p_item->i_release_date = album->releaseDate();
    return p_item;
}


artist_item*
artistToArtistItem( ArtistPtr artist )
{
    auto p_item = artist_item_create(artist->name().c_str());
    if (p_item == nullptr)
        return nullptr;
    return p_item;
}
