#ifndef HELLO_H
#define HELLO_H
#include <nanovg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HelloWorldScreenData {
	char helloWorldText[32];
};
typedef struct HelloWorldScreenData HelloWorldScreenData;

void initHelloWorld(HelloWorldScreenData* data);
void renderHelloWorld(NVGcontext* vg, float w, float h, float x, float y, HelloWorldScreenData* data);

#ifdef __cplusplus
}
#endif

#endif /* HELLO_H */