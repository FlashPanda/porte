#include "FileSystem.h"
#include <cctype>
#include <cerrno>
#include <codecvt>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <stdexcept>
#include <sstream>

#if defined(WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/stat.h>
#endif

#if defined(__LINUX__)
#  include <linux/limits.h>
#endif

/** Macro allowing to type hardocded character sequences
 * with the right type prefix (char_t: no prefix, wchar_t: 'L' prefix)
 */
/// 允许使用正确类型前缀键入硬编码字符序列的宏
/// 比如：char_t：没有前缀，wchar_t：'L'前缀
#if defined(WIN32)
#  define NSTR(str) L##str
#else
#  define NSTR(str) str
#endif

namespace panda
{
	inline StringType ToNative(const std::string& str) {
#if defined(WIN32)
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(str);
#else
		return str;
#endif
	}

	inline std::string FromNative(const StringType& str) {
#if defined(WIN32)
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(str);
#else
		return str;
#endif
	}

#if defined(WIN32)
	Path::Path(const std::string& string) { Set(ToNative(string)); }
#endif

	Path CurrentPath() {
#if !defined(WIN32)
		char temp[PATH_MAX];
		if (::getcwd(temp, PATH_MAX) == NULL)
			throw std::runtime_error("Internal error in filesystem::current_path(): " + std::string(strerror(errno)));
		return Path(temp);
#else
		std::wstring temp(MAX_PATH, '\0');
		if (!_wgetcwd(&temp[0], MAX_PATH))
			throw std::runtime_error("Internal error in filesystem::current_path(): " + std::to_string(GetLastError()));
		return Path(temp.c_str());
#endif
	}

	Path Absolute(const Path& p) {
#if !defined(WIN32)
		char temp[PATH_MAX];
		if (realpath(p.Native().c_str(), temp) == NULL)
			throw std::runtime_error("Internal error in realpath(): " + std::string(strerror(errno)));
		return path(temp);
#else
		std::wstring value = p.Native(), out(MAX_PATH, '\0');
		DWORD length = GetFullPathNameW(value.c_str(), MAX_PATH, &out[0], NULL);
		if (length == 0)
			throw std::runtime_error("Internal error in realpath(): " + std::to_string(GetLastError()));
		return Path(out.substr(0, length));
#endif
	}

	bool IsRegularFile(const Path& p) noexcept {
#if defined(WIN32)
		DWORD attr = GetFileAttributesW(p.Native().c_str());
		return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
		struct stat sb;
		if (stat(p.Native().c_str(), &sb))
			return false;
		return S_ISREG(sb.st_mode);
#endif
	}

	bool IsDirectory(const Path& p) noexcept {
#if defined(WIN32)
		DWORD result = GetFileAttributesW(p.Native().c_str());
		if (result == INVALID_FILE_ATTRIBUTES)
			return false;
		return (result & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
		struct stat sb;
		if (stat(p.Native().c_str(), &sb))
			return false;
		return S_ISDIR(sb.st_mode);
#endif
	}

	bool Exists(const Path& p) noexcept {
#if defined(WIN32)
		return GetFileAttributesW(p.Native().c_str()) != INVALID_FILE_ATTRIBUTES;
#else
		struct stat sb;
		return stat(p.Native().c_str(), &sb) == 0;
#endif
	}

	size_t FileSize(const Path& p) {
#if defined(WIN32)
		struct _stati64 sb;
		if (_wstati64(p.Native().c_str(), &sb) != 0)
			throw std::runtime_error("filesystem::file_size(): cannot stat file \"" + p.String() + "\"!");
#else
		struct stat sb;
		if (stat(p.native().c_str(), &sb) != 0)
			throw std::runtime_error("filesystem::file_size(): cannot stat file \"" + p.String() + "\"!");
#endif
		return (size_t)sb.st_size;
	}

	bool Equivalent(const Path& p1, const Path& p2) {
#if defined(WIN32)
		struct _stati64 sb1, sb2;
		if (_wstati64(p1.Native().c_str(), &sb1) != 0)
			throw std::runtime_error("filesystem::equivalent(): cannot stat file \"" + p1.String() + "\"!");
		if (_wstati64(p2.Native().c_str(), &sb2) != 0)
			throw std::runtime_error("filesystem::equivalent(): cannot stat file \"" + p2.String() + "\"!");
#else
		struct stat sb1, sb2;
		if (stat(p1.Native().c_str(), &sb1) != 0)
			throw std::runtime_error("filesystem::equivalent(): cannot stat file \"" + p1.String() + "\"!");
		if (stat(p2.Native().c_str(), &sb2) != 0)
			throw std::runtime_error("filesystem::equivalent(): cannot stat file \"" + p2.String() + "\"!");
#endif

		return (sb1.st_dev == sb2.st_dev) && (sb1.st_ino == sb2.st_ino);
	}

	bool CreateDirectory(const Path& p) noexcept {
		if (Exists(p))
			return IsDirectory(p);

#if defined(WIN32)
		return CreateDirectoryW(p.Native().c_str(), NULL) != 0;
#else
		return mkdir(p.Native().c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
#endif
	}

	bool ResizeFile(const Path& p, size_t targetLength) noexcept {
#if !defined(WIN32)
		return ::truncate(p.native().c_str(), (off_t)target_length) == 0;
#else
		HANDLE handle = CreateFileW(p.Native().c_str(), GENERIC_WRITE, 0,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (handle == INVALID_HANDLE_VALUE)
			return false;

		LARGE_INTEGER size;
		size.QuadPart = (LONGLONG)targetLength;
		if (SetFilePointerEx(handle, size, NULL, FILE_BEGIN) == 0) {
			CloseHandle(handle);
			return false;
		}
		if (SetEndOfFile(handle) == 0) {
			CloseHandle(handle);
			return false;
		}
		CloseHandle(handle);
		return true;
#endif
	}

	bool Remove(const Path& p) {
#if !defined(WIN32)
		return std::remove(p.Native().c_str()) == 0;
#else
		if (IsDirectory(p))
			return RemoveDirectoryW(p.Native().c_str()) != 0;
		else
			return DeleteFileW(p.Native().c_str()) != 0;
#endif
	}

	bool Rename(const Path& src, const Path& dst) {
#if !defined(WIN32)
		return std::rename(src.Native().c_str(), dst.Native().c_str()) == 0;
#else
		return MoveFileW(src.Native().c_str(), dst.Native().c_str()) != 0;
#endif
	}

	bool HasExtension(const std::string& value, const std::string& ending)
	{
		if (ending.size() > value.size()) return false;

		return std::equal(ending.rbegin(), ending.rend(), value.rbegin(),
			[](char a, char b) {return std::tolower(a) == std::tolower(b); });
	}

	// -----------------------------------------------------------------------------

	Path Path::Extension() const {
		if (IsEmpty() || mPath.back() == NSTR(".") || mPath.back() == NSTR(".."))
			return NSTR("");

		const StringType& name = Filename();
		size_t pos = name.find_last_of(NSTR("."));
		if (pos == StringType::npos)
			return "";
		return name.substr(pos);  // 包括 . 字符!
	}

	Path& Path::ReplaceExtension(const Path& replacement_) {
		if (IsEmpty() || mPath.back() == NSTR(".") || mPath.back() == NSTR(".."))
			return *this;

		StringType name = Filename();
		size_t pos = name.find_last_of(NSTR("."));

		if (pos != StringType::npos)
			name = name.substr(0, pos);

		StringType replacement(replacement_);
		if (!replacement.empty()) {
			StringType period(NSTR("."));
			if (std::equal(period.begin(), period.end(), replacement.begin()))
				name += replacement;
			else
				name += period + replacement;
		}

		mPath.back() = name;
		return *this;
	}

	Path Path::Filename() const {
		if (IsEmpty())
			return Path(NSTR(""));
		return Path(mPath.back());
	}

	Path Path::ParentPath() const {
		Path result;
		result.mAbsolute = mAbsolute;

		if (mPath.empty()) {
			if (!mAbsolute)
				result.mPath.push_back(NSTR(".."));
		}
		else {
			size_t until = mPath.size() - 1;
			for (size_t i = 0; i < until; ++i)
				result.mPath.push_back(mPath[i]);
		}
		return result;
	}

	// -----------------------------------------------------------------------------

	std::string Path::String() const {
		return FromNative(Str());
	}

	// -----------------------------------------------------------------------------

	Path Path::operator/(const Path& other) const {
		if (other.mAbsolute)
			throw std::runtime_error("path::operator/(): expected a relative path!");

		Path result(*this);

		for (size_t i = 0; i < other.mPath.size(); ++i)
			result.mPath.push_back(other.mPath[i]);

		return result;
	}

	Path& Path::operator=(const Path& path) {
		mPath = path.mPath;
		mAbsolute = path.mAbsolute;
		return *this;
	}

	Path& Path::operator=(Path&& path) {
		if (this != &path) {
			mPath = std::move(path.mPath);
			mAbsolute = path.mAbsolute;
		}
		return *this;
	}

#if defined(WIN32)
	Path& Path::operator=(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		Set(converter.from_bytes(str));
		return *this;
	}
#endif

	StringType Path::Str() const {
		std::basic_ostringstream<ValueType> oss;

#if !defined(WIN32)
		if (mAbsolute)
			oss << PreferredSeparator;
#endif

		for (size_t i = 0; i < mPath.size(); ++i) {
			oss << mPath[i];
			if (i + 1 < mPath.size()) {
				oss << PreferredSeparator;
			}
		}

		return oss.str();
	}

	void Path::Set(const StringType& str) {
		if (str.empty()) {
			Clear();
			return;
		}

#if defined(WIN32)
		mPath = Tokenize(str, NSTR("/\\"));
		mAbsolute = str.size() >= 2 && std::isalpha(str[0]) && str[1] == NSTR(':');
#else
		mPath = Tokenize(str, NSTR("/"));
		mAbsolute = !str.empty() && str[0] == NSTR('/');
#endif
	}

	std::vector<StringType> Path::Tokenize(const StringType& string,
		const StringType& delim) {
		StringType::size_type lastPos = 0,
			pos = string.find_first_of(delim, lastPos);
		std::vector<StringType> tokens;

		while (lastPos != StringType::npos) {
			if (pos != lastPos)
				tokens.push_back(string.substr(lastPos, pos - lastPos));
			lastPos = pos;
			if (lastPos == StringType::npos || lastPos + 1 == string.length())
				break;
			pos = string.find_first_of(delim, ++lastPos);
		}

		return tokens;
	}

	std::ostream& operator<<(std::ostream& os, const Path& path) {
		os << path.String();
		return os;
	}
}