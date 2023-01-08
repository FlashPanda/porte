#pragma once
#include <string>
#include <vector>

namespace panda
{
    /** 
     * ϵͳ���ַ������ͣ�����ϵͳAPIʱ����ַ������ͣ�
     */
#if defined(WIN32)
    using ValueType = wchar_t;
    using StringType = std::wstring;
#else
    using ValueType = char;
    using StringType = std::string;
#endif

	/// ϵͳָ���ķָ���������д·��
#if defined(WIN32)
	constexpr ValueType PreferredSeparator = L'\\';
#else
	constexpr ValueType PreferredSeparator = '/';
#endif

    /** 
     * �ļ�ϵͳ��Դ�е�·����ʾ
     * �ڹ�����ʱ��·���ı�ʾ��ʽ������ϵͳ�������洢��
     * ·�����Ա�ת�����ض�ϵͳ���ַ�������<tt>native()</tt>����<tt>string()</tt>
     */
    class Path
    {
	public:
        /// Ĭ�Ϲ��캯��������һ����·������·�������·����
		Path() : mAbsolute(false) { }

        Path(const Path& path)
			: mPath(path.mPath), mAbsolute(path.mAbsolute) {}

		Path(Path&& path)
			: mPath(std::move(path.mPath)), mAbsolute(path.mAbsolute) {}

        /** 
         * ����һ���ӱ��ظ�ʽ���ַ�������·��
         * ��Windows�ϣ�·���ķָ���������'/'����'\\'
         */
        Path(const StringType& string) { Set(string); }

        /**
         * ����һ���ӱ��ظ�ʽ���ַ�������·��
         * ��Windows�ϣ�·���ķָ���������'/'����'\\'
         */
        Path(const ValueType* string) { Set(string); }

#if defined(WIN32)
        /** 
         * ��std::string������һ��·�������㲻�Ǳ������ַ������͡������ַ�����UTF-8����
         * ��ת���ɱ��������͡�
         */
        Path(const std::string& string);

        /**
         * ��std::string������һ��·�������㲻�Ǳ������ַ������͡������ַ�����UTF-8����
         * ��ת���ɱ��������͡�
         */
        Path(const char* string) : Path(std::string(string)) { }
#endif

        /// ��·�����ó�һ����·������·�������·����
		void Clear() {
			mAbsolute = false;
			mPath.clear();
		}

        /// �ж�·���ǲ��ǿյ�
		bool IsEmpty() const { return mPath.empty(); }

		/// �ж�·���ǲ��Ǿ���·��
		bool IsAbsolute() const { return mAbsolute; }

		/// �ж�·���ǲ������·��
		bool IsRelative() const { return !mAbsolute; }

        /// ���ظ�·��
        /// ���û�и�·���������Ȿ�����һ����·�������ص�Ҳ�ǿ�·����
        Path ParentPath() const;

        /// ����·���ļ�������չ��
        /// ����·��'.'��'..'����չ��Ϊ��
        Path Extension() const;

        /// �滻���ұߵ�'.'��������ʾ�����ַ����ɲ����ַ���
        /// Ҳ�����滻��չ�������·���ǿյģ��÷�����ʲô��������
        /// ���ص����Լ������á�
        Path& ReplaceExtension(const Path& replacement = Path());

        /// ����·�����ļ������֣�������չ��
		Path Filename() const;

        /// �ñ����ַ�������ʽ����·�����������Ϳ���ֱ�ӱ����ݸ�ϵͳAPI�ˡ�
        const StringType Native() const noexcept { return Str(); }

        /// ��ʽת����������ת���ɱ�����ϵͳ�ַ����͡��ȼ������<tt>Native()</tt>
        operator StringType() const noexcept { return Native(); }

        /// �ȼ���Native()��ת����std::string����
		std::string String() const;

        /// ��Ŀ¼�ָ�����������·����
		Path operator/(const Path& other) const;

        Path& operator=(const Path& path);

        Path& operator=(Path&& path);

        /// ��ϵͳ�����ַ������͸�ֵ��·�������ַ������캯�����ơ�
        Path& operator=(const StringType& str) { Set(str); return *this; }

#if defined(WIN32)
        /// ��std::string����·��
        Path& operator=(const std::string& str);
#endif

        /// ���·�����ַ���
	    friend std::ostream& operator<<(std::ostream& os, const Path& path);

        /// �߼�������·���Ƿ�����ͬ�ģ�������ÿ���ַ���һ����
        bool operator==(const Path& p) const { return p.mPath == mPath; }

		bool operator!=(const Path& p) const { return p.mPath != mPath; }

	protected:
        StringType Str() const;

		/// ���ַ����й���һ��·��
		void Set(const StringType& str);

        /// ���ַ����ָ��delim�д��ݵķָ���
        static std::vector<StringType> Tokenize(const StringType& string,
            const StringType& delim);

	protected:
		std::vector<StringType> mPath;
		bool mAbsolute;
    };

    /// ���ص�ǰ�Ĺ���Ŀ¼���ȼ���getcwd��
	extern Path CurrentPath();

    /// ���ز����ľ���·�����ο���
    /// http ://en.cppreference.com/w/cpp/experimental/fs/absolute
    extern Path Absolute(const Path& p);

    /// ������p�Ƿ�ָ��һ�������ļ���������һ��Ŀ¼����һ������
	extern bool IsRegularFile(const Path& p) noexcept;
    /// ������p�Ƿ�ָ����һ��Ŀ¼
	extern bool IsDirectory(const Path& p) noexcept;
    /// ������p�Ƿ�ָ����һ�����ڵ��ļ�ϵͳ����
	extern bool Exists(const Path& p) noexcept;

    /// ����pָ��ĳ����ļ����ֽڴ�С
    /// ���pָ��һ��Ŀ¼�������ӣ���ô�⺯���ᱨ��
    extern size_t FileSize(const Path& p);

    /// �������path�����Ƿ���������ͬ���ļ�ϵͳ���󡣲������ļ���·�����������ӡ�
    extern bool Equivalent(const Path& p1, const Path& p2);

    /// ��p��λ�ô���һ��Ŀ¼��������mkdir����������������������ɹ�������true�����򷵻�false��
    /// ���p�Ѿ����ڣ�������ʲô��������
    extern bool CreateDirectory(const Path& p) noexcept;
    /// �ı�pָ��ĳ����ļ��Ĵ�С���͸�truncate����������������ļ���targetLength����ô�������
    /// ���ֻᱻ�������⺯��Ҫ���ļ�������ڡ�
    extern bool ResizeFile(const Path& p, size_t targetLength) noexcept;

    /// �Ƴ�һ���ļ����߿�Ŀ¼������Ƴ��ɹ�������true�������������false�������ļ������ڣ���
    extern bool Remove(const Path& p);

    /// ��һ���ļ�����Ŀ¼������������ɹ�������true�������������false��
    extern bool Rename(const Path& src, const Path& dst);

    // �Ƿ��к�׺
    extern bool HasExtension(const std::string& value, const std::string& ending);
}