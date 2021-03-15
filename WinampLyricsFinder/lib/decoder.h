#pragma once
#include <string>
#include "album.h"

class Decoder
{
public:
	virtual bool DecodeLyrics(const std::string& artist, const std::string& album, LyricsUtil::Album& struct_album) = 0;
protected:
	virtual void DecodeLyrics2(const std::string& data, LyricsUtil::Album& struct_album) = 0;
};

