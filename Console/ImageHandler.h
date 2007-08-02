#pragma once

#include "../FreeImage/FreeImagePlus.h"

//////////////////////////////////////////////////////////////////////////////

enum ImagePosition
{
	imgPosCenter	= 0,
	imgPosFit		= 1,
	imgPosTile		= 2
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ImageData
{
	ImageData()
	: strFilename(L"")
	, bRelative(false)
	, bExtend(false)
	, imagePosition(imgPosCenter)
	, crBackground(RGB(0, 0, 0))
	, crTint(RGB(0, 0, 0))
	, byTintOpacity(0)
	{
	}

	ImageData(const wstring& filename, bool relative, bool extend, ImagePosition position, COLORREF background, COLORREF tint, BYTE tintOpacity)
	: strFilename(filename)
	, bRelative(relative)
	, bExtend(extend)
	, imagePosition(position)
	, crBackground(background)
	, crTint(tint)
	, byTintOpacity(tintOpacity)
	{
	}

	ImageData(const ImageData& other)
	: strFilename(other.strFilename)
	, bRelative(other.bRelative)
	, bExtend(other.bExtend)
	, imagePosition(other.imagePosition)
	, crBackground(other.crBackground)
	, crTint(other.crTint)
	, byTintOpacity(other.byTintOpacity)
	{
	}

	ImageData& operator=(const ImageData& other)
	{
		strFilename		= other.strFilename;
		bRelative		= other.bRelative;
		bExtend			= other.bExtend;
		imagePosition	= other.imagePosition;
		crBackground	= other.crBackground;
		crTint			= other.crTint;
		byTintOpacity	= other.byTintOpacity;

		return *this;
	}

	bool operator==(const ImageData& other) const
	{
		if (strFilename != other.strFilename)		return false;
		if (bRelative != other.bRelative)			return false;
		if (bExtend != other.bExtend)				return false;
		if (imagePosition != other.imagePosition)	return false;
		if (crBackground != other.crBackground)		return false;
		if (crTint != other.crTint)					return false;
		if (byTintOpacity != other.byTintOpacity)	return false;

		return true;
	}

	wstring				strFilename;

	bool				bRelative;
	bool				bExtend;
	ImagePosition		imagePosition;

	COLORREF			crBackground;

	COLORREF			crTint;
	BYTE				byTintOpacity;

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct BackgroundImage
{
	BackgroundImage(const ImageData& data)
	: imageData(data)
	, dwOriginalImageWidth(0)
	, dwOriginalImageHeight(0)
	, dwImageWidth(0)
	, dwImageHeight(0)
	, bWallpaper(false)
	, originalImage()
	, image()
	, dcImage()
	, updateCritSec()
	{
	}

	ImageData			imageData;

	DWORD				dwOriginalImageWidth;
	DWORD				dwOriginalImageHeight;
	DWORD				dwImageWidth;
	DWORD				dwImageHeight;

	bool				bWallpaper;

	shared_ptr<fipImage> originalImage;

	CBitmap				image;
	CDC					dcImage;

	CriticalSection		updateCritSec;
};

struct MonitorEnumData
{
	MonitorEnumData(CDC& dcTempl, shared_ptr<BackgroundImage>& img)
	: bkImage(img)
	, dcTemplate(dcTempl)
	{
	}

	shared_ptr<BackgroundImage>&	bkImage;
	CDC&							dcTemplate;
};

//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<BackgroundImage> >	Images;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ImageHandler
{
	public:

		ImageHandler();
		~ImageHandler();

	public:

		shared_ptr<BackgroundImage> GetImage(const ImageData& imageData);
		shared_ptr<BackgroundImage> GetDesktopImage(ImageData& imageData);
		void ReloadDesktopImages();

		void UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<BackgroundImage>& bkImage);

	private:

		bool GetDesktopImageData(ImageData& imageData);
		bool LoadImage(shared_ptr<BackgroundImage>& bkImage);

		void CreateRelativeImage(const CDC& dc, shared_ptr<BackgroundImage>& bkImage);
		void CreateImage(const CDC& dc, const CRect& clientRect, shared_ptr<BackgroundImage>& bkImage);

		static void PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, shared_ptr<BackgroundImage>& bkImage);
		static void TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, shared_ptr<BackgroundImage>& bkImage);

		void TintImage(const CDC& dc, shared_ptr<BackgroundImage>& bkImage);

		// called by the ::EnumDisplayMonitors to create background for each display
		static BOOL CALLBACK MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData);


	private:

		Images	m_images;
};

//////////////////////////////////////////////////////////////////////////////
