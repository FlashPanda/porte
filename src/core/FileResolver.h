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

		/// ��ʼ��һ���µ��ļ��������õ�ǰ�Ĺ���Ŀ¼
		FileResolver();

		FileResolver(const FileResolver& fr);

		/// ��������·���б����Խ�������·��
		Path Resolve(const Path& path) const;

		/// ��������·������
		size_t Size() const { return mPaths.size(); }

		/// ��������·���б����ʼ������
		iterator Begin() { return mPaths.begin(); }

		/// ��������·���б�Ľ���������
		iterator End() { return mPaths.end(); }

		/// ��������·���б��const��ʼ������
		const_iterator Begin() const { return mPaths.begin(); }

		/// ��������·���б��const����������
		const_iterator End()   const { return mPaths.end(); }

		/// ���p�Ƿ�������·���б���
		bool Contains(const Path& p) const;

		/// �Ƴ�����������λ�õ�Ԫ��
		void Erase(iterator it) { mPaths.erase(it); }

		/// �Ƴ�����·���б��е�ָ��·��
		void Erase(const Path& p);

		/// �������·���б�
		void Clear() { mPaths.clear(); }

		/// ������·���б�Ŀ�ʼ����һ��·��
		void Prepend(const Path& path) { mPaths.insert(mPaths.begin(), path); }

		/// ������·���б��������һ��·��
		void Append(const Path& path) { mPaths.push_back(path); }

		/// ��������·���б��·��
		Path& operator[](size_t index) { return mPaths[index]; }

		/// ��������·���б��const·��
		const Path& operator[](size_t index) const { return mPaths[index]; }

		/// ���ؿɶ���·���ַ���
		std::string ToString() const;

	private:
		std::vector<Path> mPaths;
	};
}