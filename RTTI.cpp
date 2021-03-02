#include "RTTI.h"
#ifdef _WIN64
uintptr_t ModuleBase = 0;

TypeDescriptor* CompleteObjectLocator::GetTypeDescriptor()
{
	const auto ptr = reinterpret_cast<uintptr_t>(&signature);
	ModuleBase = ptr - CompleteObjectLocatorOffset;
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}

ClassHierarchyDescriptor* CompleteObjectLocator::GetClassDescriptor()
{
	return reinterpret_cast<ClassHierarchyDescriptor*>(ModuleBase + ClassDescriptorOffset);
}


BaseClassArray* ClassHierarchyDescriptor::GetBaseClassArray()
{
	return reinterpret_cast<BaseClassArray*>(ModuleBase + BaseClassArrayOffset);
}

BaseClassDescriptor* BaseClassArray::GetBaseClassDescriptor(unsigned long index)
{
	return reinterpret_cast<BaseClassDescriptor*>(ModuleBase + arrayOfBaseClassDescriptorOffsets[index]);
}

TypeDescriptor* BaseClassDescriptor::GetTypeDescriptor()
{
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}

#else
TypeDescriptor* CompleteObjectLocator::GetTypeDescriptor() {
	return this->pTypeDescriptor;
}
ClassHierarchyDescriptor* CompleteObjectLocator::GetClassDescriptor()
{
	return this->pClassDescriptor;
}


BaseClassArray* ClassHierarchyDescriptor::GetBaseClassArray()
{
	return this->pBaseClassArray;
}

BaseClassDescriptor* BaseClassArray::GetBaseClassDescriptor(unsigned long index)
{
	return this->arrayOfBaseClassDescriptors[index];
}

TypeDescriptor* BaseClassDescriptor::GetTypeDescriptor()
{
	return this->pTypeDescriptor;
}
#endif

ClassMeta::ClassMeta(uintptr_t VTable)
{
	this->VTable = reinterpret_cast<uintptr_t*>(VTable);
	this->Meta = this->VTable - 1;
	COL = reinterpret_cast<CompleteObjectLocator*>(*Meta);
	pTypeDescriptor = COL->GetTypeDescriptor();
	pClassDescriptor = COL->GetClassDescriptor();
	pClassArray = pClassDescriptor->GetBaseClassArray();
	numBaseClasses = pClassDescriptor->numBaseClasses;
	bMultipleInheritance = (pClassDescriptor->attributes >> 0) & 1;
	bVirtualInheritance = (pClassDescriptor->attributes >> 1) & 1;
	bAmbigious = (pClassDescriptor->attributes >> 2) & 1;
}

BaseClassDescriptor* ClassMeta::GetBaseClass(unsigned long index)
{
	return pClassArray->GetBaseClassDescriptor(index);
}
