//! \file
//! \brief Provides Gallery configuration screens

#ifndef GALLERYCONFIG_H
#define GALLERYCONFIG_H

#include "libmythui/standardsettings.h"

class GallerySettings : public GroupSetting
{
    Q_OBJECT

    StandardSetting *DirOrder() const;
    StandardSetting *ImageOrder() const;
    StandardSetting *ImageMaximumSize() const;
    StandardSetting *DateFormat() const;
    StandardSetting *Exclusions (bool enabled) const;
    StandardSetting *ClearDb    (bool enabled) const;
    void             ShowConfirmDialog();

signals:
    void ClearDbPressed();
    void DateChanged();
    void OrderChanged();
    void ExclusionsChanged();

private slots:
    static void ImageSizeChanged();

public:
    explicit GallerySettings(bool enable);
};

#endif // GALLERYCONFIG_H
