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

shared_ptr<ImageData> ImageHandler::GetImageData(const wstring& strFilename, bool bRelative, bool bExtend, SizeStyle sizeStyle, COLORREF crBackground, COLORREF crTint, BYTE byTintOpacity) {

	shared_ptr<ImageData>	imageData(new ImageData(
											strFilename, 
											bRelative, 
											bExtend, 
											sizeStyle, 
											crBackground, 
											crTint, 
											byTintOpacity));

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

	wstringstream	streamBk(wstring(strBackground.operator LPCTSTR()));
	DWORD			r = 0;
	DWORD			g = 0;
	DWORD			b = 0;

	streamBk >> r;
	streamBk >> g;
	streamBk >> b;

	imageData.reset(new ImageData(
							L"", 
							true, 
							false, 
							sizeStyleCenter, 
							RGB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b)), 
							crTint, 
							byTintOpacity));

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
		imageData->sizeStyle = sizeStyleTile;
	} else {
		if (dwWallpaperStyle == 0) {
			imageData->sizeStyle = sizeStyleCenter;
		} else {
			imageData->sizeStyle = sizeStyleResize;
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

	if (imageData->bRelative) {
		if (!imageData->image.IsNull()) return;
		// first access to relative image, create it
		CreateRelativeImage(dc, imageData);
	} else {
		CreateImage(dc, clientRect, imageData);
}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateRelativeImage(const CDC& dc, shared_ptr<ImageData>& imageData) {

	DWORD	dwPrimaryDisplayWidth	= ::GetSystemMetrics(SM_CXSCREEN);
	DWORD	dwPrimaryDisplayHeight	= ::GetSystemMetrics(SM_CYSCREEN);

	// get offsets for virtual display
	int		nBackgroundOffsetX	= ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	int		nBackgroundOffsetY	= ::GetSystemMetrics(SM_YVIRTUALSCREEN);

	imageData->dwImageWidth		= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	imageData->dwImageHeight	= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

	CDC		dcTemplate;
	CBitmap	bmpTemplate;

	dcTemplate.CreateCompatibleDC(dc);

	// set template bitmap dimensions
	DWORD	dwTemplateWidth		= imageData->originalImage.getWidth();
	DWORD	dwTemplateHeight	= imageData->originalImage.getHeight();

	if (imageData->sizeStyle == sizeStyleResize) {

		if (imageData->bExtend) {
			dwTemplateWidth	= imageData->dwImageWidth;
			dwTemplateHeight= imageData->dwImageHeight;
		} else {
			dwTemplateWidth	= dwPrimaryDisplayWidth;
			dwTemplateHeight= dwPrimaryDisplayHeight;
		}
	}

	if ((imageData->originalImage.getWidth() != dwTemplateWidth) || 
		(imageData->originalImage.getHeight() != dwTemplateHeight)) {

		// resize background image
		fipImage tempImage(imageData->originalImage);
		tempImage.rescale(static_cast<WORD>(dwTemplateWidth), static_cast<WORD>(dwTemplateHeight), FILTER_BILINEAR);

		bmpTemplate.CreateDIBitmap(
						dc, 
						tempImage.getInfoHeader(), 
						CBM_INIT, 
						tempImage.accessPixels(), 
						tempImage.getInfo(), 
						DIB_RGB_COLORS);
	} else {
		bmpTemplate.CreateDIBitmap(
						dc, 
						imageData->originalImage.getInfoHeader(), 
						CBM_INIT, 
						imageData->originalImage.accessPixels(), 
						imageData->originalImage.getInfo(), 
						DIB_RGB_COLORS);
	}

	dcTemplate.SelectBitmap(bmpTemplate);

	imageData->image.CreateCompatibleBitmap(dc, imageData->dwImageWidth, imageData->dwImageHeight);
	imageData->dcImage.SelectBitmap(imageData->image);

	CBrush	backgroundBrush(::CreateSolidBrush(imageData->crBackground));
	CRect	rect(0, 0, imageData->dwImageWidth, imageData->dwImageHeight);
	imageData->dcImage.FillRect(&rect, backgroundBrush);

	switch (imageData->sizeStyle) {

		case sizeStyleCenter : {

			imageData->dcImage.BitBlt(
						(imageData->dwImageWidth <= imageData->originalImage.getWidth()) ? 0 : (imageData->dwImageWidth - imageData->originalImage.getWidth())/2,
						(imageData->dwImageHeight <= imageData->originalImage.getHeight()) ? 0 : (imageData->dwImageHeight - imageData->originalImage.getHeight())/2,
						imageData->originalImage.getWidth(), 
						imageData->originalImage.getHeight(),
						dcTemplate,
						(imageData->dwImageWidth < imageData->originalImage.getWidth()) ? (imageData->originalImage.getWidth() - imageData->dwImageWidth)/2 : 0,
						(imageData->dwImageHeight < imageData->originalImage.getHeight()) ? (imageData->originalImage.getHeight() - imageData->dwImageHeight)/2 : 0,
						SRCCOPY);
			break;
		}

		case sizeStyleResize : {

			imageData->dcImage.BitBlt(
						0,
						0,
						imageData->dwImageWidth, 
						imageData->dwImageHeight,
						dcTemplate,
						0,
						0,
						SRCCOPY);
			break;
		}

		case sizeStyleTile : {

			// we're tiling the image, starting at coordinates (0, 0)
			DWORD dwX = 0;
			DWORD dwY = 0;
			
			DWORD dwImageOffsetX = 0;
			DWORD dwImageOffsetY = 0;
//			DWORD dwImageOffsetY = imageData->dwImageHeight + (nBackgroundOffsetY - (int)imageData->dwImageHeight*(nBackgroundOffsetY/(int)imageData->dwImageHeight));
			
			while (dwY < imageData->dwImageHeight) {
				
				dwX				= 0;
				dwImageOffsetX	= 0;
//				dwImageOffsetX	= imageData->dwImageWidth + (nBackgroundOffsetX - (int)imageData->dwImageWidth*(nBackgroundOffsetX/(int)imageData->dwImageWidth));
				
				while (dwX < imageData->dwImageWidth) {

					imageData->dcImage.BitBlt(
								dwX, 
								dwY, 
								imageData->originalImage.getWidth(), 
								imageData->originalImage.getHeight(),
								dcTemplate,
								dwImageOffsetX,
								dwImageOffsetY,
								SRCCOPY);
					
					dwX += imageData->originalImage.getWidth() - dwImageOffsetX;
					dwImageOffsetX = 0;
				}
				
				dwY += imageData->originalImage.getHeight() - dwImageOffsetY;
				dwImageOffsetY = 0;
			}
			break;
		}
	}









//	fipImage tempImage(imageData->originalImage);

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
/*
	imageData->dwImageWidth	= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	imageData->dwImageHeight= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
*/
			
/*
	// get offsets for virtual display
	m_nBackgroundOffsetX = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_nBackgroundOffsetY = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
*/


/*
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
*/
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateImage(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData) {

	if ((imageData->dwImageWidth == static_cast<DWORD>(clientRect.Width())) &&
		(imageData->dwImageHeight == static_cast<DWORD>(clientRect.Height()))) {
		// no client size change, nothing to do
		return;
	}

	CDC		dcTemplate;
	CBitmap	bmpTemplate;

	dcTemplate.CreateCompatibleDC(dc);

	imageData->dwImageWidth = clientRect.Width();
	imageData->dwImageHeight= clientRect.Height();

	if ((imageData->sizeStyle == sizeStyleResize) &&
		((imageData->originalImage.getWidth() != imageData->dwImageWidth) || (imageData->originalImage.getHeight() != imageData->dwImageHeight))) {

		// resize background image
		fipImage tempImage(imageData->originalImage);
		tempImage.rescale(static_cast<WORD>(imageData->dwImageWidth), static_cast<WORD>(imageData->dwImageHeight), FILTER_BILINEAR);

		bmpTemplate.CreateDIBitmap(
						dc, 
						tempImage.getInfoHeader(), 
						CBM_INIT, 
						tempImage.accessPixels(), 
						tempImage.getInfo(), 
						DIB_RGB_COLORS);
	} else {
		bmpTemplate.CreateDIBitmap(
						dc, 
						imageData->originalImage.getInfoHeader(), 
						CBM_INIT, 
						imageData->originalImage.accessPixels(), 
						imageData->originalImage.getInfo(), 
						DIB_RGB_COLORS);
	}

	dcTemplate.SelectBitmap(bmpTemplate);

	if (!imageData->image.IsNull()) imageData->image.DeleteObject();
	imageData->image.CreateCompatibleBitmap(dc, imageData->dwImageWidth, imageData->dwImageHeight);
	imageData->dcImage.SelectBitmap(imageData->image);

	CBrush backgroundBrush(::CreateSolidBrush(imageData->crBackground));
	imageData->dcImage.FillRect(&clientRect, backgroundBrush);

	switch (imageData->sizeStyle) {

		case sizeStyleCenter : {

			imageData->dcImage.BitBlt(
						(imageData->dwImageWidth <= imageData->originalImage.getWidth()) ? 0 : (imageData->dwImageWidth - imageData->originalImage.getWidth())/2,
						(imageData->dwImageHeight <= imageData->originalImage.getHeight()) ? 0 : (imageData->dwImageHeight - imageData->originalImage.getHeight())/2,
						imageData->originalImage.getWidth(), 
						imageData->originalImage.getHeight(),
						dcTemplate,
						(imageData->dwImageWidth < imageData->originalImage.getWidth()) ? (imageData->originalImage.getWidth() - imageData->dwImageWidth)/2 : 0,
						(imageData->dwImageHeight < imageData->originalImage.getHeight()) ? (imageData->originalImage.getHeight() - imageData->dwImageHeight)/2 : 0,
						SRCCOPY);
			break;
		}

		case sizeStyleResize : {

			imageData->dcImage.BitBlt(
						0,
						0,
						imageData->dwImageWidth, 
						imageData->dwImageHeight,
						dcTemplate,
						0,
						0,
						SRCCOPY);
			break;
		}

		case sizeStyleTile : {

			// we're tiling the image, starting at coordinates (0, 0)
			DWORD dwX = 0;
			DWORD dwY = 0;
			
			DWORD dwImageOffsetX = 0;
			DWORD dwImageOffsetY = 0;
//			DWORD dwImageOffsetY = imageData.dwImageHeight + (m_nBackgroundOffsetY - (int)imageData.dwImageHeight*(m_nBackgroundOffsetY/(int)imageData.dwImageHeight));
			
			while (dwY < imageData->dwImageHeight) {
				
				dwX				= 0;
				dwImageOffsetX	= 0;
//				dwImageOffsetX	= imageData.dwImageWidth + (m_nBackgroundOffsetX - (int)imageData.dwImageWidth*(m_nBackgroundOffsetX/(int)imageData.dwImageWidth));
				
				while (dwX < imageData->dwImageWidth) {

					imageData->dcImage.BitBlt(
								dwX, 
								dwY, 
								imageData->originalImage.getWidth(), 
								imageData->originalImage.getHeight(),
								dcTemplate,
								dwImageOffsetX,
								dwImageOffsetY,
								SRCCOPY);
					
					dwX += imageData->originalImage.getWidth() - dwImageOffsetX;
					dwImageOffsetX = 0;
				}
				
				dwY += imageData->originalImage.getHeight() - dwImageOffsetY;
				dwImageOffsetY = 0;
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

