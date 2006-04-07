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

struct ImageData
{
	ImageData(const wstring& filename, bool relative, bool extend, ImagePosition position, COLORREF background, COLORREF tint, BYTE tintOpacity)
	: strFilename(filename)
	, bRelative(relative)
	, bExtend(extend)
	, imagePosition(position)
	, crBackground(background)
	, crTint(tint)
	, byTintOpacity(tintOpacity)
	, dwOriginalImageWidth(0)
	, dwOriginalImageHeight(0)
	, dwImageWidth(0)
	, dwImageHeight(0)
	, originalImage()
	, image()
	, dcImage()
	{
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

	DWORD				dwOriginalImageWidth;
	DWORD				dwOriginalImageHeight;
	DWORD				dwImageWidth;
	DWORD				dwImageHeight;

	fipImage			originalImage;

	CBitmap				image;
	CDC					dcImage;
};

struct MonitorEnumData
{
	MonitorEnumData(CDC& dcTempl, shared_ptr<ImageData>& imgData)
	: imageData(imgData)
	, dcTemplate(dcTempl)
	{
	}

	shared_ptr<ImageData>&	imageData;
	CDC&					dcTemplate;
};

//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<ImageData> >	Images;

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

		shared_ptr<ImageData> GetImageData(const wstring& strFilename, bool bRelative, bool bExtend, ImagePosition imagePosition, COLORREF crBackground, COLORREF crTint, BYTE byTintOpacity);
		shared_ptr<ImageData> GetDesktopImageData(COLORREF crTint, BYTE byTintOpacity);
		bool LoadImage(shared_ptr<ImageData>& imageData);
		void UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData);

	private:

		void CreateRelativeImage(const CDC& dc, shared_ptr<ImageData>& imageData);
		void CreateImage(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData);

		static void PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, shared_ptr<ImageData>& imageData);
		static void TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, shared_ptr<ImageData>& imageData);

		// called by the ::EnumDisplayMonitors to create background for each display
		static BOOL CALLBACK MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData);


	private:

		Images	m_images;
};

//////////////////////////////////////////////////////////////////////////////
