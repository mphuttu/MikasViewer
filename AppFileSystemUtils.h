#pragma once

#include <string>

namespace MikasViewer::FileSystem
{
    bool IsSupportedImageFileName(const std::wstring& fileName);
    std::wstring NormalizeDirectoryPath(const std::wstring& path);
    bool DirectoryContainsExpandableContent(const std::wstring& path);
}
