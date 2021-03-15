#pragma once
#ifndef _SONG_LYRICS_DECODER_
#define _SONG_LYRICS_DECODER_
#include "decoder.h"
class SongLyricsDecoder : public Decoder
{
public:
	SongLyricsDecoder() = default;
	/**
	 * Sends GET request to www.songlyrics.com for the given information about artist and album.
	 * Calls DecodeLyrics2 for actual decoding.
	 * @param artist - Artist name
	 * @param album  - Album name
	 * @param struct_album - Album struct to be created
	 */
	bool DecodeLyrics(const std::string& artist, const std::string& album, LyricsUtil::Album& struct_album) override;
private:
	/**
	 * Decodes retreived GET respone from www.darklyrics.com
	 * UTF-8 encodes into wstring and creates an Album.
	 * @param data         - Data to be decoded
	 * @param struct_album - Album to edit/create
	 */
	void DecodeLyrics2(const std::string& data, LyricsUtil::Album& struct_album) override;
};
#endif
