#pragma once
#ifndef _LYRIC_DECODER
#define _LYRIC_DECODER
#include "decoder_types.h"
#include "album.h"
#include "utility.h"
#include "NetRequestsWin.h"
#include <sstream>
#include <codecvt>
#include <algorithm>


namespace LyricsUtil
{
    /**
     * Converts std::string to utf-8 encoded std::wstring
     * @param str - string to convert.
     * @returns utf-8 encoded string
     */
    static std::wstring UTF8ToWstring(const std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(str);
    }

    /**
     * Converts wstring to utf-8 std::string
     * @param str - wstring to convert.
     * @returns utf-8 encoded string
     */
    static std::string WstringToUTF8(const std::wstring& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(str);
    }

    static std::string SearchForAlbumLyrics(
        const std::string& host,
        const std::string& artist,
        const std::string& album)
    {
        const std::string requestURL{ host + "/search?q=" + FormatString(artist) };
        std::string data{ NetRequestsWin::Get(requestURL) };
        if (data != "failed")
        {
            std::string line;
            std::string uri;
            std::stringstream sstream{ data };
            while (std::getline(sstream, line))
            {
                if (ToLower(line).find(ToLower(FormatString(artist))) != std::string::npos)
                {
                    size_t start{ line.find_first_of('\"') + 1 };
                    size_t end{ line.find_first_of('\"', start) };
                    uri = line.substr(start, end - start);
                }
            }
            data = NetRequestsWin::Get(host + "/" + uri);
        }
        return data;
    }

    /**
     * Tries to fetch and decode from different sites until a match is found.
     * @TODO: Implement a vector to iterate through containing all decoders.
     * @param artist - Artist name
     * @param album  - Album name
     * @param struct_album - Album struct to be created
     */
    static void TryDecode(const std::string& artist, const std::string& album, Album& struct_album)
    {
		// When more decoders are implemented store all in a vector and iterate through.
        if (DarkLyricsDecoder().DecodeLyrics(artist, album, struct_album))
            return;
        else if (SongLyricsDecoder().DecodeLyrics(artist, album, struct_album))
            return;
    }
}

#endif /* _LYRIC_DECODER */