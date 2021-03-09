#include "Dumper.h"

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
	auto FunctionList = GetListOfFunctions(reinterpret_cast<void*>(VTable), sectionInfo);
	if (!FunctionList.empty())
	{
		const size_t nFunctions = FunctionList.size();
		VTableLog << "\tVirtual Functions (" << dec << nFunctions << "):\n";

		//Simple Function Classification (Similar to IDA naming conventions)
		int index = 0;
		int realOffset = 0;
		for (auto function : FunctionList)
		{
			realOffset = index * sizeof(void*);
			VTableLog << "\t" << dec << index << "\t" << hex << realOffset
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
	unsigned long prevMemberDisplacement = 0;
	unsigned long tabIndex = 0;
	for (unsigned long i = 1; i < classMeta.numBaseClasses; i++)
	{
		BaseClassDescriptor* pCurrentBaseClass = classMeta.GetBaseClass(i);
		TypeDescriptor* pCurrentTypeDesc = pCurrentBaseClass->GetTypeDescriptor();
		const unsigned long memberDisplacement = pCurrentBaseClass->where.mdisp;
		const unsigned long vbtableDisplacement = pCurrentBaseClass->where.pdisp;
		//const ptrdiff_t vdisp = pCurrentBaseClass->where.vdisp; // unused for now
		string currentBaseClassName = DemangleMicrosoft(&pCurrentTypeDesc->name);
		FilterSymbol(currentBaseClassName);
		if (vbtableDisplacement == -1)
		{
			InheritanceLog << hex << "0x" << memberDisplacement;
			// if pdisp is -1, the vtable offset for base class is actually mdisp
			if (memberDisplacement == prevMemberDisplacement) { 
				tabIndex++;
			}
			else {
				tabIndex = 1;
			}
			for (auto i = 0; i < tabIndex; i++) {
				InheritanceLog << "\t";
			}
		}
		// else, I dont know how to parse the vbtable yet;
		InheritanceLog << "\t" << currentBaseClassName << "\n";
		prevMemberDisplacement = memberDisplacement;
	}
	InheritanceLog << "\n";
}
