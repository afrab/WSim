#pragma once

class SerialPort
{
private:
	HANDLE m_hPort;

protected:
	void SetBaudRate(int BaudRateModeConstant = 57600)
	{
		DCB dcb;
		memset(&dcb,0,sizeof(dcb));
		dcb.DCBlength=sizeof(DCB);
		GetCommState(m_hPort,&dcb);
		dcb.fBinary=TRUE;
		dcb.fParity=FALSE;
		dcb.BaudRate=BaudRateModeConstant;
		dcb.ByteSize=8;
		dcb.Parity=NOPARITY;
		dcb.StopBits=ONESTOPBIT;
		dcb.fDtrControl=DTR_CONTROL_DISABLE;
		dcb.fRtsControl=RTS_CONTROL_DISABLE;
		dcb.fOutxCtsFlow=FALSE;
		dcb.fOutxDsrFlow=FALSE;
		SetCommState(m_hPort,&dcb);
	}
	
public:

	SerialPort(const char *pszDeviceName = NULL, int BaudRateConstant = 57600)
	{
		if (!pszDeviceName)
			 pszDeviceName = "COM7";
		m_hPort = CreateFile(pszDeviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
		if (m_hPort != INVALID_HANDLE_VALUE)
			SetBaudRate(BaudRateConstant);
	}
	
	~SerialPort()
	{
		if (m_hPort != INVALID_HANDLE_VALUE)
			CloseHandle(m_hPort);
	}
	
	bool Valid()
	{
		return (m_hPort != INVALID_HANDLE_VALUE);
	}
	
public:
	void SetTimeout(unsigned TimeoutInMiliseconds)
	{
		COMMTIMEOUTS tt =
		{
			TimeoutInMiliseconds,       //DWORD ReadIntervalTimeout;
			2,							//DWORD ReadTotalTimeoutMultiplier;
			TimeoutInMiliseconds,       //DWORD ReadTotalTimeoutConstant;
			2,                          //DWORD WriteTotalTimeoutMultiplier;
			50                          //DWORD WriteTotalTimeoutConstant;
		};
		SetCommTimeouts(m_hPort,&tt);
	}
	
	enum 
	{
		RTS = 0x01,
		DTR = 0x02,
	};
	
	void SetModemFlags(int Flags)
	{
		if (Flags & RTS)
			EscapeCommFunction(m_hPort, SETRTS);
		if (Flags & DTR)
			EscapeCommFunction(m_hPort, SETDTR);
	}
	
	void ClearModemFlags(int Flags)
	{
		if (Flags & RTS)
			EscapeCommFunction(m_hPort, CLRRTS);
		if (Flags & DTR)
			EscapeCommFunction(m_hPort, CLRDTR);
	}
	
	size_t Read(void *pBuffer, size_t size)
	{
		DWORD dwOk = 0;
		ReadFile(m_hPort, pBuffer, size, &dwOk, NULL);
		int er = GetLastError();
		return dwOk;
	}

};
