#pragma once

#include <msxml.h>

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class XmlHelper
{
	public:
		
		static HRESULT OpenXmlDocument(const wstring& strFilename, CComPtr<IXMLDOMDocument>& pXmlDocument, CComPtr<IXMLDOMElement>& pRootElement);

		static HRESULT GetDomElement(const CComPtr<IXMLDOMElement>& pRootElement, const CComBSTR& bstrPath, CComPtr<IXMLDOMElement>& pElement);

		static void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD& dwValue, DWORD dwDefaultValue);
		static void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, int& nValue, int nDefaultValue);
		static void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE& byValue, BYTE byDefaultValue);
		static void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool& bValue, bool bDefaultValue);
		static void GetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, wstring& strValue, const wstring& strDefaultValue);

		static void GetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue, COLORREF crDefaultValue);

		static void SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, DWORD dwValue);
		static void SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, int nValue);
		static void SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, BYTE byValue);
		static void SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, bool bValue);
		static void SetAttribute(const CComPtr<IXMLDOMElement>& pElement, const CComBSTR& bstrName, const wstring& strValue);

		static void SetRGBAttribute(const CComPtr<IXMLDOMElement>& pElement, COLORREF& crValue);
};

//////////////////////////////////////////////////////////////////////////////
