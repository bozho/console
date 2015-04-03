#pragma once

#include "../FreeImage/FreeImagePlus.h"

//////////////////////////////////////////////////////////////////////////////

enum ImagePosition
{
  /* new names like Win7 walppaper settings */
  imagePositionCenter      = 0,
  imagePositionStretch     = 1,
  imagePositionTile        = 2,
  imagePositionFit         = 3,
  imagePositionFill        = 4,

  /* old names */
  /*
  imgPosCenter             = 0,
  imgPosFit                = 1,
  imgPosTile               = 2,
  imgPosFitWithAspectRatio = 3
  */
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ImageData
{
	ImageData()
	: strFilename(L"")
	, bRelative(false)
	, bExtend(false)
	, imagePosition(imagePositionCenter)
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

	std::shared_ptr<fipImage> originalImage;

	CBitmap				image;
	CDC					dcImage;

	CriticalSection		updateCritSec;
};

struct MonitorEnumData
{
	MonitorEnumData(const CDC& dcTempl, std::shared_ptr<BackgroundImage>& img)
	: bkImage(img)
	, dcTemplate(dcTempl)
	{
	}

	std::shared_ptr<BackgroundImage>&	bkImage;
	const CDC&                        dcTemplate;
};

//////////////////////////////////////////////////////////////////////////////

typedef vector<std::shared_ptr<BackgroundImage> >	Images;

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

		std::shared_ptr<BackgroundImage> GetImage(const ImageData& imageData);
		std::shared_ptr<BackgroundImage> GetDesktopImage(ImageData& imageData);
		void ReloadDesktopImages();

		void UpdateImageBitmap(const CDC& dc, const CRect& clientRect, std::shared_ptr<BackgroundImage>& bkImage);
		static inline bool IsWin8(void) { return m_win8; }

	private:

		static bool GetDesktopImageData(ImageData& imageData);
		static bool LoadImage(std::shared_ptr<BackgroundImage>& bkImage);

		static void CalcRescale(DWORD& dwNewWidth, DWORD& dwNewHeight, std::shared_ptr<BackgroundImage>& bkImage);
		static void PaintRelativeImage(const CDC& dc, CBitmap&	bmpTemplate, std::shared_ptr<BackgroundImage>& bkImage, DWORD& dwDisplayWidth, DWORD& dwDisplayHeight);
		static void CreateRelativeImage(const CDC& dc, std::shared_ptr<BackgroundImage>& bkImage);
		static void CreateImage(const CDC& dc, const CRect& clientRect, std::shared_ptr<BackgroundImage>& bkImage);

		static void PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, std::shared_ptr<BackgroundImage>& bkImage);
		static void TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, std::shared_ptr<BackgroundImage>& bkImage);

		static void TintImage(const CDC& dc, std::shared_ptr<BackgroundImage>& bkImage);

		// called by the ::EnumDisplayMonitors to create background for each display
		static BOOL CALLBACK MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData);
		static BOOL CALLBACK MonitorEnumProcWin8(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData);
		static bool CheckWin8(void);

		static void LoadDesktopWallpaperWin8(MonitorEnumData* pEnumData);

	private:

		Images	m_images;
		static bool	m_win8;
};

//////////////////////////////////////////////////////////////////////////////
