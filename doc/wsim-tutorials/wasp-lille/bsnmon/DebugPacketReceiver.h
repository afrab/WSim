#pragma once
#include "SerialPort.h"
#include "../../lib/core/dbgstruct.h"

class DebugMicropacketReceiver
{
private:
	SerialPort *m_pPort;

protected:
	virtual void OnMicropacketReceived(WOS::Debug::MicropacketHeader *pHdr, unsigned char *pData, unsigned dataSize)=0;

	unsigned char CalculateChecksum(unsigned char *pData, unsigned size)
	{
		unsigned char s = 0;
		for (unsigned i = 0; i < size; i++)
			s += pData[i];
		return s;
	}

	//! Waits until a valid s_MicropacketSignature is transferred.
	void SynchronizePacketBeginning()
	{
		unsigned char ch;
		int idx = 0;
		for (;;)
		{
			while (!m_pPort->Read(&ch, 1));
/*			printf("%02X ", ch);
			if (ch == 0x53)
				printf("\n");*/
			if (ch == WOS::Debug::s_MicropacketSignature[idx])
			{
				if (++idx >= (sizeof(WOS::Debug::s_MicropacketSignature) - 1))
					return;
			}
			else
				idx = 0;
		}
	}

public:

	DebugMicropacketReceiver()
	{
		m_pPort = NULL;
	}

	void SetSerialPort(SerialPort *pPort)
	{
		m_pPort = pPort;
		if (!pPort)
			return;

		//Control the test/reset pins. See BSL.PY for more details.
		pPort->SetModemFlags(SerialPort::DTR);

		pPort->SetModemFlags(SerialPort::RTS);
		pPort->ClearModemFlags(SerialPort::RTS);

		pPort->ClearModemFlags(SerialPort::DTR);

		pPort->SetModemFlags(SerialPort::RTS);
		pPort->ClearModemFlags(SerialPort::RTS);

		pPort->SetTimeout(2000);
	}

	void MainLoop()
	{
		WOS::Debug::MicropacketHeader hdr;
		unsigned char MicropacketData[256];
		for (;;)
		{
			SynchronizePacketBeginning();
			if (m_pPort->Read(&hdr, sizeof(hdr)) != sizeof(hdr))
				continue;
			if (m_pPort->Read(MicropacketData, hdr.MicropacketBodySize) != hdr.MicropacketBodySize)
				continue;
			unsigned char checksum = 0;
			if (m_pPort->Read(&checksum, 1) != 1)
				continue;
			unsigned char refChecksum = CalculateChecksum(MicropacketData, hdr.MicropacketBodySize);
			if (checksum != refChecksum)
				continue;
			OnMicropacketReceived(&hdr, MicropacketData, hdr.MicropacketBodySize);
		}
	}
};

class DebugPacketReceiver : public DebugMicropacketReceiver
{
private:
	unsigned m_Allocated;
	unsigned m_AlreadyReceived;
	unsigned char *m_pBuffer;

public:

	DebugPacketReceiver()
	{
		m_Allocated = 1024;
		m_AlreadyReceived = 0;
		m_pBuffer = (unsigned char *)malloc(m_Allocated);
	}

	~DebugPacketReceiver()
	{
		if (m_pBuffer)
			free(m_pBuffer);
	}

protected:
	virtual void OnPacketReceived(unsigned char *pData, unsigned dataSize)=0;

	virtual void OnMicropacketReceived(WOS::Debug::MicropacketHeader *pHdr, unsigned char *pData, unsigned dataSize)
	{
		switch (pHdr->MicropacketType)
		{
		case WOS::Debug::MicropacketTypeStartPacket:
		case WOS::Debug::MicropacketTypeStartPacket | WOS::Debug::MicropacketTypeFinalizePacketFlag:
			m_AlreadyReceived = 0;
		case WOS::Debug::MicropacketTypeContinuePacket:
		case WOS::Debug::MicropacketTypeContinuePacket | WOS::Debug::MicropacketTypeFinalizePacketFlag:			
			break;
		default:
			return;
		}

		unsigned newSize = m_AlreadyReceived + dataSize;
		if (newSize >= m_Allocated)
		{
			m_Allocated = (m_Allocated * 2) + newSize;
			m_pBuffer = (unsigned char *)realloc(m_pBuffer, m_Allocated);
		}
		memcpy(m_pBuffer + m_AlreadyReceived, pData, dataSize);
		m_AlreadyReceived += dataSize;
		if (pHdr->MicropacketType & WOS::Debug::MicropacketTypeFinalizePacketFlag)
		{
			OnPacketReceived(m_pBuffer, m_AlreadyReceived);
			m_AlreadyReceived = 0;
		}
	}
};
