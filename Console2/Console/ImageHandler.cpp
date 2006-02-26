#include "StdAfx.h"
#include "ImageHandler.h"

//////////////////////////////////////////////////////////////////////////////

ImageHandler::ImageHandler()
: m_images()
{
}

ImageHandler::~ImageHandler()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<ImageData> ImageHandler::GetImageData(const wstring& strFilename, bool bRelative, bool bResize, bool bExtend, COLORREF crTint, BYTE byTintOpacity) {

	shared_ptr<ImageData>	imageData(new ImageData(strFilename, bRelative, bResize, bExtend, crTint, byTintOpacity));
	Images::iterator		itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if (*(*itImage) == *imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	m_images.push_back(imageData);
	return imageData;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<ImageData> ImageHandler::GetDesktopImageData(COLORREF crTint, BYTE byTintOpacity) {

	shared_ptr<ImageData>	imageData;
	CRegKey					keyColors;

	if (keyColors.Open(HKEY_CURRENT_USER, L"Control Panel\\Colors", KEY_READ) != ERROR_SUCCESS) return imageData;

	CString	strBackground;
	DWORD	dwDataSize = 32;

	if (keyColors.QueryStringValue(L"Background", strBackground.GetBuffer(dwDataSize), &dwDataSize) != ERROR_SUCCESS) {
		strBackground.ReleaseBuffer();
		return imageData;
	}
	strBackground.ReleaseBuffer();

	imageData.reset(new ImageData(L"", true, true, true, crTint, byTintOpacity));

	wstringstream	streamBk(wstring(strBackground.operator LPCTSTR()));
	DWORD			r = 0;
	DWORD			g = 0;
	DWORD			b = 0;

	streamBk >> r;
	streamBk >> g;
	streamBk >> b;

	imageData->crBackground = RGB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));

	CRegKey keyDesktop;
	if (keyDesktop.Open(HKEY_CURRENT_USER, L"Control Panel\\Desktop", KEY_READ) != ERROR_SUCCESS) return imageData;

	DWORD	dwChars = MAX_PATH;
	CString	strWallpaperFile;
	DWORD	dwWallpaperStyle= 0;
	DWORD	dwWallpaperTile	= 0;

	keyDesktop.QueryDWORDValue(L"WallpaperStyle", dwWallpaperStyle);
	keyDesktop.QueryDWORDValue(L"TileWallpaper", dwWallpaperStyle);
	keyDesktop.QueryStringValue(L"Wallpaper", strWallpaperFile.GetBuffer(dwChars), &dwChars);
	strWallpaperFile.ReleaseBuffer();

	imageData->strFilename = strWallpaperFile;

	if (dwWallpaperTile == 1) {
		imageData->imgStyle = imgStyleTile;
	} else {
		if (dwWallpaperStyle == 0) {
			imageData->imgStyle = imgStyleCenter;
		} else {
			imageData->imgStyle = imgStyleResize;
		}
	}

	return imageData;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::LoadImage(shared_ptr<ImageData>& imageData) {

	USES_CONVERSION;

	if (imageData.get() == NULL) return false;
	if (imageData->originalImage.isValid()) return true;

	// load background image
	if (!imageData->originalImage.load(W2A(imageData->strFilename.c_str()))) return false;

	imageData->dwOriginalImageWidth	= imageData->originalImage.getWidth();
	imageData->dwOriginalImageHeight= imageData->originalImage.getHeight();

	imageData->originalImage.convertTo24Bits();
	
	// ... if needed, tint the background image
	if (imageData->byTintOpacity > 0) {
		
		BYTE*	pPixels = imageData->originalImage.accessPixels();
		BYTE*	pPixelsEnd =  pPixels + 3*imageData->originalImage.getWidth()*imageData->originalImage.getHeight();
		BYTE*	pPixelSubel = pPixels;

		while (pPixelSubel < pPixelsEnd) {

			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - imageData->byTintOpacity) + GetBValue(imageData->crTint)*imageData->byTintOpacity)/100); 
			++pPixelSubel;
			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - imageData->byTintOpacity) + GetGValue(imageData->crTint)*imageData->byTintOpacity)/100);
			++pPixelSubel;
			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - imageData->byTintOpacity) + GetRValue(imageData->crTint)*imageData->byTintOpacity)/100);
			++pPixelSubel;
		}
	}

	// create background CDC and paint bitmap
	imageData->dcImage.CreateCompatibleDC(NULL);

	m_images.push_back(imageData);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData) {

	DWORD dwClientWidth = clientRect.right - clientRect.left;
	DWORD dwClientHeight= clientRect.bottom - clientRect.top;

	if (!imageData->bRelative && 
		(imageData->dwImageWidth == dwClientWidth) &&
		(imageData->dwImageHeight == dwClientHeight)) {

		// non-relative image, no client size change, nothing to do
		return;
	}

	if (imageData->bRelative) {

		if (!imageData->image.IsNull()) return;
		// first access to relative image, create it
		CreateRelativeImage(dc, imageData);

	} else {

		// resize background image
		fipImage tempImage(imageData->originalImage);

		tempImage.rescale(static_cast<WORD>(dwClientWidth), static_cast<WORD>(dwClientHeight), FILTER_BILINEAR);

		if (!imageData->image.IsNull()) imageData->image.DeleteObject();
		imageData->image.CreateDIBitmap(
							dc, 
							tempImage.getInfoHeader(), 
							CBM_INIT, 
							tempImage.accessPixels(), 
							tempImage.getInfo(), 
							DIB_RGB_COLORS);

		imageData->dcImage.SelectBitmap(imageData->image);

		imageData->dwImageWidth = dwClientWidth;
		imageData->dwImageHeight= dwClientHeight;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateRelativeImage(const CDC& dc, shared_ptr<ImageData>& imageData) {

	fipImage tempImage(imageData->originalImage);

/*
	DWORD	dwPrimaryDisplayWidth	= ::GetSystemMetrics(SM_CXSCREEN);
	DWORD	dwPrimaryDisplayHeight	= ::GetSystemMetrics(SM_CYSCREEN);
	
	DWORD	dwBackgroundWidth		= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	DWORD	dwBackgroundHeight		= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);


	if (imageData->bExtend) {
		imageData->dwImageWidth	= dwBackgroundWidth;
		imageData->dwImageHeight= dwBackgroundHeight;
	} else {
		imageData->dwImageWidth	= dwPrimaryDisplayWidth;
		imageData->dwImageHeight= dwPrimaryDisplayHeight;
	}
*/

	// for relative background, background size is equal to virtual screen size
	imageData->dwImageWidth	= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	imageData->dwImageHeight= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
			
/*
	// get offsets for virtual display
	m_nBackgroundOffsetX = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_nBackgroundOffsetY = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
*/


	tempImage.rescale(static_cast<WORD>(imageData->dwImageWidth), static_cast<WORD>(imageData->dwImageHeight), FILTER_BILINEAR);

	if (!imageData->image.IsNull()) imageData->image.DeleteObject();
	imageData->image.CreateDIBitmap(
						dc, 
						tempImage.getInfoHeader(), 
						CBM_INIT, 
						tempImage.accessPixels(), 
						tempImage.getInfo(), 
						DIB_RGB_COLORS);

	imageData->dcImage.SelectBitmap(imageData->image);
}

//////////////////////////////////////////////////////////////////////////////

