#pragma once

#include "Includes.h"
#include "Logging.h"

template<typename T, template<typename> class crtpType>
struct crtp
{
	auto underlying() noexcept { return static_cast<T *>(this); }
	auto underlying() const noexcept { return static_cast<const T *>(this); }

private:
	crtp() = default;
	friend crtpType<T>;
};

template<typename T>
auto str_to_num(std::string_view str) noexcept -> std::optional<T>
{
	if (T temp; str.empty() || [&temp, &str]() -> bool {
			if constexpr (std::is_floating_point_v<T>) // g++ and clang++ don't support floating point from_chars
				if constexpr (std::is_same_v<T, float>)
					try
					{
						temp = std::stof(std::string(str));
						return false;
					}
					catch (const std::exception &)
					{
						return true;
					}
				else
				{
					assert(false && "Only floating point number float is supported.");
					return true;
				}
			else
				return std::from_chars(str.data(), str.data() + str.size(), temp).ec != std::errc();
		}())
		return std::nullopt;
	else
		return temp;
}

template<typename _T>
auto guarded_get(std::optional<_T> &&opt, std::string_view message_on_error)
{
	if (opt.has_value())
		return opt.value();

	throw std::runtime_error(message_on_error.data());
}

auto guarded_get_string(const rj::Value &val) -> const char *
{
	if (val.IsString())
		return val.GetString();

	throw std::runtime_error("String expected at data variable.");
}

auto guarded_get_section(const rj::Value &val, std::string_view sec) -> const auto &
{
	if (auto iter = val.FindMember(sec.data()); iter != val.MemberEnd())
		return iter->value;

	throw std::runtime_error("Object \"" + std::string(sec) + "\" not found in the json data.");
}

auto guarded_get_section(rj::Value &val, std::string_view sec) -> auto &
{
	if (auto iter = val.FindMember(sec.data()); iter != val.MemberEnd())
		return iter->value;

	throw std::runtime_error("Object \"" + std::string(sec) + "\" not found in the json data.");
}

auto append_filename(std::string f, std::string_view name)
{
	f.insert(f.find_first_of('.'), name);
	return f;
}

// func -> bool if should continue
template<typename F>
auto enclosed_do(F &&func, std::string_view activity)
{
	for (char err_c = 1; true;) try
		{
			if (!func())
				break;

			err_c = 1;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL)
				<< "Error " << +err_c << " of 5 during " << activity << ": " << e.what();

			if (err_c++ == 5)
				return true;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return true;
		}

	return false;
}