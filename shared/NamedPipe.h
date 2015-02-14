#pragma once

//////////////////////////////////////////////////////////////////////////////

class NamedPipe
{
	public:

		NamedPipe()
			: m_strName(L"")
			, m_hNamedPipe()
			, m_hEvent()
			, m_bConnected(false)
		{
		}

		~NamedPipe()
		{
		}

		void Create(const std::wstring& strName, const std::wstring& strUser)
		{
			m_strName = strName;

			std::shared_ptr<SECURITY_ATTRIBUTES> sa;
			std::shared_ptr<void>                creatorSID; // PSID
			std::shared_ptr<ACL>                 acl;
			std::shared_ptr<void>                sd;         // PSECURITY_DESCRIPTOR

			if (strUser.length() > 0)
			{
				EXPLICIT_ACCESS          ea[2];
				SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;

				::ZeroMemory(&ea, 2*sizeof(EXPLICIT_ACCESS));

				// initialize an EXPLICIT_ACCESS structure for an ACE
				// the ACE will allow Everyone full access
				ea[0].grfAccessPermissions = GENERIC_ALL;
				ea[0].grfAccessMode        = SET_ACCESS;
				ea[0].grfInheritance       = NO_INHERITANCE;
				ea[0].Trustee.TrusteeForm  = TRUSTEE_IS_NAME;
				ea[0].Trustee.TrusteeType  = TRUSTEE_IS_USER;
				ea[0].Trustee.ptstrName    = (LPTSTR)strUser.c_str();

				{
					// create a SID for the BUILTIN\Administrators group
					PSID tmpSID = NULL;
					if (!::AllocateAndInitializeSid(
								&SIDAuthCreator,
								1,
								SECURITY_CREATOR_OWNER_RID,
								0, 0, 0, 0, 0, 0, 0,
								&tmpSID)) 
					{
						Win32Exception::ThrowFromLastError("AllocateAndInitializeSid");
					}

					creatorSID.reset(tmpSID, ::FreeSid);
				}

				// initialize an EXPLICIT_ACCESS structure for an ACE
				// the ACE will allow the Administrators group full access
				ea[1].grfAccessPermissions = GENERIC_ALL;
				ea[1].grfAccessMode        = SET_ACCESS;
				ea[1].grfInheritance       = NO_INHERITANCE;
				ea[1].Trustee.TrusteeForm  = TRUSTEE_IS_SID;
				ea[1].Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
				ea[1].Trustee.ptstrName    = (LPTSTR)creatorSID.get();

				{
					PACL tmpACL = NULL;
					if (::SetEntriesInAcl(2, ea, NULL, &tmpACL) != ERROR_SUCCESS) 
					{
						Win32Exception::ThrowFromLastError("SetEntriesInAcl");
					}

					acl.reset(tmpACL, ::LocalFree);
				}

				// initialize a security descriptor
				sd.reset(::LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH), ::LocalFree);
				if (!sd)
				{
					Win32Exception::ThrowFromLastError("LocalAlloc");
				}
	 
				if (!::InitializeSecurityDescriptor(sd.get(), SECURITY_DESCRIPTOR_REVISION)) 
				{
					Win32Exception::ThrowFromLastError("InitializeSecurityDescriptor");
				}

				// add the ACL to the security descriptor
				if (!::SetSecurityDescriptorDacl(
						sd.get(),
						TRUE,      // bDaclPresent flag
						acl.get(),
						FALSE))    // not a default DACL
				{
					Win32Exception::ThrowFromLastError("SetSecurityDescriptorDacl");
				}

				// initialize a security attributes structure
				sa.reset(new SECURITY_ATTRIBUTES);
				sa->nLength              = sizeof (SECURITY_ATTRIBUTES);
				sa->lpSecurityDescriptor = sd.get();
				sa->bInheritHandle       = FALSE;
			}

			m_hNamedPipe.reset(
				::CreateNamedPipe(
					m_strName.c_str(),
					PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_BYTE,
					1,
					1024,
					1024,
					0,
					sa.get()));

			if( m_hNamedPipe.get() == INVALID_HANDLE_VALUE )
			{
				Win32Exception::ThrowFromLastError("CreateNamedPipe");
			}

			m_hEvent.reset(::CreateEvent(NULL, FALSE, FALSE, NULL));

			if( m_hEvent.get() == NULL )
			{
				Win32Exception::ThrowFromLastError("CreateEvent");
			}

			m_overlap.hEvent = m_hEvent.get();

			// starts connection to the client
			if( !::ConnectNamedPipe(m_hNamedPipe.get(), &m_overlap) )
			{
				if( ::GetLastError() != ERROR_IO_PENDING )
					Win32Exception::ThrowFromLastError("ConnectNamedPipe");
			}
			else
			{
				m_bConnected = true;
			}
		}

		void Open(const std::wstring& strName)
		{
			m_strName = strName;

			m_hNamedPipe.reset(
				::CreateFile(
					m_strName.c_str(),
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL));

			if( m_hNamedPipe.get() == INVALID_HANDLE_VALUE )
			{
				Win32Exception::ThrowFromLastError("CreateFile");
			}

			m_hEvent.reset(::CreateEvent(NULL, FALSE, FALSE, NULL));

			if( m_hEvent.get() == NULL )
			{
				Win32Exception::ThrowFromLastError("CreateEvent");
			}

			m_overlap.hEvent = m_hEvent.get();
		}

		inline HANDLE Get() { return m_hEvent.get(); }
		void WaitConnect()
		{
			if( m_bConnected ) return;

			switch( ::WaitForSingleObject(m_hEvent.get(), 10000) )
			{
			case WAIT_OBJECT_0:
				EndAsync();
				m_bConnected = true;
				break;

			case WAIT_TIMEOUT:
				throw std::exception("timeout (NamedPipe::WaitConnect)");
				break;

			case WAIT_FAILED:
				Win32Exception::ThrowFromLastError("WaitForSingleObject");
			}
		}

		void Write(const void * data, size_t size)
		{
			DWORD dwNumberOfBytesWritten;

			if( !::WriteFile(
				m_hNamedPipe.get(),
				data,
				static_cast<DWORD>(size),
				&dwNumberOfBytesWritten,
				NULL) )
			{
				Win32Exception::ThrowFromLastError("WriteFile");
			}
		}

		size_t BeginReadAsync(void * buffer, size_t size)
		{
			DWORD dwNumberOfBytesRead;
			if( !::ReadFile(
				m_hNamedPipe.get(),
				buffer,
				static_cast<DWORD>(size),
				&dwNumberOfBytesRead,
				&m_overlap) )
			{
				if( ::GetLastError() != ERROR_IO_PENDING )
					Win32Exception::ThrowFromLastError("ReadFile");
			}
			else
			{
				return static_cast<size_t>(dwNumberOfBytesRead);
			}

			return 0;
		}

		size_t EndAsync()
		{
			DWORD dwNumberOfBytesTransferred;
			if( !::GetOverlappedResult(
				m_hNamedPipe.get(),
				&m_overlap,
				&dwNumberOfBytesTransferred,
				FALSE) )
			{
				Win32Exception::ThrowFromLastError("GetOverlappedResult");
			}

			return static_cast<size_t>(dwNumberOfBytesTransferred);
		}

	private:

		std::wstring m_strName;
		std::unique_ptr<void, CloseHandleHelper> m_hNamedPipe;

		OVERLAPPED m_overlap;
		std::unique_ptr<void, CloseHandleHelper> m_hEvent;
		bool m_bConnected;
};

//////////////////////////////////////////////////////////////////////////////
