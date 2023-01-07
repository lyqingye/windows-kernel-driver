#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,InitializationGlobalContext)
#pragma alloc_text(PAGE,QueryModuleInformation)
#pragma alloc_text(PAGE,QueryNtosKrnlModuleInformation)
#endif

NTSTATUS InitializationGlobalContext(PDRIVER_OBJECT DriverObject, PVOID Buffer, SIZE_T BufferSize, PGLOBAL_CONTEXT Context) {
	NTSTATUS Status = STATUS_SUCCESS;
	if (BufferSize < sizeof(GLOBAL_CONTEXT)) {
		return STATUS_BUFFER_TOO_SMALL;
	}

	if (DriverObject == NULL || Context == NULL || Buffer == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	RtlZeroMemory(Context, sizeof(GLOBAL_CONTEXT));
	(*Context) = *(PGLOBAL_CONTEXT)Buffer;

	SYSTEM_MODULE_ENTRY NtosKrnlModuleInfo;
	Status = QueryNtosKrnlModuleInformation(DriverObject, &NtosKrnlModuleInfo);

	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	NtosKrnlBase = (ULONG_PTR)NtosKrnlModuleInfo.ImageBase;
	ExpBlockOnLockedHandleEntry = (EXP_BLOCK_ON_LOCKED_HANDLE_ENTRY)PTR_ADD_OFFSET(NtosKrnlBase, Context->pfnExpBlockOnLockedHandleEntry);
	ExfUnblockPushLock = (EXF_UNBLOCK_PUSH_LOCK)PTR_ADD_OFFSET(NtosKrnlBase, Context->pfnExfUnblockPushLock);
	ExDestroyHandle = (EX_DESTROY_HANDLE)PTR_ADD_OFFSET(NtosKrnlBase, Context->pfnExDestroyHandle);
	PspLockProcessListExclusive = (PSP_LOCK_PROCESS_LIST_EXCLUSIVE)PTR_ADD_OFFSET(NtosKrnlBase, Context->pfnPspLockProcessListExclusive);
	PspUnlockProcessListExclusive = (PSP_UNLOCK_PROCESS_LIST_EXCLUSIVE)PTR_ADD_OFFSET(NtosKrnlBase, Context->pfnPspUnlockProcessListExclusive);

	PspCidTable = (PHANDLE_TABLE*)PTR_ADD_OFFSET(NtosKrnlBase, Context->PspCidTable);
	ObHeaderCookie = (ULONG_PTR)PTR_ADD_OFFSET(NtosKrnlBase, Context->ObHeaderCookie);
	ObTypeIndexTable = (ULONG_PTR)PTR_ADD_OFFSET(NtosKrnlBase, Context->ObTypeIndexTable);
	ObpRootDirectoryObject = (ULONG_PTR)PTR_ADD_OFFSET(NtosKrnlBase, Context->ObpRootDirectoryObject);
	PsLoadedModuleList = (PLIST_ENTRY*)PTR_ADD_OFFSET(NtosKrnlBase, Context->PsLoadedModuleList);
	PsLoadedModuleResource = (PERESOURCE*)PTR_ADD_OFFSET(NtosKrnlBase, Context->PsLoadedModuleResource);
	PsActiveProcessHead = (PLIST_ENTRY*)PTR_ADD_OFFSET(NtosKrnlBase, Context->PsActiveProcessHead);

	OffsetTypeIndexOfObjectHeader = Context->OffsetTypeIndexOfObjectHeader;
	OffsetTypeInfoOfObjectType = Context->OffsetTypeInfoOfObjectType;
	OffsetTypeNameOfObjectType = Context->OffsetTypeNameOfObjectType;
	SizeOfObjectHeader = Context->SizeOfObjectHeader;
	OffsetDirectoryTableBaseOfEProcess = Context->OffsetDirectorTableBaseOfEProcess;

	return Status;
}

NTSTATUS QueryNtosKrnlModuleInformation(PDRIVER_OBJECT DriverObject, PSYSTEM_MODULE_ENTRY ModuleEntry) {
	UNICODE_STRING NtosKernelName;
	RtlInitUnicodeString(&NtosKernelName, L"ntoskrnl.exe");
	return QueryModuleInformation(DriverObject, &NtosKernelName, ModuleEntry);
}

NTSTATUS QueryModuleInformation(PDRIVER_OBJECT DriverObject, PUNICODE_STRING BaseName, PSYSTEM_MODULE_ENTRY ModuleEntry) {
	if (DriverObject == NULL || DriverObject->DriverSection == NULL || BaseName == NULL || ModuleEntry == NULL) {
		return STATUS_INVALID_PARAMETER;
	}
	RtlZeroMemory((PVOID)ModuleEntry, sizeof(SYSTEM_MODULE_ENTRY));

	PKLDR_DATA_TABLE_ENTRY pModuleList = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

	PLIST_ENTRY pListEntry = pModuleList->InLoadOrderLinks.Flink;
	while (pListEntry != NULL && pListEntry != &pModuleList->InLoadOrderLinks)
	{
		PKLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		DbgPrint("%wZ %p\n", pEntry->BaseDllName, pEntry);
		if (RtlCompareUnicodeString(&pEntry->BaseDllName, BaseName, TRUE) == 0) {
			ModuleEntry->ImageBase = pEntry->DllBase;
			ModuleEntry->ImageSize = pEntry->SizeOfImage;
			RtlCopyMemory(ModuleEntry->FullPathName, pEntry->FullDllName.Buffer, min(pEntry->FullDllName.Length, 128));
			return STATUS_SUCCESS;
		}

		pListEntry = pListEntry->Flink;
	}
	return STATUS_NOT_FOUND;
}
