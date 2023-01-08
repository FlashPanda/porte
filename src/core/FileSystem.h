#pragma once
#include <string>
#include <vector>

namespace panda
{
    /** 
     * 系统的字符集类型（调用系统API时候的字符集类型）
     */
#if defined(WIN32)
    using ValueType = wchar_t;
    using StringType = std::wstring;
#else
    using ValueType = char;
    using StringType = std::string;
#endif

	/// 系统指定的分隔符，用于写路径
#if defined(WIN32)
	constexpr ValueType PreferredSeparator = L'\\';
#else
	constexpr ValueType PreferredSeparator = '/';
#endif

    /** 
     * 文件系统资源中的路径表示
     * 在构建的时候，路径的表示方式会随着系统解析并存储。
     * 路径可以被转换成特定系统的字符串，用<tt>native()</tt>或者<tt>string()</tt>
     */
    class Path
    {
	public:
        /// 默认构造函数。构造一个空路径。空路径是相对路径。
		Path() : mAbsolute(false) { }

        Path(const Path& path)
			: mPath(path.mPath), mAbsolute(path.mAbsolute) {}

		Path(Path&& path)
			: mPath(std::move(path.mPath)), mAbsolute(path.mAbsolute) {}

        /** 
         * 构造一个从本地格式的字符串来的路径
         * 在Windows上，路径的分隔符可以是'/'或者'\\'
         */
        Path(const StringType& string) { Set(string); }

        /**
         * 构造一个从本地格式的字符串来的路径
         * 在Windows上，路径的分隔符可以是'/'或者'\\'
         */
        Path(const ValueType* string) { Set(string); }

#if defined(WIN32)
        /** 
         * 从std::string构建里一个路径，即便不是本机的字符串类型。假设字符串是UTF-8编码
         * 以转换成本机的类型。
         */
        Path(const std::string& string);

        /**
         * 从std::string构建里一个路径，即便不是本机的字符串类型。假设字符串是UTF-8编码
         * 以转换成本机的类型。
         */
        Path(const char* string) : Path(std::string(string)) { }
#endif

        /// 把路径设置成一个空路径。空路径是相对路径。
		void Clear() {
			mAbsolute = false;
			mPath.clear();
		}

        /// 判断路径是不是空的
		bool IsEmpty() const { return mPath.empty(); }

		/// 判断路径是不是绝对路径
		bool IsAbsolute() const { return mAbsolute; }

		/// 判断路径是不是相对路径
		bool IsRelative() const { return !mAbsolute; }

        /// 返回父路径
        /// 如果没有父路径，或者这本身就是一个空路径，返回的也是空路径。
        Path ParentPath() const;

        /// 返回路径文件名的扩展名
        /// 特殊路径'.'和'..'的扩展名为空
        Path Extension() const;

        /// 替换最右边的'.'符号所表示的子字符串成参数字符串
        /// 也就是替换扩展名。如果路径是空的，该方法就什么都不做。
        /// 返回的是自己的引用。
        Path& ReplaceExtension(const Path& replacement = Path());

        /// 返回路径的文件名部分，包括扩展名
		Path Filename() const;

        /// 用本机字符串的形式返回路径，这样它就可以直接被传递给系统API了。
        const StringType Native() const noexcept { return Str(); }

        /// 隐式转换操作符，转换成本机的系统字符类型。等价与调用<tt>Native()</tt>
        operator StringType() const noexcept { return Native(); }

        /// 等价于Native()，转换成std::string类型
		std::string String() const;

        /// 用目录分隔符连接两个路径。
		Path operator/(const Path& other) const;

        Path& operator=(const Path& path);

        Path& operator=(Path&& path);

        /// 用系统本机字符串类型赋值给路径。和字符串构造函数类似。
        Path& operator=(const StringType& str) { Set(str); return *this; }

#if defined(WIN32)
        /// 用std::string构建路径
        Path& operator=(const std::string& str);
#endif

        /// 输出路径的字符串
	    friend std::ostream& operator<<(std::ostream& os, const Path& path);

        /// 逻辑意义上路径是否是相同的，而不是每个字符都一样。
        bool operator==(const Path& p) const { return p.mPath == mPath; }

		bool operator!=(const Path& p) const { return p.mPath != mPath; }

	protected:
        StringType Str() const;

		/// 从字符串中构建一个路径
		void Set(const StringType& str);

        /// 将字符串分割，用delim中传递的分隔符
        static std::vector<StringType> Tokenize(const StringType& string,
            const StringType& delim);

	protected:
		std::vector<StringType> mPath;
		bool mAbsolute;
    };

    /// 返回当前的工作目录（等价于getcwd）
	extern Path CurrentPath();

    /// 返回参数的绝对路径，参看：
    /// http ://en.cppreference.com/w/cpp/experimental/fs/absolute
    extern Path Absolute(const Path& p);

    /// 检查参数p是否指向一个常规文件，而不是一个目录或者一个链接
	extern bool IsRegularFile(const Path& p) noexcept;
    /// 检查参数p是否指向了一个目录
	extern bool IsDirectory(const Path& p) noexcept;
    /// 检查参数p是否指向了一个存在的文件系统对象。
	extern bool Exists(const Path& p) noexcept;

    /// 返回p指向的常规文件的字节大小
    /// 如果p指向一个目录或者链接，那么这函数会报错。
    extern size_t FileSize(const Path& p);

    /// 检查两个path对象是否引用了相同的文件系统对象。不管是文件、路径、还是链接。
    extern bool Equivalent(const Path& p1, const Path& p2);

    /// 在p的位置创建一个目录，就像是mkdir命令做的那样。如果创建成功，返回true。否则返回false。
    /// 如果p已经存在，函数就什么都不做。
    extern bool CreateDirectory(const Path& p) noexcept;
    /// 改变p指向的常规文件的大小，就跟truncate被调用那样。如果文件比targetLength大，那么多出来的
    /// 部分会被丢弃。这函数要求文件必须存在。
    extern bool ResizeFile(const Path& p, size_t targetLength) noexcept;

    /// 移除一个文件或者空目录。如果移除成功，返回true。如果出错，返回false（比如文件不存在）。
    extern bool Remove(const Path& p);

    /// 将一个文件或者目录重命名。如果成功，返回true。如果出错，返回false。
    extern bool Rename(const Path& src, const Path& dst);

    // 是否有后缀
    extern bool HasExtension(const std::string& value, const std::string& ending);
}