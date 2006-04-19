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

shared_ptr<BackgroundImage> ImageHandler::GetImage(const ImageData& imageData)
{
	shared_ptr<BackgroundImage>	bkImage(new BackgroundImage(imageData));

	Images::iterator		itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if ((*itImage)->imageData == bkImage->imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	// else, try to load image
	if (!LoadImage(bkImage)) return shared_ptr<BackgroundImage>();

	m_images.push_back(bkImage);

	return bkImage;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

shared_ptr<BackgroundImage> ImageHandler::GetDesktopImage(ImageData& imageData)
{
	CRegKey					keyColors;

	if (keyColors.Open(HKEY_CURRENT_USER, L"Control Panel\\Colors", KEY_READ) != ERROR_SUCCESS) return shared_ptr<BackgroundImage>();

	CString	strBackground;
	DWORD	dwDataSize = 32;

	if (keyColors.QueryStringValue(L"Background", strBackground.GetBuffer(dwDataSize), &dwDataSize) != ERROR_SUCCESS)
	{
		strBackground.ReleaseBuffer();
		return shared_ptr<BackgroundImage>();
	}
	strBackground.ReleaseBuffer();

	wstringstream	streamBk(wstring(strBackground.operator LPCTSTR()));
	DWORD			r = 0;
	DWORD			g = 0;
	DWORD			b = 0;

	streamBk >> r;
	streamBk >> g;
	streamBk >> b;

	CRegKey keyDesktop;
	if (keyDesktop.Open(HKEY_CURRENT_USER, L"Control Panel\\Desktop", KEY_READ) != ERROR_SUCCESS) return shared_ptr<BackgroundImage>();

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

	// set background data
	imageData.strFilename	= strWallpaperFile;
	imageData.bRelative	= true;
	imageData.bExtend		= false;
	imageData.crBackground	= RGB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));

	if (strWallpaperTile == L"1")
	{
		imageData.imagePosition= imgPosTile;
	}
	else
	{
		if (strWallpaperStyle == L"0")
		{
			imageData.imagePosition= imgPosCenter;
		}
		else
		{
			imageData.imagePosition= imgPosFit;
		}
	}

	shared_ptr<BackgroundImage>	bkImage(new BackgroundImage(imageData));

	// now, find the image
	Images::iterator itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if ((*itImage)->imageData == bkImage->imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	// else, try to load image
	if (!LoadImage(bkImage)) return shared_ptr<BackgroundImage>();

	m_images.push_back(bkImage);

	return bkImage;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<BackgroundImage>& bkImage)
{
	if (bkImage->imageData.bRelative)
	{
		if (!bkImage->image.IsNull()) return;
		// first access to relative image, create it
		CreateRelativeImage(dc, bkImage);
	}
	else
	{
		CreateImage(dc, clientRect, bkImage);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::LoadImage(shared_ptr<BackgroundImage>& bkImage)
{
	USES_CONVERSION;

	if (bkImage.get() == NULL) return false;

	// load background image
	if (!bkImage->originalImage.load(W2A(bkImage->imageData.strFilename.c_str()))) return false;

	bkImage->dwOriginalImageWidth	= bkImage->originalImage.getWidth();
	bkImage->dwOriginalImageHeight	= bkImage->originalImage.getHeight();

	bkImage->originalImage.convertTo24Bits();
	
	// ... if needed, tint the background image
	if (bkImage->imageData.byTintOpacity > 0)
	{
		BYTE*	pPixels = bkImage->originalImage.accessPixels();
		BYTE*	pPixelsEnd =  pPixels + 3*bkImage->originalImage.getWidth()*bkImage->originalImage.getHeight();
		BYTE*	pPixelSubel = pPixels;

		while (pPixelSubel < pPixelsEnd)
		{
			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - bkImage->imageData.byTintOpacity) + GetBValue(bkImage->imageData.crTint)*bkImage->imageData.byTintOpacity)/100); 
			++pPixelSubel;
			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - bkImage->imageData.byTintOpacity) + GetGValue(bkImage->imageData.crTint)*bkImage->imageData.byTintOpacity)/100);
			++pPixelSubel;
			*pPixelSubel = (BYTE) ((unsigned long)(*pPixelSubel * (100 - bkImage->imageData.byTintOpacity) + GetRValue(bkImage->imageData.crTint)*bkImage->imageData.byTintOpacity)/100);
			++pPixelSubel;
		}
	}

	// create background CDC and paint bitmap
	bkImage->dcImage.CreateCompatibleDC(NULL);

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateRelativeImage(const CDC& dc, shared_ptr<BackgroundImage>& bkImage)
{
	DWORD	dwPrimaryDisplayWidth	= ::GetSystemMetrics(SM_CXSCREEN);
	DWORD	dwPrimaryDisplayHeight	= ::GetSystemMetrics(SM_CYSCREEN);

	bkImage->dwImageWidth		= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	bkImage->dwImageHeight	= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

	CDC		dcTemplate;
	CBitmap	bmpTemplate;

	dcTemplate.CreateCompatibleDC(dc);

	// set template bitmap dimensions
	DWORD	dwTemplateWidth		= bkImage->originalImage.getWidth();
	DWORD	dwTemplateHeight	= bkImage->originalImage.getHeight();

	if (bkImage->imageData.imagePosition == imgPosFit)
	{
		if (bkImage->imageData.bExtend)
		{
			dwTemplateWidth	= bkImage->dwImageWidth;
			dwTemplateHeight= bkImage->dwImageHeight;
		}
		else
		{
			dwTemplateWidth	= dwPrimaryDisplayWidth;
			dwTemplateHeight= dwPrimaryDisplayHeight;
		}
	}

	if ((bkImage->originalImage.getWidth() != dwTemplateWidth) || 
		(bkImage->originalImage.getHeight() != dwTemplateHeight))
	{
		// resize background image
		fipImage tempImage(bkImage->originalImage);
		tempImage.rescale(static_cast<WORD>(dwTemplateWidth), static_cast<WORD>(dwTemplateHeight), FILTER_BILINEAR);

		bmpTemplate.CreateDIBitmap(
						dc, 
						tempImage.getInfoHeader(), 
						CBM_INIT, 
						tempImage.accessPixels(), 
						tempImage.getInfo(), 
						DIB_RGB_COLORS);
	}
	else
	{
		bmpTemplate.CreateDIBitmap(
						dc, 
						bkImage->originalImage.getInfoHeader(), 
						CBM_INIT, 
						bkImage->originalImage.accessPixels(), 
						bkImage->originalImage.getInfo(), 
						DIB_RGB_COLORS);
	}

	dcTemplate.SelectBitmap(bmpTemplate);

	bkImage->image.CreateCompatibleBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight);
	bkImage->dcImage.SelectBitmap(bkImage->image);

	CBrush	backgroundBrush(::CreateSolidBrush(bkImage->imageData.crBackground));
	CRect	rect(0, 0, bkImage->dwImageWidth, bkImage->dwImageHeight);
	bkImage->dcImage.FillRect(&rect, backgroundBrush);

	if (bkImage->imageData.imagePosition == imgPosTile)
	{
		TileTemplateImage(
			dcTemplate, 
			::GetSystemMetrics(SM_XVIRTUALSCREEN), 
			::GetSystemMetrics(SM_YVIRTUALSCREEN), 
			bkImage);

	}
	else
	{
		if (bkImage->imageData.bExtend)
		{
			PaintTemplateImage(
				dcTemplate, 
				0, 
				0,
				bkImage->dwImageWidth, 
				bkImage->dwImageHeight, 
				bkImage->dwImageWidth, 
				bkImage->dwImageHeight, 
				bkImage);
		}
		else
		{
			MonitorEnumData	enumData(dcTemplate, bkImage);
			::EnumDisplayMonitors(NULL, NULL, ImageHandler::MonitorEnumProc, reinterpret_cast<LPARAM>(&enumData));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateImage(const CDC& dc, const CRect& clientRect, shared_ptr<BackgroundImage>& bkImage)
{
	if ((bkImage->dwImageWidth == static_cast<DWORD>(clientRect.Width())) &&
		(bkImage->dwImageHeight == static_cast<DWORD>(clientRect.Height())))
	{
		// no client size change, nothing to do
		return;
	}

	CDC		dcTemplate;
	CBitmap	bmpTemplate;

	dcTemplate.CreateCompatibleDC(dc);

	bkImage->dwImageWidth = clientRect.Width();
	bkImage->dwImageHeight= clientRect.Height();

	if ((bkImage->imageData.imagePosition == imgPosFit) &&
		((bkImage->originalImage.getWidth() != bkImage->dwImageWidth) || (bkImage->originalImage.getHeight() != bkImage->dwImageHeight)))
	{
		// resize background image
		fipImage tempImage(bkImage->originalImage);
		tempImage.rescale(static_cast<WORD>(bkImage->dwImageWidth), static_cast<WORD>(bkImage->dwImageHeight), FILTER_BILINEAR);

		bmpTemplate.CreateDIBitmap(
						dc, 
						tempImage.getInfoHeader(), 
						CBM_INIT, 
						tempImage.accessPixels(), 
						tempImage.getInfo(), 
						DIB_RGB_COLORS);
	}
	else
	{
		bmpTemplate.CreateDIBitmap(
						dc, 
						bkImage->originalImage.getInfoHeader(), 
						CBM_INIT, 
						bkImage->originalImage.accessPixels(), 
						bkImage->originalImage.getInfo(), 
						DIB_RGB_COLORS);
	}

	dcTemplate.SelectBitmap(bmpTemplate);

	if (!bkImage->image.IsNull()) bkImage->image.DeleteObject();
	bkImage->image.CreateCompatibleBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight);
	bkImage->dcImage.SelectBitmap(bkImage->image);

	CBrush backgroundBrush(::CreateSolidBrush(bkImage->imageData.crBackground));
	bkImage->dcImage.FillRect(&clientRect, backgroundBrush);

	if (bkImage->imageData.imagePosition == imgPosTile)
	{
		TileTemplateImage(
			dcTemplate, 
			0, 
			0, 
			bkImage);

	}
	else
	{
		PaintTemplateImage(
			dcTemplate, 
			0, 
			0,
			bkImage->dwImageWidth, 
			bkImage->dwImageHeight, 
			bkImage->dwImageWidth, 
			bkImage->dwImageHeight, 
			bkImage);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, shared_ptr<BackgroundImage>& bkImage)
{
	if (bkImage->imageData.imagePosition == imgPosCenter)
	{
		bkImage->dcImage.BitBlt(
					(dwDstWidth <= bkImage->dwOriginalImageWidth) ? nOffsetX : nOffsetX + (dwDstWidth - bkImage->dwOriginalImageWidth)/2,
					(dwDstHeight <= bkImage->dwOriginalImageHeight) ? nOffsetY : nOffsetY + (dwDstHeight - bkImage->dwOriginalImageHeight)/2,
					dwDstWidth, 
					dwDstHeight,
					dcTemplate,
					(dwDstWidth < bkImage->dwOriginalImageWidth) ? (bkImage->dwOriginalImageWidth - dwDstWidth)/2 : 0,
					(dwDstHeight < bkImage->dwOriginalImageHeight) ? (bkImage->dwOriginalImageHeight - dwDstHeight)/2 : 0,
					SRCCOPY);
	}
	else
	{
		bkImage->dcImage.BitBlt(
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

void ImageHandler::TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, shared_ptr<BackgroundImage>& bkImage)
{
	// we're tiling the image, starting at coordinates (0, 0)
	DWORD dwX = 0;
	DWORD dwY = 0;

	DWORD dwImageOffsetX = 0;
	DWORD dwImageOffsetY = bkImage->originalImage.getHeight() + (nOffsetY - (int)bkImage->originalImage.getHeight()*(nOffsetY/(int)bkImage->originalImage.getHeight()));

	while (dwY < bkImage->dwImageHeight)
	{
		dwX				= 0;
		dwImageOffsetX	= bkImage->originalImage.getWidth() + (nOffsetX - (int)bkImage->originalImage.getWidth()*(nOffsetX/(int)bkImage->originalImage.getWidth()));
		
		while (dwX < bkImage->dwImageWidth)
		{
			bkImage->dcImage.BitBlt(
						dwX, 
						dwY, 
						bkImage->originalImage.getWidth(), 
						bkImage->originalImage.getHeight(),
						dcTemplate,
						dwImageOffsetX,
						dwImageOffsetY,
						SRCCOPY);
			
			dwX += bkImage->originalImage.getWidth() - dwImageOffsetX;
			dwImageOffsetX = 0;
		}
		
		dwY += bkImage->originalImage.getHeight() - dwImageOffsetY;
		dwImageOffsetY = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ImageHandler::MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData)
{
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
					pEnumData->bkImage);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
