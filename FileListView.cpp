#include "pch.h"
#include "FileListView.h"
#include "AppFileSystemUtils.h"
#include "MainFrm.h"
#include <shobjidl.h>

IMPLEMENT_DYNCREATE(CFileListView, CListView)

BEGIN_MESSAGE_MAP(CFileListView, CListView)
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CFileListView::OnItemChanged)
    ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, &CFileListView::OnItemActivate)
    ON_WM_TIMER()
END_MESSAGE_MAP()

namespace
{
    const UINT_PTR ThumbnailTimer = 1;
    const int ThumbnailSize = 128;
}

CFileListView::CFileListView() noexcept
{
}

BOOL CFileListView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= LVS_ICON | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SORTASCENDING | LVS_SHAREIMAGELISTS;
    return CListView::PreCreateWindow(cs);
}

void CFileListView::OnInitialUpdate()
{
    CListView::OnInitialUpdate();
    if (!m_thumbnails.GetSafeHandle() &&
        !m_thumbnails.Create(ThumbnailSize, ThumbnailSize, ILC_COLOR32 | ILC_MASK, 0, 32))
    {
        AfxMessageBox(_T("Could not create the thumbnail image list."), MB_ICONERROR);
        return;
    }
    GetListCtrl().SetImageList(&m_thumbnails, LVSIL_NORMAL);
}

void CFileListView::ShowFolder(const CString& path, const CString& selectedFile)
{
    KillTimer(ThumbnailTimer);
    m_updating = true;
    CListCtrl& list = GetListCtrl();
    list.DeleteAllItems();
    list.SetImageList(nullptr, LVSIL_NORMAL);
    if (m_thumbnails.GetSafeHandle())
        m_thumbnails.DeleteImageList();
    if (!m_thumbnails.Create(ThumbnailSize, ThumbnailSize, ILC_COLOR32 | ILC_MASK, 0, 32))
    {
        m_folder.Empty();
        m_updating = false;
        AfxMessageBox(_T("Could not create the thumbnail image list."), MB_ICONERROR);
        return;
    }
    list.SetImageList(&m_thumbnails, LVSIL_NORMAL);
    m_folder = path;
    if (!m_folder.IsEmpty() && m_folder.Right(1) != _T("\\"))
        m_folder += _T("\\");

    WIN32_FIND_DATA data = {};
    HANDLE search = FindFirstFile(m_folder + _T("*"), &data);
    if (search == INVALID_HANDLE_VALUE)
    {
        const DWORD error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND)
        {
            CString message;
            message.Format(_T("Could not read folder:\n%s\nWindows error: %lu"), m_folder.GetString(), error);
            AfxMessageBox(message, MB_ICONERROR);
        }
    }
    else
    {
        do
        {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                MikasViewer::FileSystem::IsSupportedImageFileName(data.cFileName))
                list.InsertItem(list.GetItemCount(), data.cFileName, I_IMAGENONE);
        } while (FindNextFile(search, &data));
        const DWORD error = GetLastError();
        FindClose(search);
        if (error != ERROR_NO_MORE_FILES)
        {
            CString message;
            message.Format(_T("Could not finish reading folder:\n%s\nWindows error: %lu"), m_folder.GetString(), error);
            AfxMessageBox(message, MB_ICONERROR);
        }
    }

    int selected = list.GetItemCount() > 0 ? 0 : -1;
    if (!selectedFile.IsEmpty())
    {
        selected = -1;
        for (int i = 0; i < list.GetItemCount(); ++i)
        {
            if (list.GetItemText(i, 0).CompareNoCase(selectedFile) == 0)
            {
                selected = i;
                break;
            }
        }
    }
    m_updating = false;
    if (selected >= 0)
    {
        list.SetItemState(selected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        list.EnsureVisible(selected, FALSE);
    }

    m_nextThumbnail = 0;
    if (list.GetItemCount() > 0 && !SetTimer(ThumbnailTimer, 10, nullptr))
        AfxMessageBox(_T("Could not start thumbnail loading."), MB_ICONERROR);
}

bool CFileListView::CanNavigateImage(ImageNavigation navigation) const
{
    const CListCtrl& list = GetListCtrl();
    const int current = list.GetNextItem(-1, LVNI_SELECTED);
    const int target = GetImageNavigationTarget(current, list.GetItemCount(), navigation);
    return target >= 0 && target != current;
}

bool CFileListView::HasSelectedImage() const
{
    return GetListCtrl().GetNextItem(-1, LVNI_SELECTED) >= 0;
}

void CFileListView::NavigateImage(ImageNavigation navigation)
{
    CListCtrl& list = GetListCtrl();
    const int current = list.GetNextItem(-1, LVNI_SELECTED);
    const int target = GetImageNavigationTarget(current, list.GetItemCount(), navigation);
    if (target >= 0)
        SelectImage(current, target);
}

bool CFileListView::AdvanceSlideshow(bool repeat)
{
    CListCtrl& list = GetListCtrl();
    const int current = list.GetNextItem(-1, LVNI_SELECTED);
    const int target = GetSlideshowTarget(current, list.GetItemCount(), repeat);
    if (target < 0)
        return false;
    SelectImage(current, target);
    return true;
}

void CFileListView::SelectImage(int current, int target)
{
    if (target == current)
        return;

    CListCtrl& list = GetListCtrl();
    list.SetItemState(current, 0, LVIS_SELECTED | LVIS_FOCUSED);
    list.SetItemState(target, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    list.EnsureVisible(target, FALSE);
}

void CFileListView::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    const NMLISTVIEW* change = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
    if (!m_updating && (change->uChanged & LVIF_STATE) &&
        (change->uNewState & LVIS_SELECTED) && !(change->uOldState & LVIS_SELECTED))
    {
        CString* path = new CString(m_folder + GetListCtrl().GetItemText(change->iItem, 0));
        GetParentFrame()->SendMessage(WM_FILE_LIST_SELECTED, 0, reinterpret_cast<LPARAM>(path));
    }
    *pResult = 0;
}

void CFileListView::OnItemActivate(NMHDR* pNMHDR, LRESULT* pResult)
{
    const NMITEMACTIVATE* item = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    if (item->iItem >= 0)
        GetParentFrame()->SendMessage(WM_COMMAND, ID_MODE_VIEW);
    *pResult = 0;
}

void CFileListView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != ThumbnailTimer)
    {
        CListView::OnTimer(nIDEvent);
        return;
    }

    CListCtrl& list = GetListCtrl();
    if (m_nextThumbnail >= list.GetItemCount())
    {
        KillTimer(ThumbnailTimer);
        return;
    }

    const int item = m_nextThumbnail++;
    const CString path = m_folder + list.GetItemText(item, 0);
    IShellItemImageFactory* factory = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(path, nullptr, IID_PPV_ARGS(&factory));
    if (SUCCEEDED(hr))
    {
        HBITMAP bitmap = nullptr;
        const SIZE size = { ThumbnailSize, ThumbnailSize };
        hr = factory->GetImage(size, SIIGBF_THUMBNAILONLY, &bitmap);
        if (SUCCEEDED(hr))
        {
            CBitmap source;
            source.Attach(bitmap);
            BITMAP details = {};
            source.GetBitmap(&details);
            CDC screen;
            CDC sourceDC;
            CDC thumbnailDC;
            CBitmap thumbnail;
            const bool created = screen.CreateIC(_T("DISPLAY"), nullptr, nullptr, nullptr) &&
                sourceDC.CreateCompatibleDC(&screen) && thumbnailDC.CreateCompatibleDC(&screen) &&
                thumbnail.CreateCompatibleBitmap(&screen, ThumbnailSize, ThumbnailSize);
            int index = -1;
            if (created)
            {
                CBitmap* oldSource = sourceDC.SelectObject(&source);
                CBitmap* oldThumbnail = thumbnailDC.SelectObject(&thumbnail);
                thumbnailDC.FillSolidRect(0, 0, ThumbnailSize, ThumbnailSize, RGB(240, 240, 240));
                thumbnailDC.BitBlt((ThumbnailSize - details.bmWidth) / 2,
                    (ThumbnailSize - details.bmHeight) / 2, details.bmWidth, details.bmHeight,
                    &sourceDC, 0, 0, SRCCOPY);
                sourceDC.SelectObject(oldSource);
                thumbnailDC.SelectObject(oldThumbnail);
                index = m_thumbnails.Add(&thumbnail, static_cast<CBitmap*>(nullptr));
            }
            if (index >= 0)
                list.SetItem(item, 0, LVIF_IMAGE, nullptr, index, 0, 0, 0);
            else
                TRACE(_T("Could not add thumbnail: %s\n"), path.GetString());
        }
        factory->Release();
    }
    if (FAILED(hr))
        TRACE(_T("Could not load thumbnail: %s (0x%08lx)\n"), path.GetString(), hr);
}
