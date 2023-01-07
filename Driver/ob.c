#include "driver.h"

FORCEINLINE
POBJECT_TYPE
OBJECT_TO_OBJECT_TYPE(
	IN PVOID Object
)
{
	ASSERT(Object != NULL);
	UINT8 Cookie = *(PUINT8)(ObHeaderCookie);
	ULONG_PTR ObjectHeader = ((ULONG_PTR)Object) - SizeOfObjectHeader + sizeof(PVOID);
	UINT8 TypeIndex = *(((PUINT8)ObjectHeader) + OffsetTypeIndexOfObjectHeader);
	UINT8 Index = Cookie ^ (UINT8)(ObjectHeader >> 8) ^ TypeIndex;
	return (POBJECT_TYPE)(((PULONG_PTR)ObTypeIndexTable)[Index]);
}

FORCEINLINE
PUNICODE_STRING
OBJECT_TYPE_TO_TYPE_NAME(
	IN POBJECT_TYPE ObjectType
)
{
	return (PUNICODE_STRING)PTR_ADD_OFFSET(ObjectType, OffsetTypeNameOfObjectType);
}


FORCEINLINE
PUNICODE_STRING
OBJECT_TO_OBJECT_TYPE_NAME(
	IN PVOID Object
)
{
	return OBJECT_TYPE_TO_TYPE_NAME(OBJECT_TO_OBJECT_TYPE(Object));
}

FORCEINLINE
PVOID
OBJECT_TYPE_TO_TYPE_INFO(
	IN POBJECT_TYPE ObjectType
)
{
	return PTR_ADD_OFFSET(ObjectType, OffsetTypeInfoOfObjectType);
}


