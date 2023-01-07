#pragma once

#ifndef OB
#define OB

EXTERN_C
FORCEINLINE
POBJECT_TYPE
OBJECT_TO_OBJECT_TYPE(
	IN PVOID Object
);

EXTERN_C
FORCEINLINE
PUNICODE_STRING
OBJECT_TYPE_TO_TYPE_NAME(
	IN POBJECT_TYPE ObjectType
);

EXTERN_C
FORCEINLINE
PUNICODE_STRING
OBJECT_TO_OBJECT_TYPE_NAME(
	IN PVOID Object
);

EXTERN_C
FORCEINLINE
PVOID
OBJECT_TYPE_TO_TYPE_INFO(
	IN POBJECT_TYPE ObjectType
);

#endif // !OB