#pragma once
#define WIN32_LEAN_MEAN

#include <windows.h>
#include <tchar.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

template<class T>
void SafeRelease(T p) {
	if (p)
	{
		p->Release();
		p = nullptr;
	}
}