/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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

#include <mutex>

#include "IAlbum.h"
#include "IMedia.h"
#include "ILogger.h"
#include "media_library.hpp"
#include "media/media_item.h"
#include "media/album_item.h"
#include "media/artist_item.h"
#include "media/genre_item.h"
#include "media/playlist_item.h"

media_item* fileToMediaItem( MediaPtr file );
album_item* albumToAlbumItem( AlbumPtr album );
artist_item* artistToArtistItem( ArtistPtr album );
genre_item* genreToGenreItem( GenrePtr genre );
playlist_item* playlistToPlaylistItem( PlaylistPtr playlist );

class TizenLogger : public ILogger
{
    virtual void Error( const std::string& msg ) override
    {
        dlog_print( DLOG_ERROR, "medialibrary", msg.c_str() );
    }

    virtual void Warning( const std::string& msg ) override
    {
        dlog_print( DLOG_WARN, "medialibrary", msg.c_str() );
    }

    virtual void Info( const std::string& msg ) override
    {
        dlog_print( DLOG_INFO, "medialibrary", msg.c_str() );
    }

    virtual void Debug( const std::string& msg ) override
    {
        dlog_print( DLOG_DEBUG, "medialibrary", msg.c_str() );
    }
};

struct media_library : public IMediaLibraryCb
{
public:
    media_library();
    virtual ~media_library() = default;

    // IMediaLibraryCb
    virtual void onMediaAdded( std::vector<MediaPtr> media ) override;
    virtual void onMediaUpdated( std::vector<MediaPtr> media ) override;
    virtual void onMediaDeleted( std::vector<int64_t> media ) override;

    virtual void onArtistsAdded( std::vector<ArtistPtr> artists ) override;
    virtual void onArtistsModified( std::vector<ArtistPtr> artist ) override;
    virtual void onArtistsDeleted( std::vector<int64_t> ids ) override;

    virtual void onAlbumsAdded( std::vector<AlbumPtr> albums ) override;
    virtual void onAlbumsModified( std::vector<AlbumPtr> albums ) override;
    virtual void onAlbumsDeleted( std::vector<int64_t> ids ) override;

    virtual void onTracksAdded( std::vector<AlbumTrackPtr> tracks ) override;
    virtual void onTracksDeleted( std::vector<int64_t> trackIds ) override;

    virtual void onDiscoveryStarted( const std::string& entryPoint ) override;
    virtual void onDiscoveryCompleted( const std::string& entryPoint ) override;

    virtual void onReloadStarted( const std::string& entryPoint ) override;
    virtual void onReloadCompleted( const std::string& entryPoint ) override;

    virtual void onParsingStatsUpdated( uint32_t percent ) override;

    void registerOnChange(media_library_file_list_changed_cb cb, void* cbUserData);
    void unregisterOnChange(media_library_file_list_changed_cb cb, void* cbUserData);

    void registerOnItemUpdated(media_library_item_updated_cb cb, void* userData);
    void unregisterOnItemUpdated(media_library_item_updated_cb cb, void* userData);

    void registerProgressCb( media_library_scan_progress_cb pf_progress, void* p_data );

public:
    // Logger needs to be before ml, since ml will take a raw pointer to the logger.
    // yes, it sucks, but unique_ptr is too restrictive, and shared_ptr is overkill.
    // I'll make up my mind someday, I promise.
    std::unique_ptr<TizenLogger> logger;
    std::shared_ptr<IMediaLibrary> ml;

private:
    void sendFileUpdate( MediaPtr item, bool added );

private:
    struct FileUpdateCallbackCtx
    {
        FileUpdateCallbackCtx(media_library* _ml, media_item* _item, bool _added)
            : ml(_ml), wml(ml->ml), item(_item, media_item_destroy), added(_added) {}
        media_library* ml;
        std::weak_ptr<IMediaLibrary> wml;
        std::unique_ptr<media_item, void(*)(media_item*)> item;
        // Used to monitor media_library's lifetime.
        bool added;
    };

    struct ProgressUpdateCallbackCtx
    {
        ProgressUpdateCallbackCtx(media_library* ml, uint8_t percent)
            : ml( ml )
            , wml( ml->ml )
            , percent( percent )
        {
        }
        media_library* ml;
        std::weak_ptr<IMediaLibrary> wml;
        uint8_t percent;
    };

private:
    std::vector<std::pair<media_library_file_list_changed_cb, void*>> m_onChangeCb;
    std::vector<std::pair<media_library_item_updated_cb, void*>> m_onItemUpdatedCb;
    media_library_scan_progress_cb m_progressCb;
    void* m_progressData;
};
