#pragma once
#include <exception>

class ServerInitilizationError : public std::exception
{

};

class FileNotFoundError : public std::exception
{
public:

};