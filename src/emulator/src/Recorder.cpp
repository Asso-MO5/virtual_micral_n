#include "Recorder.h"

void Recorder::pause() { paused = true; }
void Recorder::resume() { paused = false; }
bool Recorder::is_paused() const { return paused; }
