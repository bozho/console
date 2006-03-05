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

shared_ptr<ImageData> ImageHandler::GetImageData(const wstring& strFilename, bool bRelative, bool bExtend, ImagePosition imagePosition, COLORREF crBackground, COLORREF crTint, BYTE byTintOpacity) {

	shared_ptr<ImageData>	imageData(new ImageData(
											strFilename, 
											bRelative, 
											bExtend, 
											imagePosition, 
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
							imgPosCenter, 
							RGB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b)), 
							crTint, 
							byTintOpacity));

	CRegKey keyDesktop;
	if (keyDesktop.Open(HKEY_CURRENT_USER, L"Control Panel\\Desktop", KEY_READ) != ERROR_SUCCESS) return imageData;

	DWORD	dwChars = 0;
	CString	strWallpaperFile;
	CString	strWallpaperStyle;
	CString	strWallpaperTile;

	dwChars = 32;
	keyDesktop.QueryStringValue(L"WallpaperStyle", strWallpaperStyle.GetBuffer(dwChars), &dwChars);
	dwChars = 32;
	keyDesktop.QueryStringValue(L"TileWallpaper", strWallpaperTile.GetBuffer(dwChars), &dwChars);
	dwChars = MAX_PATH;
	keyDesktop.QueryStringValue(L"Wallpaper", strWallpaperFile.GetBuffer(dwChars), &dwChars);

	strWallpaperStyle.ReleaseBuffer();
	strWallpaperFile.ReleaseBuffer();
	strWallpaperTile.ReleaseBuffer();

	imageData->strFilename = strWallpaperFile;

	if (strWallpaperTile == L"1") {
		imageData->imagePosition = imgPosTile;
	} else {
		if (strWallpaperStyle == L"0") {
			imageData->imagePosition = imgPosCenter;
		} else {
			imageData->imagePosition = imgPosFit;
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

	imageData->dwImageWidth		= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	imageData->dwImageHeight	= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

	CDC		dcTemplate;
	CBitmap	bmpTemplate;

	dcTemplate.CreateCompatibleDC(dc);

	// set template bitmap dimensions
	DWORD	dwTemplateWidth		= imageData->originalImage.getWidth();
	DWORD	dwTemplateHeight	= imageData->originalImage.getHeight();

	if (imageData->imagePosition == imgPosFit) {

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

	if (imageData->imagePosition == imgPosTile) {

		TileTemplateImage(
			dcTemplate, 
			::GetSystemMetrics(SM_XVIRTUALSCREEN), 
			::GetSystemMetrics(SM_YVIRTUALSCREEN), 
			imageData);

	} else {

		if (imageData->bExtend) {
			PaintTemplateImage(
				dcTemplate, 
				0, 
				0,
				imageData->dwImageWidth, 
				imageData->dwImageHeight, 
				imageData->dwImageWidth, 
				imageData->dwImageHeight, 
				imageData);
		} else {
			MonitorEnumData	enumData(dcTemplate, imageData);
			::EnumDisplayMonitors(NULL, NULL, ImageHandler::MonitorEnumProc, reinterpret_cast<LPARAM>(&enumData));
		}
	}
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

	if ((imageData->imagePosition == imgPosFit) &&
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

	if (imageData->imagePosition == imgPosTile) {

		TileTemplateImage(
			dcTemplate, 
			0, 
			0, 
			imageData);

	} else {
		PaintTemplateImage(
			dcTemplate, 
			0, 
			0,
			imageData->dwImageWidth, 
			imageData->dwImageHeight, 
			imageData->dwImageWidth, 
			imageData->dwImageHeight, 
			imageData);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, shared_ptr<ImageData>& imageData) {

	if (imageData->imagePosition == imgPosCenter) {

		imageData->dcImage.BitBlt(
					(dwDstWidth <= imageData->dwOriginalImageWidth) ? nOffsetX : nOffsetX + (dwDstWidth - imageData->dwOriginalImageWidth)/2,
					(dwDstHeight <= imageData->dwOriginalImageHeight) ? nOffsetY : nOffsetY + (dwDstHeight - imageData->dwOriginalImageHeight)/2,
					dwDstWidth, 
					dwDstHeight,
					dcTemplate,
					(dwDstWidth < imageData->dwOriginalImageWidth) ? (imageData->dwOriginalImageWidth - dwDstWidth)/2 : 0,
					(dwDstHeight < imageData->dwOriginalImageHeight) ? (imageData->dwOriginalImageHeight - dwDstHeight)/2 : 0,
					SRCCOPY);
	} else {

		imageData->dcImage.BitBlt(
					(dwDstWidth <= dwSrcWidth) ? nOffsetX : nOffsetX + (dwDstWidth - dwSrcWidth)/2,
					(dwDstHeight <= dwSrcHeight) ? nOffsetY : nOffsetY + (dwDstHeight - dwSrcHeight)/2,
					dwDstWidth, 
					dwDstHeight,
					dcTemplate,
					(dwDstWidth < dwSrcWidth) ? (dwSrcWidth - dwDstWidth)/2 : 0,
					(dwDstHeight < dwSrcHeight) ? (dwSrcHeight - dwDstHeight)/2 : 0,
					SRCCOPY);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, shared_ptr<ImageData>& imageData) {

	// we're tiling the image, starting at coordinates (0, 0)
	DWORD dwX = 0;
	DWORD dwY = 0;

	DWORD dwImageOffsetX = 0;
	DWORD dwImageOffsetY = imageData->originalImage.getHeight() + (nOffsetY - (int)imageData->originalImage.getHeight()*(nOffsetY/(int)imageData->originalImage.getHeight()));

	while (dwY < imageData->dwImageHeight) {
		
		dwX				= 0;
		dwImageOffsetX	= imageData->originalImage.getWidth() + (nOffsetX - (int)imageData->originalImage.getWidth()*(nOffsetX/(int)imageData->originalImage.getWidth()));
		
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
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ImageHandler::MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData) {

	MonitorEnumData* pEnumData = reinterpret_cast<MonitorEnumData*>(lpData);

	CRect	rectMonitor(lprcMonitor);
	DWORD	dwPrimaryDisplayWidth	= ::GetSystemMetrics(SM_CXSCREEN);
	DWORD	dwPrimaryDisplayHeight	= ::GetSystemMetrics(SM_CYSCREEN);


	ImageHandler::PaintTemplateImage(
					pEnumData->dcTemplate, 
					rectMonitor.left - ::GetSystemMetrics(SM_XVIRTUALSCREEN), 
					rectMonitor.top - ::GetSystemMetrics(SM_YVIRTUALSCREEN), 
					dwPrimaryDisplayWidth,
					dwPrimaryDisplayHeight,
					rectMonitor.Width(), 
					rectMonitor.Height(), 
					pEnumData->imageData);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
