#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "DebugPacketReceiver.h"

class DebugPacketParser : public DebugPacketReceiver
{
private:
	void *DequeueMessagePart(unsigned char **pBody, unsigned *pLeftSize, unsigned *pPartLength = NULL, bool *pIndirect = NULL)
	{
		signed short Size = *((signed short *)*pBody);
		if (Size < 0)
		{
			if (pIndirect)
				*pIndirect = true;
			Size = -Size;
		}
		else if (pIndirect)
			*pIndirect = false;
		if ((unsigned)Size > *pLeftSize)
			return NULL;
		void *pRet = (*pBody) + 2;
		*pLeftSize -= (Size + 2);
		*pBody += (Size + 2);
		if (pPartLength)
			*pPartLength = Size;
		return pRet;
	}

	void OnPrintfPacket(unsigned char *pBody, unsigned bodySize)
	{
		char *pFmt = (char *)DequeueMessagePart(&pBody, &bodySize);
		if (!pFmt)
		{
			printf("(invalid wos_debug_printf() packet received)\n");
			return;
		}
		char *pArgs = new char[bodySize];
		char *pIndirectArgs = new char[bodySize];
		unsigned originalBodySize = bodySize;
		memset(pArgs, 0, bodySize);
		size_t usedNow = 0, usedIndirect = 0;
		unsigned len = 0;
		void *pData;
		bool indirect = false;
		while (pData = DequeueMessagePart(&pBody, &bodySize, &len, &indirect))
		{
			if ((len == 1) && (((char *)pData)[0] == 1))
			{
				vprintf(pFmt, (va_list)pArgs);
				delete pArgs;
				delete pIndirectArgs;
				return;
			}
			if (indirect)
			{
				memcpy(pIndirectArgs + usedIndirect, pData, len);
				*((char **)(pArgs + usedNow)) = pIndirectArgs + usedIndirect;
				usedIndirect += len;
				usedNow += sizeof(char *);
			}
			else
			{
				int aligned = ((len + 3) / 4) * 4;
				if ((usedNow + aligned) >= originalBodySize)
					break;
				memcpy(pArgs + usedNow, pData, len);
				usedNow += aligned;
			}
		}
		printf("%s", pFmt);
		delete pArgs;
		delete pIndirectArgs;
		return;
	}
	
protected:

	virtual void OnPacketReceived(WOS::Debug::DebugPacketHeader *pHeader, unsigned char *pBody, unsigned bodySize)
	{
		switch (pHeader->PacketType)
		{
		case WOS::Debug::DebugLogNormal:
			fwrite(pBody, bodySize, 1, stdout);
			break;
		case WOS::Debug::DebugLogPrintf:
			OnPrintfPacket(pBody, bodySize);
			break;
		}
	}

	virtual void OnPacketReceived(unsigned char *pData, unsigned dataSize)
	{
		WOS::Debug::DebugPacketHeader *pHdr = (WOS::Debug::DebugPacketHeader *)pData;
		OnPacketReceived(pHdr, pData + sizeof(*pHdr), dataSize - sizeof(*pHdr));
	}
};

int main(int argc, char* argv[])
{
	char *pszPort = 0;
	if (argc > 1)
		pszPort = argv[1];
	SerialPort port(pszPort);
	if (!port.Valid())
	{
		printf("Cannot open serial port!\n");
		return 1;
	}
	DebugPacketParser parser;
	parser.SetSerialPort(&port);
	parser.MainLoop();
}

