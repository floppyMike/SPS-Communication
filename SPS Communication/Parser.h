#pragma once
#include "Includes.h"

template<typename T>
struct Command
{
	std::string name;
	T val;
};

template<template<typename> class... Ex>
class basic_MessageCommands : public Ex<basic_MessageCommands<Ex...>>...
{
public:
	basic_MessageCommands() = default;

private:
	std::vector<Command<int>> m_commands;
};

template<typename Impl>
class EParser
{
public:
	void parse_message(std::string&& message)
	{
		size_t idx = 0;
		std::equal(message.begin(), message.begin() + 7, "#START");
		
	}

private:

};


using MessageCommands = basic_MessageCommands<EParser>;