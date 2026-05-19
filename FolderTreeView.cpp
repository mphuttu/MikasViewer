#pragma once
#include "pch.h"
#include "FolderTreeView.h"
#include "MainFrm.h"
#include <shellapi.h>

IMPLEMENT_DYNCREATE(CFolderTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CFolderTreeView, CTreeView)
    ON_NOTIFY_REFLECT(TVN_SELCHANGED,    &CFolderTreeView::OnTvnSelchanged)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, &CFolderTreeView::OnTvnItemExpanding)
    ON_NOTIFY_REFLECT(TVN_DELETEITEM,    &CFolderTreeView::OnTvnDeleteItem)
END_MESSAGE_MAP()

CFolderTreeView::CFolderTreeView() noexcept
{}

CFolderTreeView::~CFolderTreeView()
{}

// Hakee Windowsin järjestelmäikonikuvaluettelon indeksin polun ikonille
int CFolderTreeView::GetSysIconIndex(const CString& path, bool bOpen)
{
    SHFILEINFO sfi = {};
    DWORD flags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
    if (bOpen) flags |= SHGFI_OPENICON;
    SHGetFileInfo(path, 0, &sfi, sizeof(sfi), flags);
    return sfi.iIcon;
}

// Tarkistaa onko tiedosto tuettu kuvaformaatti
/*static*/ bool CFolderTreeView::IsImageFile(const CString& fileName)
{
    int dotPos = fileName.ReverseFind(_T('.'));
    if (dotPos < 0) return false;
    CString ext = fileName.Mid(dotPos);
    ext.MakeLower();
    return ext == _T(".jpg")  || ext == _T(".jpeg") ||
           ext == _T(".png")  || ext == _T(".bmp")  ||
           ext == _T(".gif")  || ext == _T(".tif")  ||
           ext == _T(".tiff") || ext == _T(".webp");
}

// Tarkistaa onko kansiossa alikansioita tai kuvatiedostoja (expand-painiketta varten)
bool CFolderTreeView::HasExpandableContent(const CString& strPath)
{
    UINT oldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    CFileFind finder;
    CString searchPath = strPath;
    if (searchPath.Right(1) != _T("\\")) searchPath += _T("\\");
    searchPath += _T("*.*");

    BOOL bWorking = finder.FindFile(searchPath);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();
        if (finder.IsDots()) continue;
        if (finder.IsDirectory() || IsImageFile(finder.GetFileName()))
        {
            finder.Close();
            SetErrorMode(oldMode);
            return true;
        }
    }
    SetErrorMode(oldMode);
    return false;
}

void CFolderTreeView::OnInitialUpdate()
{
    CTreeView::OnInitialUpdate();

    CTreeCtrl& tree = GetTreeCtrl();
    tree.ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS);

    // Aseta Windowsin järjestelmän image list (ikonit kuten Explorerissa)
    SHFILEINFO sfi = {};
    HIMAGELIST hSysImgList = (HIMAGELIST)SHGetFileInfo(
        _T("C:\\"), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
    if (hSysImgList)
        TreeView_SetImageList(tree.m_hWnd, hSysImgList, TVSIL_NORMAL);

    // Estä "levy ei ole valmis" -dialogi inaccessible-asemilla
    UINT oldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    // Lisää kaikki loogiset asemat (C:\, D:\, jne.)
    TCHAR szDrives[512] = {};
    GetLogicalDriveStrings(_countof(szDrives), szDrives);
    for (TCHAR* p = szDrives; *p; p += _tcslen(p) + 1)
    {
        CString drivePath(p);

        // Hakee aseman näyttönimen ja ikonin kuten Explorerissa (esim. "Windows (C:)")
        SHFILEINFO driveInfo = {};
        SHGetFileInfo(drivePath, 0, &driveInfo, sizeof(driveInfo),
            SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

        CString displayName = driveInfo.szDisplayName[0]
            ? CString(driveInfo.szDisplayName) : drivePath;
        int iIcon     = driveInfo.iIcon;
        int iOpenIcon = GetSysIconIndex(drivePath, true);

        HTREEITEM hDrive = tree.InsertItem(displayName, iIcon, iOpenIcon, TVI_ROOT, TVI_LAST);
        tree.SetItemData(hDrive, (DWORD_PTR)new CString(drivePath));

        if (HasExpandableContent(drivePath))
            tree.InsertItem(_T(""), 0, 0, hDrive, TVI_LAST); // dummy → näyttää +
    }

    SetErrorMode(oldMode);
}

// Lisää alikansiot ja kuvatiedostot hParent-solmuun lazysti
void CFolderTreeView::AddSubFolders(HTREEITEM hParent, const CString& strPath)
{
    CTreeCtrl& tree = GetTreeCtrl();

    // Poista dummy-lapsi
    HTREEITEM hChild = tree.GetChildItem(hParent);
    if (hChild != NULL && tree.GetItemData(hChild) == 0)
        tree.DeleteItem(hChild);

    UINT oldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    CFileFind finder;
    CString searchPath = strPath;
    if (searchPath.Right(1) != _T("\\")) searchPath += _T("\\");
    searchPath += _T("*.*");

    BOOL bWorking = finder.FindFile(searchPath);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();
        if (finder.IsDots()) continue;

        CString name     = finder.GetFileName();
        CString fullPath = finder.GetFilePath();

        if (finder.IsDirectory())
        {
            if (fullPath.Right(1) != _T("\\")) fullPath += _T("\\");

            int iIcon     = GetSysIconIndex(fullPath, false);
            int iOpenIcon = GetSysIconIndex(fullPath, true);

            HTREEITEM hItem = tree.InsertItem(name, iIcon, iOpenIcon, hParent, TVI_SORT);
            tree.SetItemData(hItem, (DWORD_PTR)new CString(fullPath));

            // Dummy jos kansiossa on lisää sisältöä → näyttää +
            if (HasExpandableContent(fullPath))
                tree.InsertItem(_T(""), 0, 0, hItem, TVI_LAST);
        }
        else if (IsImageFile(name))
        {
            int iIcon = GetSysIconIndex(fullPath, false);
            HTREEITEM hItem = tree.InsertItem(name, iIcon, iIcon, hParent, TVI_SORT);
            tree.SetItemData(hItem, (DWORD_PTR)new CString(fullPath));
            // Tiedostoilla ei lapsia
        }
    }
    finder.Close();
    SetErrorMode(oldMode);
}

// Käyttäjä avaa (+) solmun → lataa oikeat lapset lazysti
void CFolderTreeView::OnTvnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTREEVIEW pNMTV = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;

    if (pNMTV->action != TVE_EXPAND)
        return;

    CTreeCtrl& tree = GetTreeCtrl();
    HTREEITEM hItem = pNMTV->itemNew.hItem;

    // Jos ensimmäinen lapsi on dummy (data == 0), lataa oikeat alikansiot
    HTREEITEM hFirstChild = tree.GetChildItem(hItem);
    if (hFirstChild != NULL && tree.GetItemData(hFirstChild) == 0)
    {
        CString* pPath = reinterpret_cast<CString*>(tree.GetItemData(hItem));
        if (pPath)
            AddSubFolders(hItem, *pPath);
    }
}

// Käyttäjä valitsee kansion tai tiedoston → lähetä polku MainFramelle
void CFolderTreeView::OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTREEVIEW pNMTV = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    CString* pPath = reinterpret_cast<CString*>(GetTreeCtrl().GetItemData(pNMTV->itemNew.hItem));

    if (pPath && !pPath->IsEmpty())
    {
        CString* pCopy = new CString(*pPath);
        // Kansiot päättyvät '\', tiedostot eivät
        UINT msg = (pPath->Right(1) == _T("\\")) ? WM_FOLDER_SELECTED : WM_FILE_SELECTED;
        GetParentFrame()->SendMessage(msg, 0, (LPARAM)pCopy);
    }
    *pResult = 0;
}

// Solmu poistetaan puusta → vapauta muisti
void CFolderTreeView::OnTvnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTREEVIEW pNMTV = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    CString* pPath = reinterpret_cast<CString*>(pNMTV->itemOld.lParam);
    delete pPath; // delete nullptr on turvallista C++:ssa
    *pResult = 0;
}