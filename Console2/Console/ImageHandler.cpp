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

void ImageHandler::LoadImage(shared_ptr<ImageData>& imageData) {

	USES_CONVERSION;

	if (imageData->originalImage.isValid()) return;

	// load background image
	if (!imageData->originalImage.load(W2A(imageData->strFilename.c_str()))) return;

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

	if (imageData->bRelative) {

		// for relative background, background size is equal to virtual screen size
		imageData->dwImageWidth	= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		imageData->dwImageHeight= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
			
/*
			// get offsets for virtual display
			m_nBackgroundOffsetX = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
			m_nBackgroundOffsetY = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
*/
	}

	m_images.push_back(imageData);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::UpdateImageBitmap(const CDC& dc, const CRect& clientRect, shared_ptr<ImageData>& imageData) {

	DWORD dwClientWidth = clientRect.right - clientRect.left;
	DWORD dwClientHeight= clientRect.bottom - clientRect.top;

	if (!imageData->bRelative && 
		(imageData->dwImageWidth == dwClientWidth) &&
		(imageData->dwImageHeight == dwClientHeight)) {

		return;
	}

	if (imageData->bRelative) {

		if (!imageData->image.IsNull()) return;

		fipImage tempImage(imageData->originalImage);

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

		return;

	}

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


/*
	imageData->dcImage.BitBlt(
					0,
					0,
					dwClientWidth, 
					dwClientHeight,
					tempImage.hdcImage,
					0,
					0,
					SRCCOPY);
*/
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::AdjustBackgroundImage(shared_ptr<ImageData>& imageData, DWORD dwWidth, DWORD dwHeight) {

}

//////////////////////////////////////////////////////////////////////////////
