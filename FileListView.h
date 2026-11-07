#pragma once

#include <afxcview.h>
#include "ImageNavigation.h"

class CFileListView : public CListView
{
protected:
    CFileListView() noexcept;
    DECLARE_DYNCREATE(CFileListView)

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    void ShowFolder(const CString& path, const CString& selectedFile = CString());
    bool CanNavigateImage(ImageNavigation navigation) const;
    bool HasSelectedImage() const;
    void NavigateImage(ImageNavigation navigation);
    bool AdvanceSlideshow(bool repeat);

protected:
    afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemActivate(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    DECLARE_MESSAGE_MAP()

private:
    void SelectImage(int current, int target);
    CString m_folder;
    CImageList m_thumbnails;
    int m_nextThumbnail = 0;
    bool m_updating = false;
};
