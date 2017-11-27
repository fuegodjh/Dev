#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

#ifndef SPARKY_EMSCRIPTEN
	#include <Windows.h>
#else if
	#include <emscripten/emscripten.h>
#endif

#define SPARKY_LOG_LEVEL_FATAL 0
#define SPARKY_LOG_LEVEL_ERROR 1
#define SPARKY_LOG_LEVEL_WARN  2
#define SPARKY_LOG_LEVEL_INFO  3

namespace std {
	template <typename T, typename = typename std::enable_if<!std::is_same<T, std::string>>>
	string to_string(const T& t)
	{
		return std::string("[Unsupported type (") + typeid(T).name() + std::string(")!] (to_string)");
	}
}

namespace Sparky {
	namespace Internal {

		static char to_string_buffer[1024 * 10];

		template <class T>
		struct has_iterator
		{
			template<class U> static char(&test(typename U::iterator const*))[1];
			template<class U> static char(&test(...))[2];
			static const bool value = (sizeof(test<T>(0)) == 1);
		};
			
		template <typename T>
		static const char* to_string(const T& t)
		{
			return to_string_internal<T>(t, std::integral_constant<bool, has_iterator<T>::value>());
		}

		template <>
		static const char* to_string<char>(const char& t)
		{
			return &t;
		}

		template <>
		static const char* to_string<char*>(char* const& t)
		{
			return t;
		}

		template <>
		static const char* to_string<const char*>(const char* const & t)
		{
			return t;
		}

		template <>
		static const char* to_string<std::string>(std::string const & t)
		{
			return t.c_str();
		}

		template <typename T>
		static std::string format_iterators(T& begin, T& end)
		{
			std::string result;
			bool first = true;
			while (begin != end) {
				if (!first)
					result += ", ";
				result += to_string(*begin);
				first = false;
				begin++;
			}
			return result;
		}

		template <typename T>
		static const char* to_string_internal(const T& v, const std::true_type& ignored)
		{
			sprintf(to_string_buffer, "Container of size: %d, contents: %s", v.size(), format_iterators(v.begin(), v.end()).c_str());
			return to_string_buffer;
		}

		template <typename T>
		static const char* to_string_internal(const T& t, const std::false_type& ignored)
		{
			auto x = std::to_string(t);
			return strcpy(to_string_buffer, x.c_str());
		}
			
		template<typename X, typename Y>
		static const char* to_string(const std::pair<typename X, typename Y>& v)
		{
			sprintf(to_string_buffer, "(Key: %s, Value: %s)", to_string(v.first), to_string(v.second));
			return to_string_buffer;
		}

		template<>
		static const char* to_string_internal<const char*>(const char* const& v, const std::false_type& ignored)
		{
			return v;
		}

		template <typename First>
		static void print_log_internal(char* buffer, int& position, First&& first)
		{
			const char* formatted = to_string<First>(first);
			size_t length = strlen(formatted);
			memcpy(&buffer[position], formatted, length);
			position += (int)length;
		}

		template <typename First, typename... Args>
		static void print_log_internal(char* buffer, int& position, First&& first, Args&&... args)
		{
			const char* formatted = to_string<First>(first);
			size_t length = strlen(formatted);
			memcpy(&buffer[position], formatted, length);
			position += (int)length;
			if (sizeof...(Args))
			{
				print_log_internal(buffer, position, std::forward<Args>(args)...);
			}
		}

		template <typename... Args>
		static void log_message(int level, bool newline, Args... args)
		{
			char buffer[1024 * 10];
			int position = 0;
			print_log_internal(buffer, position, std::forward<Args>(args)...);
			
			if(newline) buffer[position++] = '\n';
			buffer[position] = 0;
			
#ifdef SPARKY_EMSCRIPTEN
			EM_ASM{
				switch (level)
				{
					case SPARKY_LOG_LEVEL_FATAL:
						console.log(buffer, 'background: red; color: white');
					break;
					case SPARKY_LOG_LEVEL_ERROR:
						console.log(buffer, 'color: red');
					break;
					case SPARKY_LOG_LEVEL_WARN:
						console.log(buffer, 'color: yellow');
					break;
				}
			};
#else if
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			switch (level)
			{
				case SPARKY_LOG_LEVEL_FATAL:
					SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				break;
				case SPARKY_LOG_LEVEL_ERROR:
					SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
				break;
				case SPARKY_LOG_LEVEL_WARN:
					SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				break;
			}
			std::cout << buffer;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
		}
	}
}

#ifndef SPARKY_LOG_LEVEL
	#define SPARKY_LOG_LEVEL SPARKY_LOG_LEVEL_INFO
#endif

#if SPARKY_LOG_LEVEL >= SPARKY_LOG_LEVEL_FATAL
#define SPARKY_FATAL(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_FATAL, true, "SPARKY:  ", __VA_ARGS__)
#define _SPARKY_FATAL(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_FATAL, false, "SPARKY:  ", __VA_ARGS__)
#define __SPARKY_FATAL(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_FATAL, false, __VA_ARGS__)
#define ___SPARKY_FATAL(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_FATAL, true, __VA_ARGS__)
#else if
#define SPARKY_FATAL(...)
#define _SPARKY_FATAL(...)
#define __SPARKY_FATAL(...)
#define ___SPARKY_FATAL(...)
#endif

#if SPARKY_LOG_LEVEL >= SPARKY_LOG_LEVEL_ERROR
#define SPARKY_ERROR(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_ERROR, true, "SPARKY:  ", __VA_ARGS__)
#define _SPARKY_ERROR(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_ERROR, false, "SPARKY:  ", __VA_ARGS__)
#define __SPARKY_ERROR(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_ERROR, false, __VA_ARGS__)
#define ___SPARKY_ERROR(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_ERROR, true, __VA_ARGS__)
#else if
#define SPARKY_ERROR(...)
#define _SPARKY_ERROR(...)
#define __SPARKY_ERROR(...)
#define ___SPARKY_ERROR(...)
#endif

#if SPARKY_LOG_LEVEL >= SPARKY_LOG_LEVEL_WARN
#define SPARKY_WARN(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_WARN, true, "SPARKY:  ", __VA_ARGS__)
#define _SPARKY_WARN(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_WARN, false, "SPARKY:  ", __VA_ARGS__)
#define __SPARKY_WARN(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_WARN, false, __VA_ARGS__)
#define ___SPARKY_WARN(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_WARN, true, __VA_ARGS__)
#else if
#define SPARKY_WARN(...)
#define _SPARKY_WARN(...)
#define __SPARKY_WARN(...)
#define ___SPARKY_WARN(...)
#endif

#if SPARKY_LOG_LEVEL >= SPARKY_LOG_LEVEL_INFO
#define SPARKY_INFO(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_INFO, true, "SPARKY:  ", __VA_ARGS__)
#define _SPARKY_INFO(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_INFO, false, "SPARKY:  ", __VA_ARGS__)
#define __SPARKY_INFO(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_INFO, false, __VA_ARGS__)
#define ___SPARKY_INFO(...) Sparky::Internal::log_message(SPARKY_LOG_LEVEL_INFO, true, __VA_ARGS__)
#else if
#define SPARKY_INFO(...)
#define _SPARKY_INFO(...)
#define __SPARKY_INFO(...)
#define ___SPARKY_INFO(...)
#endif

#define SPARKY_ASSERT(x, ...) \
	do { \
		if (!(x)) \
			{ \
			char* file = __FILE__; \
			unsigned int last = 0; \
			for (int i = 0; file[i] != '\0'; i++) \
			{ \
				if (file[i] == '\\' || file[i] == '/') \
					last = i; \
			} \
			SPARKY_FATAL("-------------------------"); \
			SPARKY_FATAL("    ASSERTION FAILED!    "); \
			SPARKY_FATAL("-------------------------"); \
			SPARKY_FATAL(#x); \
			SPARKY_FATAL(__VA_ARGS__); \
			SPARKY_FATAL("-> ", &file[last + 1], ":", __LINE__); \
			*(int*)NULL = 8; \
		} \
	} while(0)