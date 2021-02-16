#ifndef HELLO_HPP
#define HELLO_HPP
#include <string>
#include <nanovg.h>

class HelloWorldScreen
{
private:
	std::string m_helloWorldText;

public:
	HelloWorldScreen();
	void render(NVGcontext* vg, float w, float h, float x, float y);
};
#endif /* HELLO_HPP */