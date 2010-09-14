#pragma once

class ConsoleException
{
	public:
		ConsoleException(const wstring& message)
		: m_message(message)
		{
		}

		const wstring& GetMessage() const { return m_message; }

	private:

		wstring m_message;
};
