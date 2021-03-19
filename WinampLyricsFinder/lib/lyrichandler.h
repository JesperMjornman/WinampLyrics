#pragma once
#ifndef _LYRIC_HANDLER
#define _LYRIC_HANDLER 

#include "NetRequestsWin.h"
#include "utility.h"
#include "lyricdecoder.h"
#include "album.h"

#include <string>
#include <unordered_map>

/**
 * \brief 
 * 
 * Represents a handler for fetching and saving lyrics from a fetched album.
 * 
 */
class LyricHandler
{
public:
	/**
	 * Constructor
	 */
	LyricHandler();
	
	/**
	 * Fetch lyrics of "artist" and "album" through any given function pointer. 
	 * Note that the given decoder also serves as the handler for fetching lyrics.
	 * @param artist  - Artist name
	 * @param album   - Album name
	 * @param decoder - Callable function pointer. Must accept two (w)strings "artist" and "album"
	 */
	template<class pFunc>
	typename std::enable_if<is_function_ptr<pFunc>::value>::type
	GetLyrics(const std::string& artist, const std::string& album, pFunc decoder);
	
	/**
	 * Fetch lyrics from a specific song without setting the Album lyrics. 
	 * Defined as some sites doesn't allow for album searches.
	 * 
	 * @param artist  - Artist name
	 * @param song    - Song name
	 * @param decoder - Callable function pointer. Must accept two (w)strings "artist" and "album"
	 * @returns Song lyrics
	 */
	template<class pFunc, typename std::enable_if<is_function_ptr<pFunc>::value>::type>
	std::string GetSongLyrics(const std::string& artist, const std::string& song, pFunc decoder);

	/**
	 * Fetch lyrics from a specific song without setting the Album lyrics.
	 * Defined as some sites doesn't allow for album searches.
	 *
	 * @param artist  - Artist name
	 * @param song    - Song name
	 * @param decoder - Callable function pointer. Must accept two (w)strings "artist" and "album"
	 * @returns Song lyrics
	 */
	template<class pFunc, typename std::enable_if<is_function_ptr<pFunc>::value>::type>
	std::wstring GetSongLyrics(const std::wstring& artist, const std::wstring& song, pFunc decoder);

	/**
	 * Returns reference to song from album through index of hashmap
	 * @param s - song name present in albums hashmap.
	 */
	std::wstring& operator[](std::wstring& s); 
	std::wstring& operator[](std::string& s);
	std::wstring& operator[](const wchar_t* s);
	std::wstring& operator[](const char* s);

	const std::pair<std::wstring, bool> GetInterval(const std::wstring& song, const int start, const int count) const;

	const LyricsUtil::Album& GetAlbum() const noexcept;
	const size_t GetSize() const noexcept;
	const int GetLineCount(const std::string& s)  const noexcept;
	const int GetLineCount(const std::wstring& s) const noexcept;

	friend std::wostream& operator<<(std::wostream& os, const LyricHandler& lh);
protected:	
	LyricsUtil::Album album;
};

template<class pFunc>
inline typename std::enable_if<is_function_ptr<pFunc>::value>::type 
LyricHandler::GetLyrics(const std::string& artist, 
						const std::string& album,
						pFunc decoder) 
{
	decoder(artist, album, this->album);
}

#endif