#pragma once
#ifndef _NET_REQUESTS_WINDOWS
#define _NET_REQUESTS_WINDOWS
#include<string>
#include<unordered_map>

struct Response
{
	int code;
	std::string desc;
};

class Header
{
public:
	Header() : code{}, desc{}, data{} {}
	
	int code;
	std::string desc;

	std::string& operator[](std::string item) { return data[item]; }

	const std::string& Get(const std::string item) const noexcept;
	std::string& Get(const std::string item) noexcept;
private:
	std::unordered_map<std::string, std::string> data;
};

class NetRequestsWin
{
public:
	static std::string Get(const std::string& url, bool redirect = false);
	static inline std::string GetHostPath(std::string url);
	static inline std::string GetURIPath(std::string url);
	static inline Response GetResponse(const std::string& data);
	static inline Header GetHeader(const std::string& data);
	//static inline std::unordered_map<std::string, std::string> GetHeader(const std::string& data);
};

#endif