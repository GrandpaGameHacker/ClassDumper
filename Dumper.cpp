#include "Dumper.h"


bool IsValid(void* VTable_start, SectionInfo* sectionInfo)
{
	auto* vtable = static_cast<uintptr_t*>(VTable_start);
	auto* meta = vtable - 1;
	if (sectionInfo->RDATA.base <= *meta && *meta <= sectionInfo->RDATA.end)
	{
		if (sectionInfo->TEXT.base <= *vtable && *vtable <= sectionInfo->TEXT.end)
		{
			auto* COL = reinterpret_cast<CompleteObjectLocator*>(*meta);
#ifdef _WIN64
			if (COL->signature == 1)
			{
				auto *descriptor = COL->GetTypeDescriptor();
#else
			if (COL->signature == 0) {
				auto descriptor = COL->pTypeDescriptor;
#endif
				if (IsBadReadPointer(reinterpret_cast<void*>(descriptor)) != 0)
				{
					return false;
				}
				// Test if string is ".?AV" real quick
				if (*reinterpret_cast<unsigned long*>(&descriptor->name) == TYPEDESCRIPTOR_SIGNITURE)
				{
					return true;
				}
			}
		}
	}
	return false;
}

vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo)
{
	vector<uintptr_t> functionList;
	auto* vtable = static_cast<uintptr_t*>(VTable_start);
	auto function_ptr = *vtable;
	while (sectionInfo->TEXT.base <= function_ptr && function_ptr <= sectionInfo->TEXT.end)
	{
		functionList.push_back(function_ptr);
		vtable++;
		function_ptr = *vtable;
	}
	return functionList;
}

vector<uintptr_t> FindAllVTables(SectionInfo* sectionInfo)
{
	vector<uintptr_t> vtableList;
	uintptr_t ptr = sectionInfo->RDATA.base + sizeof(uintptr_t);
	while (ptr < sectionInfo->RDATA.end)
	{
		if (IsValid(reinterpret_cast<void*>(ptr), sectionInfo))
		{
			vtableList.push_back(ptr);
		}
		ptr += sizeof(uintptr_t);
	}
	return vtableList;
}

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

// having a big buffer seems to speed things up
ofstream VTableLog;
ofstream InheritanceLog;
const size_t bufsize = 1024 * 1024;
char buf1[bufsize];
char buf2[bufsize];

void InitializeLogs(const string& folderName)
{
	string vtable_path = Utf8Encode(GetDesktopPath());
	string inheritance_path = vtable_path;

	vtable_path += "/Class_Dumper/";
	CreateDirectory(vtable_path.c_str(), nullptr);
	vtable_path += folderName;
	CreateDirectory(vtable_path.c_str(), nullptr);
	vtable_path += "/vtable.txt";
	VTableLog.open(vtable_path);


	inheritance_path += "/Class_Dumper/";
	CreateDirectory(inheritance_path.c_str(), nullptr);
	inheritance_path += folderName;
	CreateDirectory(inheritance_path.c_str(), nullptr);
	inheritance_path += "/inheritance.txt";
	InheritanceLog.open(inheritance_path);

	VTableLog.rdbuf()->pubsetbuf(buf1, bufsize);
	InheritanceLog.rdbuf()->pubsetbuf(buf2, bufsize);
}

void LogModuleStart(char* moduleName)
{
	VTableLog << "<" << moduleName << '>' << "\n";
	InheritanceLog << "<" << moduleName << '>' << "\n";
}

void LogModuleEnd(char* moduleName)
{
	VTableLog << "< end " << moduleName << '>' << "\n\n";
	InheritanceLog << "< end " << moduleName << '>' << "\n\n";
}

void CloseLogs()
{
	VTableLog.close();
	InheritanceLog.close();
}

void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo)
{
	static string prevClassName;
	bool bIsVTableOffset = false;

	auto classMeta = ClassMeta(VTable);

	string className = DemangleMicrosoft(&classMeta.pTypeDescriptor->name);
	string parentClassName = className;
	FilterSymbol(parentClassName);

	// if class is an interface/inherited virtual class for the previous class
	// grab the real name
	if (className == prevClassName && classMeta.COL->offset != 0)
	{
		const unsigned long v_offset = classMeta.COL->offset;
		for (unsigned long i = 0; i < classMeta.numBaseClasses; i++)
		{
			BaseClassDescriptor* pCurrentBaseClass = classMeta.GetBaseClass(i);
			TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->GetTypeDescriptor();

			if (pCurrentBaseClass->where.mdisp == v_offset)
			{
				bIsVTableOffset = true;
				className = DemangleMicrosoft(&pCurrentTypeDesc->name);
				break;
			}
		}
	}
	else
	{
		prevClassName = className;
	}
	// virtual class attributes
	const char M = (classMeta.bMultipleInheritance) ? 'M' : ' ';
	const char V = (classMeta.bVirtualInheritance) ? 'V' : ' ';
	const char A = (classMeta.bAmbigious) ? 'A' : ' ';

	// dump table info
	if (bIsVTableOffset)
	{
		VTableLog << M << V << A
			<< hex << " 0x" << VTable
			<< "\t+" << GetRVA(VTable, sectionInfo)
			<< "\t" << parentClassName << " -> " << className << "\t" << "\n";
	}
	else
	{
		VTableLog << M << V << A
			<< hex << " 0x" << VTable
			<< "\t+" << GetRVA(VTable, sectionInfo)
			<< "\t" << className << "\t" << "\n";
	}
	// dump functions
	int index = 0;
	auto FunctionList = GetListOfFunctions(reinterpret_cast<void*>(VTable), sectionInfo);
	if (!FunctionList.empty())
	{
		const size_t nFunctions = FunctionList.size();
		VTableLog << "\tVirtual Functions (" << nFunctions << "):\n";

		//Simple Function Classification (Similar to IDA naming conventions)
		for (auto function : FunctionList)
		{
			VTableLog << "\t" << dec << index
				<< "\t" << hex << "0x" << function
				<< "\t+" << GetRVA(function, sectionInfo);

			BYTE* fnByte = reinterpret_cast<BYTE*>(function);
			if (fnByte[0] == RET_INSTR or fnByte[0] == RET_INT_INSTR)
			{
				VTableLog << "\t\tnullsub_" << hex << function << "\n";
			}
			else
			{
				VTableLog << "\t\tsub_" << hex << function << "\n";
			}
			index++;
		}
	}
	VTableLog << "\n\n";
}

void DumpInheritanceInfo(uintptr_t VTable)
{
	static string LastclassName; //used for filtering duplicate classes

	auto classMeta = ClassMeta(VTable);

	string className = DemangleMicrosoft(&classMeta.pTypeDescriptor->name);

	if (className == LastclassName)
		return;

	LastclassName = className;

	FilterSymbol(className);
	if (classMeta.numBaseClasses > 1)
	{
		InheritanceLog << className << ":" << "\n";
	}
	else
	{
		InheritanceLog << className << " (No Base Classes)" << "\n\n";
		return;
	}
	// iterate and dump all base classes
	for (unsigned long i = 1; i < classMeta.numBaseClasses; i++)
	{
		BaseClassDescriptor* pCurrentBaseClass = classMeta.GetBaseClass(i);
		TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->GetTypeDescriptor();
		const ptrdiff_t mdisp = pCurrentBaseClass->where.mdisp;
		const ptrdiff_t pdisp = pCurrentBaseClass->where.pdisp;
		//const ptrdiff_t vdisp = pCurrentBaseClass->where.vdisp; // unused for now

		string currentBaseClassName = DemangleMicrosoft(&pCurrentTypeDesc->name);
		FilterSymbol(currentBaseClassName);
		if (pdisp == -1)
		{
			// if pdisp is -1, the vtable offset for base class is actually mdisp
			InheritanceLog << hex << "0x" << mdisp << "\t";
		}
		// else, I dont know how to parse the vbtable yet;
		InheritanceLog << currentBaseClassName << "\n";
	}
	InheritanceLog << "\n";
}
