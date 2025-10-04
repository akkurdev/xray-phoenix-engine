#pragma once
#include <string_view>
#include <exception>
#include <format>

inline std::string_view exception_to_string(const std::string_view& function_name, const std::exception& ex)
{
	return std::format("LUABIND EXCEPTION:\n Function: {}\n {}", function_name, ex.what()).c_str();
}