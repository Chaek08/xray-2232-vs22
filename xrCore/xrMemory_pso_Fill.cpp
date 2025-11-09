#include "stdafx.h"
#pragma hdrstop

void		xrMemFill_x86	(void* dest,  int value, u32 count)
{
	memset	(dest,int(value),count);
}
