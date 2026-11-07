#pragma once

enum class ImageNavigation
{
    Previous,
    Next,
    First,
    Last
};

constexpr int GetImageNavigationTarget(int current, int count, ImageNavigation navigation)
{
    if (current < 0 || current >= count)
        return -1;

    switch (navigation)
    {
    case ImageNavigation::Previous: return current > 0 ? current - 1 : current;
    case ImageNavigation::Next: return current + 1 < count ? current + 1 : current;
    case ImageNavigation::First: return 0;
    case ImageNavigation::Last: return count - 1;
    }
    return -1;
}

constexpr int GetSlideshowTarget(int current, int count, bool repeat)
{
    if (current < 0 || current >= count)
        return -1;
    if (current + 1 < count)
        return current + 1;
    return repeat ? 0 : -1;
}
