#pragma once
#include <string>
#include <vector>
#include <windows.h>


class DirectroyIterator {
public:
	using StringBuffer = std::wstring;

public:
	DirectroyIterator(const std::wstring& path);
	~DirectroyIterator();

	bool has_next()const;
	StringBuffer get_next();

public:
	DirectroyIterator(const DirectroyIterator& other) = delete;
	DirectroyIterator( DirectroyIterator&& other) = delete;
	DirectroyIterator& operator=(const DirectroyIterator& other) = delete;
	DirectroyIterator& operator=(DirectroyIterator&& other) = delete;

private:
	HANDLE get_find_handler(const std::wstring& path);

private:
	HANDLE _hfind;
	WIN32_FIND_DATAW _next_file; //we had to keep this as a member because of the first call to "has_next". the method to know whether there is a next file, you first need to query for the "First File". that is, we have to load it be before the first call to has_next. 
	
	bool _has_next;

};
