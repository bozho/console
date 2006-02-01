#include "stdafx.h"
#include "XmlHelper.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HRESULT XmlHelper::OpenXmlDocument(const wstring& strFilename, const wstring& strDefaultFilename, CComPtr<IXMLDOMDocument>& pXmlDocument, CComPtr<IXMLDOMElement>& pRootElement) {

	HRESULT hr					= S_OK;
	VARIANT_BOOL bLoadSuccess	= false;
	
	hr = pXmlDocument.CoCreateInstance(__uuidof(DOMDocument));
	if (FAILED(hr) || (pXmlDocument.p == NULL)) return false;

	if (strFilename.length() > 0) {
		hr = pXmlDocument->load(CComVariant(strFilename.c_str()), &bLoadSuccess);
	}

	if (FAILED(hr) || (!bLoadSuccess)) {

		if (strDefaultFilename.length() == 0) return E_FAIL;

		wchar_t szModuleFileName[MAX_PATH + 1];
		::ZeroMemory(szModuleFileName, (MAX_PATH+1)*sizeof(wchar_t));
		::GetModuleFileName(NULL, szModuleFileName, MAX_PATH);

		wstring strModuleFileName(szModuleFileName);
		wstring strDefaultOptionsFileName(strModuleFileName.substr(0, strModuleFileName.rfind(L'\\')+1));

		strDefaultOptionsFileName += strDefaultFilename;

		hr = pXmlDocument->load(CComVariant(strDefaultOptionsFileName.c_str()), &bLoadSuccess);
		if (FAILED(hr) || (!bLoadSuccess)) return E_FAIL;
	}

	hr = pXmlDocument->get_documentElement(&pRootElement);
	if (FAILED(hr)) return E_FAIL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

HRESULT XmlHelper::GetDomElement(const CComPtr<IXMLDOMElement>& pRootElement, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement) {

	HRESULT					hr = S_OK;
	CComPtr<IXMLDOMNode>	pNode;
	
	if (pRootElement.p == NULL) return E_FAIL;

	hr = pRootElement->selectSingleNode(bstrPath, &pNode);
	if (hr != S_OK) return E_FAIL;

	return pNode.QueryInterface(&pElement);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		dwValue = _wtol(varValue.bstrVal);
	} else {
		dwValue = dwDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE& byValue, BYTE byDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		byValue = static_cast<BYTE>(_wtoi(varValue.bstrVal));
	} else {
		byValue = byDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		bValue = (_wtol(varValue.bstrVal) > 0);
	} else {
		bValue = bDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue) {

	CComVariant	varValue;

	if (pElement->getAttribute(bstrName, &varValue) == S_OK) {
		strValue = varValue.bstrVal;
	} else {
		strValue = strDefaultValue;
	}
}

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////

void XmlHelper::GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue) {

	DWORD r;
	DWORD g;
	DWORD b;

	GetAttribute(pElement, CComBSTR(L"r"), r, GetRValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"g"), g, GetGValue(crDefaultValue));
	GetAttribute(pElement, CComBSTR(L"b"), b, GetBValue(crDefaultValue));

	crValue = RGB(r, g, b);
}

//////////////////////////////////////////////////////////////////////////////

