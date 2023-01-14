#ifndef PS
#define PS

typedef VOID(FASTCALL *PSP_LOCK_PROCESS_LIST_EXCLUSIVE)(
	IN PKTHREAD KThread
);

typedef VOID(FASTCALL *PSP_UNLOCK_PROCESS_LIST_EXCLUSIVE)(
	IN PKTHREAD KThread
);

NTSTATUS
RemoveSystemModuleFromLoadedModuleList(
	PUNICODE_STRING ModuleBaseName,
	PLIST_ENTRY* RemovedEntry
);

VOID
InsertModuleEntryToLoadedModuleList(
	PLIST_ENTRY* Entry
);

NTSTATUS
ReadProcessVirtualMemory(
	HANDLE ProcessId,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
);

NTSTATUS
WriteProcessVirtualMemory(
	HANDLE ProcessId,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
);

NTSTATUS
ReadVirtualMemory(
	PVOID PageTable,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
);

NTSTATUS
WriteVirtualMemory(
	PVOID PageTable,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
);

FORCEINLINE
NTSTATUS
ReadPhysicalMemory(
	PVOID PhysicalAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
);

FORCEINLINE
NTSTATUS
WritePhysicalMemory(
	PVOID PhysicalAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
);

PVOID
TranslateVirtualAddressToPhysicalAddress(
	PVOID PageTable,
	PVOID VirtualAddress
);

BOOLEAN
InitSystemPageTableInformation();
#endif // !PS
