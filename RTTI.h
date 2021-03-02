#pragma once
#include <Windows.h>

struct PMD
{
	unsigned long mdisp; // member displacement
	long pdisp; // vbtable displacement
	long vdisp; // displacement inside vbtable
};

struct TypeDescriptor
{
	uintptr_t pVFTable; // type_info vftable ptr
	uintptr_t reserved; // reserved for future use
	char name; // type name
};

#ifdef _WIN64
extern uintptr_t ModuleBase;

struct BaseClassDescriptor
{
	unsigned long TypeDescriptorOffset; // type descriptor of the class
	unsigned long numContainedBases; // number of nested classes in BaseClassArray
	PMD where; // pointer to member displacement info
	unsigned long attributes; // flags, generally unused

	TypeDescriptor* GetTypeDescriptor();
};

#pragma warning(disable : 4200)
struct BaseClassArray
{
	unsigned long arrayOfBaseClassDescriptorOffsets[]; // describes base classes for the complete class
	BaseClassDescriptor* GetBaseClassDescriptor(unsigned long index);
};
#pragma warning (default : 4200)

struct ClassHierarchyDescriptor
{
	unsigned long signature; // 1 if 64 bit, 0 if 32bit
	unsigned long attributes; // bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
	unsigned long numBaseClasses; // number of classes in pBaseClassArray
	unsigned long BaseClassArrayOffset;

	BaseClassArray* GetBaseClassArray();
};

struct CompleteObjectLocator
{
	unsigned long signature; // 1 if 64 bit, 0 if 32bit
	unsigned long offset; // offset of this vftable in the complete class
	unsigned long cdOffset; // constructor displacement offset
	unsigned long TypeDescriptorOffset; //  TypeDescriptor of the complete class
	unsigned long ClassDescriptorOffset; // describes inheritance hierarchy
	unsigned long CompleteObjectLocatorOffset; // Used to get the base address of module

	TypeDescriptor* GetTypeDescriptor();
	ClassHierarchyDescriptor* GetClassDescriptor();
};

#else

struct BaseClassDescriptor
{
	TypeDescriptor* pTypeDescriptor; // type descriptor of the class
	unsigned long numContainedBases; // number of nested classes in BaseClassArray
	PMD where; // pointer to member displacement info
	unsigned long attributes; // flags, generally unused

	TypeDescriptor* GetTypeDescriptor();
};

struct BaseClassArray {
	// 0x4000 is the maximum number of inheritance allowed in some standards, but it will never exceed that lol ;)
	// Did this to avoid using C99 Variable Length Arrays, its not in the C++ standard
	BaseClassDescriptor* arrayOfBaseClassDescriptors[0x4000]; // describes base classes for the complete class
	BaseClassDescriptor* GetBaseClassDescriptor(unsigned long index);
};

struct ClassHierarchyDescriptor
{
	unsigned long signature; // 1 if 64 bit, 0 if 32bit
	unsigned long attributes; // bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
	unsigned long numBaseClasses;// number of classes in pBaseClassArray
	BaseClassArray* pBaseClassArray;

	BaseClassArray* GetBaseClassArray();
};

struct CompleteObjectLocator
{
	unsigned long signature; // 1 if 64 bit, 0 if 32bit
	unsigned long offset; // offset of this vftable in the complete class
	unsigned long cdOffset; // constructor displacement offset
	TypeDescriptor* pTypeDescriptor; //  TypeDescriptor of the complete class
	ClassHierarchyDescriptor* pClassDescriptor; // describes inheritance hierarchy

	TypeDescriptor* GetTypeDescriptor();
	ClassHierarchyDescriptor* GetClassDescriptor();
};
#endif

struct ClassMeta
{
	explicit ClassMeta(uintptr_t VTable);
	BaseClassDescriptor* GetBaseClass(unsigned long index);

	uintptr_t* VTable;
	uintptr_t* Meta;
	CompleteObjectLocator* COL;
	TypeDescriptor* pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor;
	BaseClassArray* pClassArray;
	unsigned long numBaseClasses;
	bool bMultipleInheritance;
	bool bVirtualInheritance;
	bool bAmbigious;
};
