#include "Window.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Win32Window.h"
#endif

#ifdef __ORBIS__
#include "../Plugins/PlayStation4/PS4Window.h"
#endif

#include "RendererBase.h"
#include "GameTimer.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace NCL;
using namespace Rendering;

Window*		Window::window		= nullptr;
Keyboard*	Window::keyboard	= nullptr;
Mouse*		Window::mouse		= nullptr;
GameTimer	Window::timer;

Window::Window()	{
	renderer	= nullptr;
	window		= this;
}

Window::~Window()	{
	delete keyboard;keyboard= nullptr;
	delete mouse;	mouse	= nullptr;
	window = nullptr;
}

Window* Window::CreateGameWindow(std::string title, int sizeX, int sizeY, bool fullScreen, int offsetX, int offsetY) {
	if (window) {
		return nullptr;
	}
#ifdef _WIN32
	return new Win32Code::Win32Window(title, sizeX, sizeY, fullScreen, offsetX, offsetY);
#endif
#ifdef __ORBIS__
	return new PS4::PS4Window(title, sizeX, sizeY, fullScreen, offsetX, offsetY);
#endif
}

void	Window::SetRenderer(RendererBase* r) {
	if (renderer && renderer != r) {
		renderer->OnWindowDetach();
	}
	renderer = r;

	if (r) {
		renderer->OnWindowResize(size.x, size.y);
	}
}

bool	Window::UpdateWindow() {
	std::this_thread::yield();
	timer.Tick();

	if (mouse) {
		mouse->UpdateFrameState(timer.GetTimeDeltaMSec());
	}
	if (keyboard) {
		keyboard->UpdateFrameState(timer.GetTimeDeltaMSec());
	}

	return InternalUpdate();
}

void Window::ResizeRenderer() {
	if (renderer) {
		renderer->OnWindowResize((int)size.x, (int)size.y);
	}
}

void Window::GetLocalIPV4Address(std::string& IPAddress)
{
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return;
	}

	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
		std::cerr << "Error:" << WSAGetLastError() << " when getting local host name.\n";
		WSACleanup();
		return;
	}

	struct addrinfo hints = {}, * addrs;
	hints.ai_family = AF_INET; // AF_INET for IPv4, AF_INET6 for IPv6, AF_UNSPEC for either
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int result = getaddrinfo(hostname, nullptr, &hints, &addrs);
	if (result != 0) {
		std::cerr << "getaddrinfo failed with error: " << result << "\n";
		WSACleanup();
		return;
	}

	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &((struct sockaddr_in*)addrs->ai_addr)->sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Address: " << ipstr << std::endl;
	IPAddress = ipstr;

	freeaddrinfo(addrs);
	WSACleanup();

#else
	// play station
#endif
}