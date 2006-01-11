#pragma once

#include "../FreeImage/FreeImagePlus.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

struct ImageData {

	ImageData(const wstring& filename, bool relative, bool resize, bool extend, COLORREF tint, BYTE tintOpacity)
	: strFilename(filename)
	, bRelative(relative)
	, bResize(resize)
	, bExtend(extend)
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
		if (bResize != other.bResize)				return false;
		if (bExtend != other.bExtend)				return false;
		if (crTint != other.crTint)					return false;
		if (byTintOpacity != other.byTintOpacity)	return false;

		return true;
	}

	wstring				strFilename;

	bool				bRelative;
	bool				bResize;
	bool				bExtend;

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

		shared_ptr<ImageData> GetImageData(const wstring& strFilename, bool bRelative, bool bResize, bool bExtend, COLORREF crTint, BYTE byTintOpacity);
		void LoadImage(shared_ptr<ImageData>& imageData);
		void UpdateImageBitmap(const CDC& dc, const RECT& clientRect, shared_ptr<ImageData>& imageData);
		void AdjustBackgroundImage(shared_ptr<ImageData>& imageData, DWORD dwWidth, DWORD dwHeight);

	private:

	private:

		Images	m_images;
};

//////////////////////////////////////////////////////////////////////////////
