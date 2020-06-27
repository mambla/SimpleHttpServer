#include "FileHandler.h"

inline FileHandlerBase::FileHandlerBase()
	:_hfile(INVALID_HANDLE_VALUE),
	_permissions(GENERIC_READ | GENERIC_WRITE)
{
}

FileHandlerBase::FileHandlerBase(const std::wstring& file_path)

	: FileHandlerBase()
{
	_hfile = CreateFileW(file_path.c_str(),
		_permissions,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
}


HANDLE FileHandlerBase::get_file_handler() const
{
	return _hfile;
}

FileHandlerBase::~FileHandlerBase()
{
	CloseHandle(_hfile);
}

