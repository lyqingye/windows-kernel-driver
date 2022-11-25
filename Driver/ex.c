#include "driver.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,ExpLookupHandleTableEntry)
#pragma alloc_text(PAGE,ExEnumHandleTable)
#pragma alloc_text(PAGE,ExUnlockHandleTableEntry)
#pragma alloc_text(PAGE,ExpLockHandleTableEntry)

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


BOOLEAN
FORCEINLINE
ExpLockHandleTableEntry(
	PHANDLE_TABLE HandleTable,
	PHANDLE_TABLE_ENTRY HandleTableEntry)
{
	LONG_PTR NewValue;
	LONG_PTR CurrentValue;

	PAGED_CODE();

	while (TRUE) {
		while (TRUE) {
			CurrentValue = ReadForWriteAccess(((volatile LONG_PTR*)&HandleTableEntry->InfoTable));
			if (CurrentValue & EXHANDLE_TABLE_ENTRY_LOCK_BIT) {
				NewValue = CurrentValue - EXHANDLE_TABLE_ENTRY_LOCK_BIT;

				if ((LONG_PTR)(InterlockedCompareExchangePointer(&HandleTableEntry->InfoTable,
					(PVOID)NewValue,
					(PVOID)CurrentValue)) == CurrentValue) {

					return TRUE;
				}
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

	InterlockedExchangeAdd64((PLONGLONG)&HandleTableEntry->InfoTable, EXHANDLE_TABLE_ENTRY_LOCK_BIT);

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

	for (LocalHandle.Value = 0;
		(HandleTableEntry = ExpLookupHandleTableEntry(HandleTable, LocalHandle)) != NULL;
		LocalHandle.Value += HANDLE_VALUE_INC) {
		if (ExpIsValidObjectEntry(HandleTableEntry)) {
			if (ExpLockHandleTableEntry(HandleTable, HandleTableEntry)) {
				if (EnumHandleProcedure != NULL) {
					ResultValue = (*EnumHandleProcedure)(HandleTableEntry,
						LocalHandle.GenericHandleOverlay,
						EnumParameter);
				}
				ULONGLONG theHandle = (HandleTableEntry->LowValue >> 0x10) & 0xfffffffffffffff0;
				DbgPrint("%p\n", theHandle);
				ExUnlockHandleTableEntry(HandleTable, HandleTableEntry);
				if (ResultValue) {
					break;
				}
			}
		}
	}
	KeLeaveCriticalRegion();
}
