// code found in http://win7shell.googlecode.com/svn/trunk/jumplist.cpp

#include "StdAfx.h"
#include "JumpList.h"

#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>

// Creates a CLSID_ShellLink to insert into the Tasks section of the Jump List.  This type of Jump
// List item allows the specification of an explicit command line to execute the task.
static HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl)
{
	IShellLink *psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
	if (SUCCEEDED(hr))
	{
		wchar_t szModulePath[MAX_PATH];
		::ZeroMemory(szModulePath, sizeof(szModulePath));
		::GetModuleFileName(NULL, szModulePath, MAX_PATH);
		hr = psl->SetPath(szModulePath);
		if (SUCCEEDED(hr))
		{
			hr = psl->SetArguments(pszArguments);
			if (SUCCEEDED(hr))
			{
				// The title property is required on Jump List items provided as an IShellLink
				// instance.  This value is used as the display name in the Jump List.
				IPropertyStore *pps;
				hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
				if (SUCCEEDED(hr))
				{
					PROPVARIANT propvar;
					hr = InitPropVariantFromString(pszTitle, &propvar);
					if (SUCCEEDED(hr))
					{
						hr = pps->SetValue(PKEY_Title, propvar);
						if (SUCCEEDED(hr))
						{
							hr = pps->Commit();
							if (SUCCEEDED(hr))
							{
								hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
							}
						}

						PropVariantClear(&propvar);
					}

					pps->Release();
				}
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		psl->Release();
	}

	return hr;
}

static void SetIcon(IShellLink *psl, shared_ptr<TabData> tab)
{
	if (tab->bUseDefaultIcon)
	{
		wchar_t szModulePath[MAX_PATH];
		::ZeroMemory(szModulePath, sizeof(szModulePath));
		::GetModuleFileName(NULL, szModulePath, MAX_PATH);

		psl->SetIconLocation(szModulePath, 0);
	}
	else
		if (tab->strIcon.length() > 0)
			psl->SetIconLocation(Helpers::ExpandEnvironmentStrings(tab->strIcon).c_str(), 0);
}

void JumpList::CreateList(TabDataVector& tabDataVector)
{
	TabDataVector::iterator	it = tabDataVector.begin();

	ICustomDestinationList *pcdl = 0;
	HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
	if (FAILED(hr))
		return;

	UINT cMinSlots;
	IObjectCollection *poc = 0;
	IObjectArray *poaRemoved = 0;
	if (FAILED(pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved))))
		goto bail;

	if (FAILED(CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc))))
		goto bail;

	for (it; it != tabDataVector.end(); ++it)
	{
		IShellLink * psl;
		wstring quotedName(L"-reuse -t \"");
		quotedName.append((*it)->strTitle);
		quotedName.append(L"\"");
		if (SUCCEEDED(_CreateShellLink(quotedName.c_str(), (*it)->strTitle.c_str(), &psl)))
		{
			SetIcon(psl, *it);
			poc->AddObject(psl);
			psl->Release();
		}
	}

	IObjectArray * poa;
	hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
	if (SUCCEEDED(hr))
	{
		// Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
		// category that is displayed at the bottom of the Jump List, after all other
		// categories.
		hr = pcdl->AddUserTasks(poa);
		poa->Release();
	}

	pcdl->CommitList();

bail:
	if (poc)
		poc->Release();

	if (poaRemoved)
		poaRemoved->Release();

	if (pcdl)
		pcdl->Release();
}
