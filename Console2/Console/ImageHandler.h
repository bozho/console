#pragma once

#include "../FreeImage/FreeImagePlus.h"

//////////////////////////////////////////////////////////////////////////////

enum SizeStyle {

	sizeStyleCenter	= 0,
    sizeStyleResize	= 1,
	sizeStyleTile	= 2
};

//////////////////////////////////////////////////////////////////////////////

struct ImageData {

	ImageData(const wstring& filename, bool relative, bool extend, SizeStyle style, COLORREF background, COLORREF tint, BYTE tintOpacity)
	: strFilename(filename)
	, bRelative(relative)
	, bExtend(extend)
	, sizeStyle(style)
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

	bool operator==(const ImageData& other) const {

		if (strFilename != other.strFilename)		return false;
		if (bRelative != other.bRelative)			return false;
		if (bExtend != other.bExtend)				return false;
		if (sizeStyle != other.sizeStyle)			return false;
		if (crBackground != other.crBackground)		return false;
		if (crTint != other.crTint)					return false;
		if (byTintOpacity != other.byTintOpacity)	return false;

		return true;
	}

	wstring				strFilename;

	bool				bRelative;
	bool				bExtend;
	SizeStyle			sizeStyle;

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

//////////////////////////////////////////////////////////////////////////////

typedef vector<shared_ptr<ImageData> >	Images;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class ImageHandler {

	public:

		ImageHandler();
		~ImageHandler();

	public:

		shared_ptr<ImageData> GetImageData(const wstring& strFilename, bool bRelative, bool bExtend, SizeStyle sizeStyle, COLORREF crBackground, COLORREF crTint, BYTE byTintOpacity);
		shared_ptr<ImageData> GetDesktopImageData(COLORREF crTint, BYTE byTintOpacity);
		bool LoadImage(shared_ptr<ImageData>& imageData);
		void UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData);

	private:

		void CreateRelativeImage(const CDC& dc, shared_ptr<ImageData>& imageData);
		void CreateImage(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData);

	private:

		Images	m_images;
};

//////////////////////////////////////////////////////////////////////////////
