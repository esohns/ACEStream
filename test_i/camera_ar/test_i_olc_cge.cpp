#define OLC_CGE_APPLICATION
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmeapi.h"

#include "olcConsoleGameEngine.h"
#else
#include "archive/olcConsoleGameEngineSDL.h"

int len = 0, done = 0, bits = 0, which = 0,
  sample_size = 0, position = 0, rate = 0;
Sint16 *stream[2];

std::atomic<bool> olcConsoleGameEngine::m_bAtomActive(false);
std::condition_variable olcConsoleGameEngine::m_cvGameFinished;
std::mutex olcConsoleGameEngine::m_muxGame;
#endif // ACE_WIN32 || ACE_WIN64
