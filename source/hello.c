#include "hello.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void initHelloWorld(HelloWorldScreenData* data)
{
	char helloWorldText[32] = "Hello World!";
	memset(data, 0, sizeof(HelloWorldScreenData));
	strncpy(data->helloWorldText, helloWorldText, sizeof(helloWorldText));
	data->helloWorldText[strlen(helloWorldText)] = '\0';
}

void renderHelloWorld(NVGcontext* vg, float w, float h, float x, float y, HelloWorldScreenData* data)
{
	// Set Screen Background to "Grey"
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
	nvgFill(vg);

	nvgFontFace(vg, "switch-standard");

	if (data->helloWorldText[0] != '\0') {
		nvgFontSize(vg, 12.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(240,240,240,192));
		nvgText(vg, x + 50, y + 50, data->helloWorldText, NULL);
	}
}