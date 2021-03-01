#include "Dumper.h"


bool IsValid(void* VTable_start, SectionInfo* sectionInfo)
{
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable_start);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	if (sectionInfo->RDATA.base <= *meta_ptr && *meta_ptr <= sectionInfo->RDATA.end) {
		if (sectionInfo->TEXT.base <= *vftable_ptr && *vftable_ptr <= sectionInfo->TEXT.end) {
			CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
#ifdef _WIN64
			if (COL->signature == 1) {
				auto TypeDesc = COL->GetTypeDescriptor();
#else
			if (COL->signature == 0) {
				auto TypeDesc = COL->pTypeDescriptor;
#endif
				if (IsBadReadPointer(reinterpret_cast<void*>(TypeDesc)) != 0) {
					return false;
				}
				// Test if string is ".?AV" real quick
				if (*reinterpret_cast<unsigned long*>(&TypeDesc->name) == TYPEDESCRIPTOR_SIGNITURE) {
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
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable_start);
	uintptr_t function_ptr = *vftable_ptr;
	while (sectionInfo->TEXT.base <= function_ptr && function_ptr <= sectionInfo->TEXT.end) {
		functionList.push_back(function_ptr);
		vftable_ptr++;
		function_ptr = *vftable_ptr;
	}
	return functionList;
}

vector<uintptr_t> FindAllVTables(SectionInfo* sectionInfo)
{
	vector<uintptr_t> VTableList;
	uintptr_t ptr = sectionInfo->RDATA.base + sizeof(uintptr_t);
	while (ptr < sectionInfo->RDATA.end) {
		if (IsValid(reinterpret_cast<void*>(ptr), sectionInfo)) {
			VTableList.push_back(ptr);
		}
		ptr += sizeof(uintptr_t);
	}
	return VTableList;
}

const string VFTableSymbolStart = "??_7";
const string VFTableSymbolEnd = "6B@";
static char buff[0x1000];

string DemangleMSVC(char* symbol)
{
	memset(buff, 0, 0x1000);
	char* pSymbol = symbol;
	if (*(char*)(symbol + 4) == '?') pSymbol = symbol + 1;
	else if (*(char*)symbol == '.') pSymbol = symbol + 4;
	else if (*(char*)symbol == '?') pSymbol = symbol + 2;
	else
	{
		g_console.WriteBold("invalid msvc mangled name");
	}
	string ModifiedSymbol = pSymbol;
	ModifiedSymbol.insert(0, VFTableSymbolStart);
	ModifiedSymbol.insert(ModifiedSymbol.size(), VFTableSymbolEnd);
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

void FilterSymbol(string& Symbol)
{
	vector<string> filters = 
	{
		"::`vftable'",
		"const ",
		"::`anonymous namespace'"
	};
	for (string filter : filters)
	{
		StringFilter(Symbol, filter);
	}
}

bool SymbolComparator(uintptr_t v1, uintptr_t v2)
{
	v1 = v1 - sizeof(uintptr_t); v2 = v2 - sizeof(uintptr_t);
	uintptr_t* v1p = (uintptr_t*) v1; uintptr_t* v2p = (uintptr_t*) v2;
	CompleteObjectLocator* COL1 = reinterpret_cast<CompleteObjectLocator*>(*v1p);
	CompleteObjectLocator* COL2 = reinterpret_cast<CompleteObjectLocator*>(*v2p);
#ifdef _WIN64
	TypeDescriptor* TD1 = COL1->GetTypeDescriptor();
	TypeDescriptor* TD2 = COL2->GetTypeDescriptor();
#else
	TypeDescriptor* TD1 = COL1->pTypeDescriptor;
	TypeDescriptor* TD2 = COL2->pTypeDescriptor;
#endif

	string Symbol1 = DemangleMSVC(&TD1->name);
	string Symbol2 = DemangleMSVC(&TD2->name);
	
	if (Symbol1 == Symbol2) {
		return (v1 < v2);
	}
	else {
		return (Symbol1 < Symbol2);
	}
}

void SortSymbols(vector<uintptr_t>& vtable_list)
{
	sort(vtable_list.begin(), vtable_list.end(), SymbolComparator);
}


ofstream VTableLog;
ofstream InheritanceLog;
const size_t bufsize = 1024 * 1024;
char buf1[bufsize];
char buf2[bufsize];

void InitializeLogs(string folderName)
{
	string vtable_path = utf8_encode(GetDesktopPath());
	string inheritance_path = vtable_path;

	vtable_path += "/Class_Dumper/";
	CreateDirectory(vtable_path.c_str(), 0);
	vtable_path += folderName;
	CreateDirectory(vtable_path.c_str(), 0);
	vtable_path += "/vtable.txt";
	VTableLog.open(vtable_path);

	
	inheritance_path += "/Class_Dumper/";
	CreateDirectory(inheritance_path.c_str(), 0);
	inheritance_path += folderName;
	CreateDirectory(inheritance_path.c_str(), 0);
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
	static string LastclassName;
	bool bIsVTableOffsetClass = false;
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
#ifdef _WIN64
	TypeDescriptor* pTypeDescriptor = COL->GetTypeDescriptor();
	ClassHierarchyDescriptor* pClassDescriptor = COL->GetClassDescriptor();
	BaseClassArray* pClassArray = pClassDescriptor->GetBaseClassArray();
#else
	TypeDescriptor* pTypeDescriptor = COL->pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor = COL->pClassDescriptor;
	BaseClassArray* pClassArray = pClassDescriptor->pBaseClassArray;
#endif
	string className = DemangleMSVC(&pTypeDescriptor->name);
	string OwnerClassName = className;
	FilterSymbol(OwnerClassName);
	if (className == LastclassName && COL->offset != 0)
	{
		unsigned long numBaseClasses = pClassDescriptor->numBaseClasses;
		unsigned long v_offset = COL->offset;
		for (unsigned long i = 0; i < numBaseClasses; i++) {
#ifdef _WIN64
			BaseClassDescriptor* pCurrentBaseClass = pClassArray->GetBaseClassDescriptor(i);
			TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->GetTypeDescriptor();
#else
			BaseClassDescriptor* pCurrentBaseClass = pClassArray->arrayOfBaseClassDescriptors[i];
			TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->pTypeDescriptor;
#endif
			if (pCurrentBaseClass->where.mdisp == v_offset) {
				bIsVTableOffsetClass = true;
				className = DemangleMSVC(&pCurrentTypeDesc->name);
				break;
			}
		}
	}
	else {
		LastclassName = className;
	}
	auto FunctionList = GetListOfFunctions((void*)VTable, sectionInfo);
	bool MultipleInheritance = pClassDescriptor->attributes & 0b01;
	bool VirtualInheritance = pClassDescriptor->attributes & 0b10;
	char MH = (MultipleInheritance) ? 'M' : ' ';
	char VH = (VirtualInheritance) ? 'V' : ' ';
	if (bIsVTableOffsetClass) {
		VTableLog << MH << VH << hex << "0x" << VTable << "\t+" << GetRVA(VTable, sectionInfo) << "\t" << OwnerClassName << " -> " << className << "\t" << "\n";
	}
	else {
		VTableLog << MH << VH << hex << "0x" << VTable << "\t+" << GetRVA(VTable, sectionInfo) << "\t" << className << "\t" << "\n";
	}
	
	int index = 0;
	if (!FunctionList.empty())
	{
		VTableLog << "\tVirtual Functions:\n";
		// Function Classification (Similar to IDA naming conventions) (disgusting code dont read)
		for (auto function : FunctionList) {
			VTableLog << "\t" << dec << index << "\t" << hex << "0x" << function << "\t+" << GetRVA(function, sectionInfo);
			BYTE* fnByte = (BYTE*) function;
			if (fnByte[0] == RET_INSTR) {
				VTableLog << "\t\tnullsub_" << hex << function << "\n";
			}
			else if(fnByte[0] == RET_INT_INSTR){
				WORD ret_integer = *(WORD*)&fnByte[1];
				VTableLog << "\t\tret" << ret_integer << "_" << hex << function << "\n";
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

void DumpInheritanceInfo(uintptr_t VTable, SectionInfo* sectionInfo)
{
	static string LastclassName;
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
#ifdef _WIN64
	TypeDescriptor* pTypeDescriptor = COL->GetTypeDescriptor();
	ClassHierarchyDescriptor* pClassDescriptor = COL->GetClassDescriptor();
	BaseClassArray* pClassArray = pClassDescriptor->GetBaseClassArray();
#else
	TypeDescriptor* pTypeDescriptor = COL->pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor = COL->pClassDescriptor;
	BaseClassArray* pClassArray = pClassDescriptor->pBaseClassArray;
#endif
	string className = DemangleMSVC(&pTypeDescriptor->name);
	if (className == LastclassName) {
		return;
	}
	LastclassName = className;
	FilterSymbol(className);
	unsigned long numBaseClasses = pClassDescriptor->numBaseClasses;
	if (numBaseClasses > 1)\
	{
		InheritanceLog << className << ":" << "\n";
	}
	else
	{
		InheritanceLog << className << " (No Base Classes)" << "\n\n";
		return;
	}
	
	for (unsigned long i = 1; i < numBaseClasses; i++) {
#ifdef _WIN64
		BaseClassDescriptor* pCurrentBaseClass = pClassArray->GetBaseClassDescriptor(i);
		TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->GetTypeDescriptor();
#else
		BaseClassDescriptor* pCurrentBaseClass = pClassArray->arrayOfBaseClassDescriptors[i];
		TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->pTypeDescriptor;
#endif
		ptrdiff_t mdisp = pCurrentBaseClass->where.mdisp;
		ptrdiff_t pdisp = pCurrentBaseClass->where.pdisp;
		ptrdiff_t vdisp = pCurrentBaseClass->where.vdisp;

		string currentBaseClassName = DemangleMSVC(&pCurrentTypeDesc->name);
		FilterSymbol(currentBaseClassName);
		if (pdisp == -1) { // if pdisp is -1, the vtable offset for base class is actually mdisp
			InheritanceLog << hex << "0x" << mdisp << "\t";
		}
		// else, I dont know how to parse the vbtable yet;
		InheritanceLog << currentBaseClassName << "\n";
	}
	InheritanceLog << "\n";
}