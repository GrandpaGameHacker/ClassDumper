#pragma once
#include <Windows.h>
// RTTI
//struct CompleteObjectLocator;
//struct ClassHierarchyDescriptor;
//struct BaseClassArray;
//struct BaseClassDescriptor;
//struct PMD;
//struct TypeDescriptor;

#ifdef _WIN64
struct TypeDescriptor
{
    uintptr_t pVFTable; // type_info vftable ptr
    uintptr_t reserved; // reserved for future use
    char name; // type name
};

struct PMD
{
    ptrdiff_t mdisp; // member displacement
    ptrdiff_t pdisp; // vbtable displacement
    ptrdiff_t vdisp; // displacement inside vbtable
};

struct BaseClassDescriptor
{
    unsigned long TypeDescriptorOffset; // type descriptor of the class
    unsigned long numContainedBases; // number of nested classes in BaseClassArray
    PMD where; // pointer to member displacement info
    unsigned long attributes; // flags, generally unused

    TypeDescriptor* GetTypeDescriptor(uintptr_t ModuleBase);
};

#pragma warning(disable : 4200)
struct BaseClassArray {
    unsigned long arrayOfBaseClassDescriptorOffsets[]; // describes base classes for the complete class
    BaseClassDescriptor* GetBaseClassDescriptor(unsigned long index, uintptr_t ModuleBase);
};
#pragma warning (default : 4200)

struct ClassHierarchyDescriptor
{
    unsigned long signature; // 1 if 64 bit, 0 if 32bit
    unsigned long attributes; // bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
    unsigned long numBaseClasses;// number of classes in pBaseClassArray
    unsigned long BaseClassArrayOffset;

    BaseClassArray* GetBaseClassArray(uintptr_t ModuleBase);
};

struct CompleteObjectLocator
{
    unsigned long signature; // 1 if 64 bit, 0 if 32bit
    unsigned long offset; // offset of this vftable in the complete class
    unsigned long cdOffset; // constructor displacement offset
    unsigned long TypeDescriptorOffset; //  TypeDescriptor of the complete class
    unsigned long ClassDescriptorOffset; // describes inheritance hierarchy

    TypeDescriptor* GetTypeDescriptor(uintptr_t ModuleBase);
    ClassHierarchyDescriptor* GetClassDescriptor(uintptr_t ModuleBase);
};
#else

struct TypeDescriptor
{
    uintptr_t pVFTable; // type_info vftable ptr
    uintptr_t reserved; // reserved for future use
    char name; // type name
};

struct PMD
{
    ptrdiff_t mdisp; // member displacement
    ptrdiff_t pdisp; // vbtable displacement
    ptrdiff_t vdisp; // displacement inside vbtable
};

struct BaseClassDescriptor
{
    TypeDescriptor* pTypeDescriptor; // type descriptor of the class
    unsigned long numContainedBases; // number of nested classes in BaseClassArray
    PMD where; // pointer to member displacement info
    unsigned long attributes; // flags, generally unused
};
#pragma warning(disable : 4200)
struct BaseClassArray {
    BaseClassDescriptor* arrayOfBaseClassDescriptors[]; // describes base classes for the complete class
};
#pragma warning(default: 4200)

struct ClassHierarchyDescriptor
{
    unsigned long signature; // 1 if 64 bit, 0 if 32bit
    unsigned long attributes; // bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
    unsigned long numBaseClasses;// number of classes in pBaseClassArray
    BaseClassArray* pBaseClassArray;
};

struct CompleteObjectLocator
{
    unsigned long signature; // 1 if 64 bit, 0 if 32bit
    unsigned long offset; // offset of this vftable in the complete class
    unsigned long cdOffset; // constructor displacement offset
    TypeDescriptor* pTypeDescriptor; //  TypeDescriptor of the complete class
    ClassHierarchyDescriptor* pClassDescriptor; // describes inheritance hierarchy
};

#endif