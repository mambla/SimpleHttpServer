#pragma once

#include <Windows.h>
#include <string>

class FileHandlerBase
{

public:
	FileHandlerBase();

	FileHandlerBase(const std::wstring& file_path);

	HANDLE get_file_handler()const;

	~FileHandlerBase();
private:
	HANDLE _hfile;
	const DWORD _permissions;
};
