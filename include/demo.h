#pragma once
#include <nanovg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DemoData {
    int images[12];
};
typedef struct DemoData DemoData;

int loadDemoData(NVGcontext* vg, DemoData* data);
void freeDemoData(NVGcontext* vg, DemoData* data);
void renderDemo(NVGcontext* vg, float mx, float my, float width, float height, float t, int blowup, DemoData* data);

#ifdef __cplusplus
}
#endif
