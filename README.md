# ClassDumper
A Remake of my original RTTIDumper tool
ClassDumper is a tool to quickly retrieve information about  virtual classes in games or other software.

# Basic Usage

You'll need to inject it into your target process
Output will be on the current users desktop (vtable.txt, inheritance.txt)

# About ClassDumper

While programs like IDA, Binary Ninja and Ghidra are good for this,  
I think its handy to have this information really fast rather than  
waiting for the binary to parse in these disassemblers.  

And By fast I mean **FAST**  
Generally it takes about a second or less to complete.

This remake completely changes how the tool works.
Instead of reverse lookup which is VERY SLOW and CPU heavy, we use the PE Headers to find .text and .rdata sections  
which can be used in combination to really quickly find all virtual function tables and class metadata

This tool targets both x32 and x64 bit software, however you'll need to inject  
the corresponding x32 or x64 bit DLL binary.

I will be making an injector specifically for this tool soon,  
So that it auto picks the right DLL for the target.

# A Deeper Look at How It Works
All the data we need is inside .rdata section  
it stores:
 - VTables
 - All RTTI structs
 - But also various other structures and pointers

So all we have to do is iterate over .rdata and find valid vtables  

We can use the bounds of the .rdata section and the .text section check if we have a valid vtable.  
In 64 bit memory is layed out like  
>  0x0: vtable-ptr ; This is the start of the object  
>  ...: Class Members..  

then inside vtable:  
> -0x8: CompleteObjectLocator* ; information about class and hierarchy  
> 0x0 : ... all the virtual member functions    

That means that the CompleteObjectLocator is stored sizeof(void*) behind the start of the vtable  
We can check that this is a real vtable by seeing if the vtable points to functions in .text  
and that the pointer behind the vtable is indeed a CompleteObjectLocator inside .rdata  

To check if the CompleteObjectLocator is valid all we need to do is test:
  - If the CompleteObjectLocator signature is either a 0 or a 1 (0 for 32bit, 1 for 64bit?)
  - pTypeDescriptor is a valid pointer
  - That TypeDescriptor.name starts with ".?AV"

After finding all vtables its just a game of parsing the various RTTI structures and vtables and dumping all the info.
