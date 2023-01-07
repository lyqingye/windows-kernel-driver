#include "driver.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,ExpLookupHandleTableEntry)
#pragma alloc_text(PAGE,ExEnumHandleTable)
#pragma alloc_text(PAGE,ExUnlockHandleTableEntry)
#pragma alloc_text(PAGE,ExLockHandleTableEntry)

#endif

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry(
	IN PHANDLE_TABLE HandleTable,
	IN EXHANDLE tHandle
)
{
	ULONG_PTR i, j, k;
	ULONG_PTR CapturedTable;
	ULONG TableLevel;
	PHANDLE_TABLE_ENTRY Entry = NULL;
	EXHANDLE Handle;

	PUCHAR TableLevel1;
	PUCHAR TableLevel2;
	PUCHAR TableLevel3;

	ULONG_PTR MaxHandle;

	PAGED_CODE();

	Handle = tHandle;
	Handle.TagBits = 0;
	MaxHandle = *(volatile ULONG*)&HandleTable->NextHandleNeedingPool;
	if (Handle.Value >= MaxHandle) {
		return NULL;
	}

	CapturedTable = *(volatile ULONG_PTR*)&HandleTable->TableCode;

	TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);
	CapturedTable = CapturedTable - TableLevel;

	switch (TableLevel) {

	case 0:
		TableLevel1 = (PUCHAR)CapturedTable;
		Entry = (PHANDLE_TABLE_ENTRY)&TableLevel1[Handle.Value / HANDLE_VALUE_INC * sizeof(HANDLE_TABLE_ENTRY)];
		break;
	case 1:
		TableLevel2 = (PUCHAR)CapturedTable;
		i = Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);
		Handle.Value -= i;
		j = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof(PHANDLE_TABLE_ENTRY));
		TableLevel1 = (PUCHAR) * (PHANDLE_TABLE_ENTRY*)&TableLevel2[j];
		Entry = (PHANDLE_TABLE_ENTRY)&TableLevel1[i / HANDLE_VALUE_INC * sizeof(HANDLE_TABLE_ENTRY)];
		break;
	case 2:
		TableLevel3 = (PUCHAR)CapturedTable;
		i = Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);
		Handle.Value -= i;
		k = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof(PHANDLE_TABLE_ENTRY));
		j = k % (MIDLEVEL_COUNT * sizeof(PHANDLE_TABLE_ENTRY));
		k -= j;
		k /= MIDLEVEL_COUNT;
		TableLevel2 = (PUCHAR) * (PHANDLE_TABLE_ENTRY*)&TableLevel3[k];
		TableLevel1 = (PUCHAR) * (PHANDLE_TABLE_ENTRY*)&TableLevel2[j];
		Entry = (PHANDLE_TABLE_ENTRY)&TableLevel1[i * (sizeof(HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];
		break;
	default:
		_assume(0);
	}

	return Entry;
}


VOID
FORCEINLINE
ExLockHandleTableEntry(
	PHANDLE_TABLE HandleTable,
	PHANDLE_TABLE_ENTRY HandleTableEntry)
{
	LONG_PTR CurrentValue;

	PAGED_CODE();
	
	while (TRUE) {
		while (TRUE) {
			CurrentValue = ReadForWriteAccess(((volatile LONG_PTR*)HandleTableEntry));
			if ((CurrentValue & EXHANDLE_TABLE_ENTRY_LOCK_BIT) != 0) {
				if ((LONG_PTR)(InterlockedCompareExchange64((PLONG64)&HandleTableEntry->InfoTable,
					(LONG64)CurrentValue - EXHANDLE_TABLE_ENTRY_LOCK_BIT,
					(LONG64)CurrentValue)) == CurrentValue) {

					return;
				}
			}
			else {
				break;
			}
		}
		if (!CurrentValue) {
			break;
		}
	    ExpBlockOnLockedHandleEntry(&HandleTable->HandleContentionEvent, HandleTableEntry, (HANDLE)CurrentValue);
	}
}

VOID
FORCEINLINE
ExUnlockHandleTableEntry(
	__inout PHANDLE_TABLE HandleTable,
	__inout PHANDLE_TABLE_ENTRY HandleTableEntry
)
{
	PAGED_CODE();

	InterlockedExchangeAdd64((PLONG64)&HandleTableEntry->InfoTable, EXHANDLE_TABLE_ENTRY_LOCK_BIT);

	if (HandleTable->HandleContentionEvent) {
	   ExfUnblockPushLock(&HandleTable->HandleContentionEvent, 0);
	}
	return;
}


VOID
ExEnumHandleTable(
	__in PHANDLE_TABLE HandleTable,
	__in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
	__in PVOID EnumParameter
)
{
	BOOLEAN ResultValue;
	EXHANDLE LocalHandle;
	PHANDLE_TABLE_ENTRY HandleTableEntry;

	PAGED_CODE();

	ResultValue = FALSE;

	KeEnterCriticalRegion();
	for (LocalHandle.Value = HANDLE_VALUE_INC;
		(HandleTableEntry = ExpLookupHandleTableEntry(HandleTable, LocalHandle)) != NULL;
		LocalHandle.Value += HANDLE_VALUE_INC) {
		if (ExpIsValidObjectEntry(HandleTableEntry)) {
			ExLockHandleTableEntry(HandleTable, HandleTableEntry);
			if (EnumHandleProcedure != NULL) {
				ResultValue = (*EnumHandleProcedure)(HandleTableEntry,
					LocalHandle.GenericHandleOverlay,
					EnumParameter);
			}
			PVOID Object = (PVOID)((HandleTableEntry->LowValue >> 0x10) & 0xfffffffffffffff0);
			PUNICODE_STRING Name = OBJECT_TO_OBJECT_TYPE_NAME(Object);
			DbgPrint("%p %wZ \n", Object,Name );
			ExUnlockHandleTableEntry(HandleTable, HandleTableEntry);
			if (ResultValue) {
				break;
			}
		}
	}
	KeLeaveCriticalRegion();
}
