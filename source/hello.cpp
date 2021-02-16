#include "hello.hpp"

HelloWorldScreen::HelloWorldScreen()
{
	this->m_helloWorldText = "Hello World!";
}

void HelloWorldScreen::render(NVGcontext* vg, float w, float h, float x, float y)
{
	// Set Screen Background to "Grey"
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
	nvgFill(vg);

	nvgFontFace(vg, "switch-standard");

	nvgFontSize(vg, 12.0f);
	nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
	nvgFillColor(vg, nvgRGBA(240,240,240,192));
	nvgText(vg, x + 50, y + 50, this->m_helloWorldText.c_str(), NULL);
}