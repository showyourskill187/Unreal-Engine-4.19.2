// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "OnlineAsyncTaskManagerNull.h"

void FOnlineAsyncTaskManagerNull::OnlineTick()
{
	check(NullSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}

