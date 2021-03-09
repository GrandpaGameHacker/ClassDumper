#include "Symbols.h"
const string VTABLE_SYMBOL_PREFIX = "??_7";
const string VTABLE_SYMBOL_SUFFIX = "6B@";
static char buff[0x1000];

string DemangleMicrosoft(char* symbol)
{
	memset(buff, 0, 0x1000);
	char* pSymbol = symbol;
	// don't question this magic, it works :)
	if (*static_cast<char*>(symbol + 4) == '?') pSymbol = symbol + 1;
	else if (*static_cast<char*>(symbol) == '.') pSymbol = symbol + 4;
	else if (*static_cast<char*>(symbol) == '?') pSymbol = symbol + 2;
	else
	{
		g_console.WriteBold("invalid msvc mangled name");
	}
	string ModifiedSymbol = pSymbol;
	ModifiedSymbol.insert(0, VTABLE_SYMBOL_PREFIX);
	ModifiedSymbol.insert(ModifiedSymbol.size(), VTABLE_SYMBOL_SUFFIX);
	if (!((UnDecorateSymbolName(ModifiedSymbol.c_str(), buff, 0x1000, 0)) != 0))
	{
		g_console.FWriteBold("Error Code: %d", GetLastError());
		return string(symbol); //Failsafe
	}
	return string(buff);
}

void StringFilter(string& string, const std::string& substring)
{
	size_t pos;
	while ((pos = string.find(substring)) != string::npos)
	{
		string.erase(pos, substring.length());
	}
}

static vector<string> filters =
{
	"::`vftable'",
	"const ",
	"::`anonymous namespace'"
};

void FilterSymbol(string& symbol)
{
	for (auto& filter : filters)
	{
		StringFilter(symbol, filter);
	}
}

bool SymbolComparator(uintptr_t v1, uintptr_t v2)
{
	v1 = v1 - sizeof(uintptr_t);
	v2 = v2 - sizeof(uintptr_t);
	auto* pv1 = reinterpret_cast<uintptr_t*>(v1);
	auto* pv2 = reinterpret_cast<uintptr_t*>(v2);
	auto* col1 = reinterpret_cast<CompleteObjectLocator*>(*pv1);
	auto* col2 = reinterpret_cast<CompleteObjectLocator*>(*pv2);
	auto* td1 = col1->GetTypeDescriptor();
	auto* td2 = col2->GetTypeDescriptor();

	const auto symbol1 = DemangleMicrosoft(&td1->name);
	const auto symbol2 = DemangleMicrosoft(&td2->name);

	if (symbol1 == symbol2)
	{
		return (v1 < v2);
	}
	return (symbol1 < symbol2);
}

void SortSymbols(vector<uintptr_t>& vtableList)
{
	sort(vtableList.begin(), vtableList.end(), SymbolComparator);
}
