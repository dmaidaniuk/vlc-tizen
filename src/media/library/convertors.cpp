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

static char*
path_from_url(const char* psz_str)
{
    if (psz_str == NULL || *psz_str == 0)
        return NULL;
    if (strncmp(psz_str, "file://", 7) == 0)
        psz_str += 7;
    size_t len = strlen(psz_str);
    char* psz_res;
    char* psz_dest = psz_res = (char*)malloc((len + 1) * sizeof(*psz_dest));

    while (*psz_str)
    {
        if (*psz_str == '%' && psz_str[1] != 0 && psz_str[2] != 0)
        {
            sscanf(psz_str + 1,"%02x",(int*)psz_dest);
            ++psz_dest;
            psz_str += 3;
        }
        else
        {
            *psz_dest++ = *psz_str++;
        }
    }
    *psz_dest = 0;
    return psz_res;
}


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
                auto year = albumTrack->releaseYear();
                if (year != 0)
                {
                    media_item_set_meta(mi, MEDIA_ITEM_META_YEAR, std::to_string(year).c_str());
                }
                auto artwork = album->artworkUrl();
                mi->psz_snapshot = path_from_url(artwork.c_str());
            }
            mi->i_track_number = albumTrack->trackNumber();
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
    p_item->i_id = album->id();
    p_item->i_nb_tracks = album->nbTracks();
    p_item->psz_artwork = path_from_url(album->artworkUrl().c_str());
    return p_item;
}

artist_item*
artistToArtistItem( ArtistPtr artist )
{
    auto p_item = artist_item_create(artist->name().c_str());
    if (p_item == nullptr)
        return nullptr;
    if (artist->artworkUrl().empty() == false)
        p_item->psz_artwork = path_from_url( artist->artworkUrl().c_str() );

    auto albums = artist->albums();
    p_item->i_nb_albums = albums.size();
    return p_item;
}
