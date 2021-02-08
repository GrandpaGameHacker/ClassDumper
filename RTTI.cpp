#include "RTTI.h"
#ifdef _WIN64
TypeDescriptor* CompleteObjectLocator::GetTypeDescriptor(uintptr_t ModuleBase) {
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}
ClassHierarchyDescriptor* CompleteObjectLocator::GetClassDescriptor(uintptr_t ModuleBase)
{
	return reinterpret_cast<ClassHierarchyDescriptor*>(ModuleBase + ClassDescriptorOffset);
}


BaseClassArray* ClassHierarchyDescriptor::GetBaseClassArray(uintptr_t ModuleBase)
{
	return reinterpret_cast<BaseClassArray*>(ModuleBase + BaseClassArrayOffset);
}

BaseClassDescriptor* BaseClassArray::GetBaseClassDescriptor(unsigned long index, uintptr_t ModuleBase)
{
	return reinterpret_cast<BaseClassDescriptor*>(ModuleBase + arrayOfBaseClassDescriptorOffsets[index]);
}

TypeDescriptor* BaseClassDescriptor::GetTypeDescriptor(uintptr_t ModuleBase)
{
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}
#endif