#pragma once
#ifndef _LYRICS_UTILITY
#define _LYRICS_UTILITY
#include <type_traits>
#include <utility>
#include <string>
#include <algorithm>

template <typename Fun>
struct is_function_ptr 
	: std::integral_constant<bool, std::is_pointer<Fun>::value 
							&& std::is_function<typename std::remove_pointer<Fun>::type>
							::value>
{};

static std::wstring ToLower(const std::wstring& s)
{
    std::wstring transformed{ s };
    std::transform(transformed.begin(), transformed.end(), transformed.begin(),
        [](wchar_t c) { return std::tolower(c); });

    return transformed;
}

static std::string ToLower(const std::string& s)
{
    std::string transformed{ s };
    std::transform(transformed.begin(), transformed.end(), transformed.begin(),
        [](char c) { return std::tolower(c); });

    return transformed;
}


static std::string FormatString(const std::string& str, bool isAlbumName = false, const char whiteCharReplacement = 0)
{
    std::string formatted{};
    bool parentheses = false;
    // Format string, replace bad characters.
    for (char c : str)
    {
        if (c > 47 && (c < 91 || c > 96 || c < 123)
            && !(c < 65 && c > 57))
        {
            if(!parentheses)
                formatted += c > 90 ? c : c + 32;
        }
        else if (c == '(' && isAlbumName)
        {
            parentheses = true;
            continue;
        }
        else if (c == ')' && parentheses)
        {
            parentheses = false;
            continue;
        }
        else if (c == ' ' && whiteCharReplacement > 0)
        {
            formatted += whiteCharReplacement;
        }
        else if (!isAlbumName && c < 0) // ISO-8859-1
        {
            switch (c)
            {
            case 128:
            case 135:
                formatted += 'c';
                break;
            case 129:
            case 150:
            case 151:
            case 154:
            case 163:
                formatted += 'u';
                break;
            case 130:
            case 136:
            case 137:
            case 138:
            case 144:
                formatted += 'e';
                break;
            case -23:
            case -24:
            case -25:
            case -26:
            case -27:
            case -28:
                formatted += 'a';
                break;
            case 139:
            case 140:
            case 141:
                formatted += 'i';
                break;
            case -6:
            case -7:
            case -8:
            case -9:
            case -10:
                formatted += 'o';
                break;
            case 164:
            case 165:
                formatted += 'n';
                break;
            default:
                break;

            }
        }
    }
    return formatted;
}

/**
 * Splits a string from the given separator. 
 * Note that the separator works best if it is one character.
 */
static inline std::vector<std::string> Split(const std::string& str, const std::string separator) noexcept
{
    std::vector<std::string> result{};
    if (str.length() == 0)
        return result;

    size_t uOffset{};
    size_t uTmp{};
    try
    {
        do
        {
            uTmp = uOffset;
            uOffset = str.find_first_of(separator, uOffset);
            if (uOffset == std::string::npos)
                uOffset = str.length();

            uOffset += 1;
            std::string s = str.substr(uTmp, uOffset - uTmp - 1);

            if (s[0] == ' ')
            {
                size_t uNonWhiteLocation = s.find_first_not_of(' ');
                if (uNonWhiteLocation != std::string::npos)
                    s = s.substr(uNonWhiteLocation);
            }

            if (s.length() > 1 && s[s.length() - 1] == ' ')
            {
                s = s.substr(0, s.length() - 1);
                //s = s.substr(0, s.find)
            }

            result.push_back(s);
        } while (uOffset != std::string::npos && uOffset < str.length());
    }
    catch (std::exception& e)
    {
        return std::vector<std::string>{ "except" }; 
    }
    return result;
}

#endif