#include "AppFileSystemUtils.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <string>

namespace MikasViewer::FileSystem
{
    namespace
    {
        std::wstring ToLower(const std::wstring& value)
        {
            std::wstring lowered = value;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t ch) {
                return static_cast<wchar_t>(std::towlower(ch));
            });
            return lowered;
        }
    }

    bool IsSupportedImageFileName(const std::wstring& fileName)
    {
        if (fileName.empty())
            return false;

        const auto dotPos = fileName.find_last_of(L'.');
        if (dotPos == std::wstring::npos || dotPos == fileName.size() - 1)
            return false;

        const std::wstring extension = ToLower(fileName.substr(dotPos));
        return extension == L".jpg" || extension == L".jpeg" || extension == L".png" ||
               extension == L".bmp" || extension == L".gif" || extension == L".tif" ||
               extension == L".tiff" || extension == L".webp";
    }

    std::wstring NormalizeDirectoryPath(const std::wstring& path)
    {
        if (path.empty())
            return path;

        if (path.back() == L'\\' || path.back() == L'/')
            return path;

        if (path.size() == 2 && path[1] == L':')
            return path + L"\\";

        return path + L"\\";
    }

    bool DirectoryContainsExpandableContent(const std::wstring& path)
    {
        const std::filesystem::path directory(path);
        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
            return false;

        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_directory())
                return true;

            if (entry.is_regular_file() && IsSupportedImageFileName(entry.path().filename().wstring()))
                return true;
        }

        return false;
    }
}
