#pragma once
#include <afxcmn.h>
#include <afxcview.h>

class CFolderTreeView : public CTreeView
{
public:
    CFolderTreeView() noexcept;
    DECLARE_DYNCREATE(CFolderTreeView)

public:
    virtual void OnInitialUpdate();
    virtual ~CFolderTreeView();

    void AddSubFolders(HTREEITEM hParent, const CString& strPath);

private:
    int  GetSysIconIndex(const CString& path, bool bOpen = false);
    bool HasExpandableContent(const CString& strPath);
    static bool IsImageFile(const CString& fileName);

protected:
    afx_msg void OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTvnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTvnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
};

