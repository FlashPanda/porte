#pragma once

#include "Filesystem.h"
#include <iosfwd>

namespace panda
{
	class FileResolver
	{
	public:
		using iterator = std::vector<Path>::iterator;
		using const_iterator = std::vector<Path>::const_iterator;

		/// 初始化一个新的文件解析，用当前的工作目录
		FileResolver();

		FileResolver(const FileResolver& fr);

		/// 遍历搜索路径列表，尝试解析输入路径
		Path Resolve(const Path& path) const;

		/// 返回搜索路径数量
		size_t Size() const { return mPaths.size(); }

		/// 返回搜索路径列表的起始迭代器
		iterator Begin() { return mPaths.begin(); }

		/// 返回搜索路径列表的结束迭代器
		iterator End() { return mPaths.end(); }

		/// 返回搜索路径列表的const起始迭代器
		const_iterator Begin() const { return mPaths.begin(); }

		/// 返回搜索路径列表的const结束迭代器
		const_iterator End()   const { return mPaths.end(); }

		/// 检查p是否在搜索路径列表中
		bool Contains(const Path& p) const;

		/// 移除迭代器输入位置的元素
		void Erase(iterator it) { mPaths.erase(it); }

		/// 移除搜索路径列表中的指定路径
		void Erase(const Path& p);

		/// 清空搜索路径列表
		void Clear() { mPaths.clear(); }

		/// 在搜索路径列表的开始插入一个路径
		void Prepend(const Path& path) { mPaths.insert(mPaths.begin(), path); }

		/// 在搜索路径列表的最后添加一个路径
		void Append(const Path& path) { mPaths.push_back(path); }

		/// 返回搜索路径列表的路径
		Path& operator[](size_t index) { return mPaths[index]; }

		/// 返回搜索路径列表的const路径
		const Path& operator[](size_t index) const { return mPaths[index]; }

		/// 返回可读的路径字符串
		std::string ToString() const;

	private:
		std::vector<Path> mPaths;
	};
}