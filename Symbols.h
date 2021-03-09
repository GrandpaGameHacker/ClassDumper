#pragma once
#include "stdheaders.h"
#include "RTTI.h"
#include <algorithm>
using namespace std;
extern Console g_console;
string DemangleMicrosoft(char* symbol);
void StringFilter(string& string, const std::string& substring);
void FilterSymbol(string& symbol);

bool SymbolComparator(uintptr_t v1, uintptr_t v2);
void SortSymbols(vector<uintptr_t>& vtableList);
