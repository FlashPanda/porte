#include "FileResolver.h"
#include <sstream>
#include <algorithm>

namespace porte
{
	FileResolver::FileResolver()
	{
		mPaths.push_back(CurrentPath());
	}

	FileResolver::FileResolver(const FileResolver& fr) : mPaths(fr.mPaths)
	{
	}

	void FileResolver::Erase(const Path& p) {
		mPaths.erase(std::remove(mPaths.begin(), mPaths.end(), p), mPaths.end());
	}

	bool FileResolver::Contains(const Path& p) const {
		return std::find(mPaths.begin(), mPaths.end(), p) != mPaths.end();
	}

	Path FileResolver::Resolve(const Path& path) const {
		if (!path.IsAbsolute()) {
			for (auto const& base : mPaths) {
				Path combined = base / path;
				if (Exists(combined))
					return combined;
			}
		}
		return path;
	}

	std::string FileResolver::ToString() const {
		std::ostringstream oss;
		oss << "FileResolver[" << std::endl;
		for (size_t i = 0; i < mPaths.size(); ++i) {
			oss << "  \"" << mPaths[i] << "\"";
			if (i + 1 < mPaths.size())
				oss << ",";
			oss << std::endl;
		}
		oss << "]";
		return oss.str();
	}
}