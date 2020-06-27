#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <map>


static DWORD DEFAULT_READ_SIZE = 2028;

class PathIdentifier
{
public:
	using StringBuffer = std::vector<char>;
	using StringBufferPtr = std::shared_ptr<StringBuffer>;
	enum class PathAttribute { Directory, File, None };
	using MemberFunctionPathHandler = StringBuffer (PathIdentifier::*)(void) const;
	using HandlersMapping = std::map<PathAttribute, MemberFunctionPathHandler>;

public:
	PathIdentifier(const std::wstring& abs_path, DWORD max_size_for_data = DEFAULT_READ_SIZE);
	~PathIdentifier();

public:
	PathIdentifier(const PathIdentifier& other)=delete;
	PathIdentifier& operator=(const PathIdentifier& other)=delete;
	PathIdentifier(PathIdentifier&& other) = delete;
	PathIdentifier& operator=(PathIdentifier&& other) = delete;

public:
	StringBufferPtr read_now()const;

private:
	PathAttribute get_path_attribute() const;
	HandlersMapping get_path_handlers() const;
	HANDLE get_file_hanler() const;
	PathIdentifier::StringBuffer defualt_handler() const;
	StringBuffer file_handler()const;
	StringBuffer directory_handler()const;
	

	const std::wstring _abs_path;
	const unsigned int _max_size_for_data;
	static const StringBuffer _default_empty_data;
};



