#include <asm/ioctls.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <memory.h>
#include <sys/ioctl.h>

class SerialPort
{
private:
	int m_Descriptor;

protected:
	void SetBaudRate(int BaudRateModeConstant = B57600)
	{
		termios state;
		tcgetattr(m_Descriptor, &state);
		state.c_cflag = BaudRateModeConstant | CS8 | CREAD;
		state.c_iflag |= IGNPAR;
		tcsetattr(m_Descriptor, TCSANOW, &state);
	}
	
public:
	SerialPort(const char *pszDeviceName = NULL, int BaudRateConstant = B57600)
	{
		if (!pszDeviceName)
			pszDeviceName = "/dev/ttyUSB0";
		m_Descriptor = open(pszDeviceName, O_RDWR);
		if (m_Descriptor != -1)
			SetBaudRate(BaudRateConstant);
	}
	
	~SerialPort()
	{
		if (m_Descriptor != -1)
			close(m_Descriptor);
	}
	
	bool Valid()
	{
		return (m_Descriptor != -1);
	}
	
public:
	void SetTimeout(unsigned TimeoutInMiliseconds)
	{
		termios state;
		tcgetattr(m_Descriptor, &state);
		state.c_cc[VMIN] = 0;
		state.c_cc[VTIME] = TimeoutInMiliseconds / 100;	//Convert to 1/10 of sec
		tcsetattr(m_Descriptor, TCSANOW, &state);
	}
	
	enum 
	{
		RTS = TIOCM_RTS,
		DTR = TIOCM_DTR,
	};
	
	void SetModemFlags(int Flags)
	{
		ioctl(m_Descriptor, TIOCMBIS, &Flags);
	}
	
	void ClearModemFlags(int Flags)
	{
		ioctl(m_Descriptor, TIOCMBIC, &Flags);
	}
	
	size_t Read(void *pBuffer, size_t size)
	{
		return read(m_Descriptor, pBuffer, size);
	}

};
