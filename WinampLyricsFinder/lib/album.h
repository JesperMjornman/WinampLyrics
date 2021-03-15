#pragma once
#include <unordered_map>
namespace LyricsUtil
{
	struct Album
	{
		Album() : name{ }, songs{ } 
		{
		}

		Album(std::wstring name, 
			  std::unordered_map<std::wstring, std::wstring> songs) 
			: name{ name }, songs{ songs } 
		{
		}

		std::wstring name;
		std::unordered_map<std::wstring, std::wstring> songs;
	};
}