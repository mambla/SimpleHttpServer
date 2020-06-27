#include "PathIdentifier.h"
#include <sstream>
#include <string>
#include "DirectoryIterator.h"
#include "Utils.h"

const PathIdentifier::StringBuffer PathIdentifier::_default_empty_data = StringBuffer();

PathIdentifier::PathIdentifier(const std::wstring& abs_path, DWORD max_size_for_data)
:_max_size_for_data(max_size_for_data),
_abs_path(abs_path)		
{
}

PathIdentifier::~PathIdentifier()
{

}

PathIdentifier::PathAttribute PathIdentifier::get_path_attribute()const
{
	
	DWORD attribute_identifer = GetFileAttributesW(_abs_path.c_str());
	if (INVALID_FILE_ATTRIBUTES == attribute_identifer)
	{
		return PathAttribute::None;
	}

	else if (FILE_ATTRIBUTE_DIRECTORY & attribute_identifer)
	{
		return PathAttribute::Directory;
	}
	else // is a file
	{
		return PathAttribute::File;
	}
}

PathIdentifier::HandlersMapping PathIdentifier::get_path_handlers()const
{
	HandlersMapping mapping_handlers;
	mapping_handlers.insert({ PathAttribute::File, &PathIdentifier::file_handler});
	mapping_handlers.insert({ PathAttribute::Directory, &PathIdentifier::directory_handler});
	mapping_handlers.insert({ PathAttribute::None, &PathIdentifier::defualt_handler});
	return mapping_handlers;
}

HANDLE PathIdentifier::get_file_hanler()const // for CR maker: should I return here a unique ptr that usese Win32 API "CloseHandle" function ?
{
	HANDLE hfile = CreateFileW(_abs_path.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return hfile;
}

PathIdentifier::StringBuffer PathIdentifier::defualt_handler() const
{
	return _default_empty_data;
}

PathIdentifier::StringBuffer PathIdentifier::file_handler() const
{
	static const LPOVERLAPPED DONT_USE_OVERLLAPED = NULL;

	const DWORD chunk_size = _max_size_for_data / 10;
	DWORD bytes_read = 0;
	DWORD total_bytes_read = 0;
	StringBuffer buffer(_max_size_for_data);
	BOOL status = TRUE;
	HANDLE hfile = get_file_hanler();

	while (
		(total_bytes_read < _max_size_for_data)
		&& status)
	{
		status = ReadFile(
			hfile,
			buffer.data() + total_bytes_read,
			chunk_size,
			&bytes_read,
			DONT_USE_OVERLLAPED);

		if (!status || !bytes_read)
		{
			break;
		}
		total_bytes_read += bytes_read;
	}

	buffer.resize(total_bytes_read);
	CloseHandle(hfile);

	if (status) //why did we get out of the loop
	{
		return buffer;
	}

	else
	{
		return _default_empty_data;
	}
}

PathIdentifier::StringBuffer PathIdentifier::directory_handler() const
{
	static unsigned int SIZE_OF_NEW_LINE_CHARACTER = 2;
	unsigned int total_bytes_read = 0;
	StringBuffer buffer(_max_size_for_data);
	std::wostringstream path_builder;
	unsigned long offset_to_append = 0;
	unsigned long size_going_to_be_written = 0;

	path_builder << _abs_path << L"\\*";
	DirectroyIterator iterator(path_builder.str());

	while (iterator.has_next() && (total_bytes_read < _max_size_for_data))
	{
		std::wstring file_name = iterator.get_next();
		size_going_to_be_written = 
			file_name.size() * sizeof(WCHAR)
			+ SIZE_OF_NEW_LINE_CHARACTER;

		if (size_going_to_be_written + total_bytes_read > _max_size_for_data)
		{
			break;
		}

		offset_to_append = total_bytes_read;
		add_string_to_vector(file_name, buffer, offset_to_append);

		offset_to_append = total_bytes_read + file_name.size() * sizeof(WCHAR);
		add_string_to_vector(L"\n", buffer, offset_to_append);
		total_bytes_read += size_going_to_be_written;
	
	}

	buffer.resize(total_bytes_read);
	return buffer;
}

PathIdentifier::StringBufferPtr PathIdentifier::read_now() const
{
	PathIdentifier::PathAttribute path_attribute = get_path_attribute();
	HandlersMapping path_handlers_mapping = get_path_handlers();
	MemberFunctionPathHandler selected_handler = path_handlers_mapping[path_attribute];
	StringBuffer result_buffer = (this->*selected_handler)();
	return std::make_shared<StringBuffer>(result_buffer);
}

