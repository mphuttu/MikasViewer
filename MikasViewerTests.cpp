#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>

#include "AppFileSystemUtils.h"
#include "ImageNavigation.h"

namespace
{
    std::filesystem::path CreateTestDirectory()
    {
        const auto root = std::filesystem::temp_directory_path() /
            (std::wstring(L"mikasviewer-tests-") + std::to_wstring(GetCurrentProcessId()));
        std::filesystem::remove_all(root);
        std::filesystem::create_directories(root);
        return root;
    }
}

TEST(AppFileSystemUtils, RecognizesSupportedImageExtensions)
{
    EXPECT_TRUE(MikasViewer::FileSystem::IsSupportedImageFileName(L"photo.jpg"));
    EXPECT_TRUE(MikasViewer::FileSystem::IsSupportedImageFileName(L"frame.PNG"));
    EXPECT_TRUE(MikasViewer::FileSystem::IsSupportedImageFileName(L"image.webp"));
    EXPECT_FALSE(MikasViewer::FileSystem::IsSupportedImageFileName(L"notes.txt"));
    EXPECT_FALSE(MikasViewer::FileSystem::IsSupportedImageFileName(L"archive"));
    EXPECT_FALSE(MikasViewer::FileSystem::IsSupportedImageFileName(L"file."));
}

TEST(AppFileSystemUtils, NormalizesDirectoryPaths)
{
    EXPECT_EQ(L"C:\\", MikasViewer::FileSystem::NormalizeDirectoryPath(L"C:"));
    EXPECT_EQ(L"C:\\Images\\", MikasViewer::FileSystem::NormalizeDirectoryPath(L"C:\\Images"));
    EXPECT_EQ(L"D:\\Folder\\", MikasViewer::FileSystem::NormalizeDirectoryPath(L"D:\\Folder\\"));
}

TEST(AppFileSystemUtils, DetectsExpandableContent)
{
    const auto root = CreateTestDirectory();
    const auto nestedDirectory = root / L"nested";
    const auto imageFile = root / L"sample.jpg";

    std::filesystem::create_directories(nestedDirectory);
    std::ofstream output(imageFile, std::ios::binary);
    output << "placeholder";
    output.close();

    EXPECT_TRUE(MikasViewer::FileSystem::DirectoryContainsExpandableContent(root.wstring()));

    std::filesystem::remove_all(root);
}

TEST(ImageNavigation, StaysWithinTheSelectedFolder)
{
    EXPECT_EQ(-1, GetImageNavigationTarget(-1, 0, ImageNavigation::Next));
    EXPECT_EQ(-1, GetImageNavigationTarget(-1, 3, ImageNavigation::First));
    EXPECT_EQ(-1, GetImageNavigationTarget(-1, 3, ImageNavigation::Last));
    EXPECT_EQ(-1, GetImageNavigationTarget(3, 3, ImageNavigation::Last));
    EXPECT_EQ(0, GetImageNavigationTarget(0, 3, ImageNavigation::Previous));
    EXPECT_EQ(1, GetImageNavigationTarget(0, 3, ImageNavigation::Next));
    EXPECT_EQ(0, GetImageNavigationTarget(1, 3, ImageNavigation::Previous));
    EXPECT_EQ(2, GetImageNavigationTarget(1, 3, ImageNavigation::Next));
    EXPECT_EQ(2, GetImageNavigationTarget(2, 3, ImageNavigation::Next));
    EXPECT_EQ(0, GetImageNavigationTarget(2, 3, ImageNavigation::First));
    EXPECT_EQ(2, GetImageNavigationTarget(0, 3, ImageNavigation::Last));
    EXPECT_EQ(0, GetImageNavigationTarget(0, 1, ImageNavigation::Previous));
    EXPECT_EQ(0, GetImageNavigationTarget(0, 1, ImageNavigation::Next));
    EXPECT_EQ(0, GetImageNavigationTarget(0, 1, ImageNavigation::First));
    EXPECT_EQ(0, GetImageNavigationTarget(0, 1, ImageNavigation::Last));
}

TEST(ImageNavigation, SlideshowAdvancesAndRepeatsOnlyWhenRequested)
{
    EXPECT_EQ(-1, GetSlideshowTarget(-1, 0, true));
    EXPECT_EQ(-1, GetSlideshowTarget(-1, 3, true));
    EXPECT_EQ(-1, GetSlideshowTarget(3, 3, true));
    EXPECT_EQ(1, GetSlideshowTarget(0, 3, false));
    EXPECT_EQ(2, GetSlideshowTarget(1, 3, true));
    EXPECT_EQ(-1, GetSlideshowTarget(2, 3, false));
    EXPECT_EQ(0, GetSlideshowTarget(2, 3, true));
    EXPECT_EQ(-1, GetSlideshowTarget(0, 1, false));
    EXPECT_EQ(0, GetSlideshowTarget(0, 1, true));
}
