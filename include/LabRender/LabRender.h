#ifndef LABRENDER_H
#define LABRENDER_H

#include <LabRender/Export.h>

// returns < 0 if GL could not be initialized
extern "C"
int labrender_init();

extern "C"
void labrender_shutdown();

#endif
