/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// This file was generated by NxParameterized/scripts/GenParameterized.pl
// Created: 2015.06.02 04:11:52

#include "CachedOverlaps.h"
#include <string.h>
#include <stdlib.h>

using namespace NxParameterized;

namespace physx
{
namespace apex
{
namespace destructible
{

using namespace CachedOverlapsNS;

const char* const CachedOverlapsFactory::vptr =
    NxParameterized::getVptr<CachedOverlaps, CachedOverlaps::ClassAlignment>();

const physx::PxU32 NumParamDefs = 6;
static NxParameterized::DefinitionImpl* ParamDefTable; // now allocated in buildTree [NumParamDefs];


static const size_t ParamLookupChildrenTable[] =
{
	1, 2, 3, 4, 5,
};

#define TENUM(type) physx::##type
#define CHILDREN(index) &ParamLookupChildrenTable[index]
static const NxParameterized::ParamLookupNode ParamLookupTable[NumParamDefs] =
{
	{ TYPE_STRUCT, false, 0, CHILDREN(0), 2 },
	{ TYPE_BOOL, false, (size_t)(&((ParametersStruct*)0)->isCached), NULL, 0 }, // isCached
	{ TYPE_ARRAY, true, (size_t)(&((ParametersStruct*)0)->overlaps), CHILDREN(2), 1 }, // overlaps
	{ TYPE_STRUCT, false, 1 * sizeof(IntPair_Type), CHILDREN(3), 2 }, // overlaps[]
	{ TYPE_I32, false, (size_t)(&((IntPair_Type*)0)->i0), NULL, 0 }, // overlaps[].i0
	{ TYPE_I32, false, (size_t)(&((IntPair_Type*)0)->i1), NULL, 0 }, // overlaps[].i1
};


bool CachedOverlaps::mBuiltFlag = false;
NxParameterized::MutexType CachedOverlaps::mBuiltFlagMutex;

CachedOverlaps::CachedOverlaps(NxParameterized::Traits* traits, void* buf, PxI32* refCount) :
	NxParameters(traits, buf, refCount)
{
	//mParameterizedTraits->registerFactory(className(), &CachedOverlapsFactoryInst);

	if (!buf) //Do not init data if it is inplace-deserialized
	{
		initDynamicArrays();
		initStrings();
		initReferences();
		initDefaults();
	}
}

CachedOverlaps::~CachedOverlaps()
{
	freeStrings();
	freeReferences();
	freeDynamicArrays();
}

void CachedOverlaps::destroy()
{
	// We cache these fields here to avoid overwrite in destructor
	bool doDeallocateSelf = mDoDeallocateSelf;
	NxParameterized::Traits* traits = mParameterizedTraits;
	physx::PxI32* refCount = mRefCount;
	void* buf = mBuffer;

	this->~CachedOverlaps();

	NxParameters::destroy(this, traits, doDeallocateSelf, refCount, buf);
}

const NxParameterized::DefinitionImpl* CachedOverlaps::getParameterDefinitionTree(void)
{
	if (!mBuiltFlag) // Double-checked lock
	{
		NxParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);
		if (!mBuiltFlag)
		{
			buildTree();
		}
	}

	return(&ParamDefTable[0]);
}

const NxParameterized::DefinitionImpl* CachedOverlaps::getParameterDefinitionTree(void) const
{
	CachedOverlaps* tmpParam = const_cast<CachedOverlaps*>(this);

	if (!mBuiltFlag) // Double-checked lock
	{
		NxParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);
		if (!mBuiltFlag)
		{
			tmpParam->buildTree();
		}
	}

	return(&ParamDefTable[0]);
}

NxParameterized::ErrorType CachedOverlaps::getParameterHandle(const char* long_name, Handle& handle) const
{
	ErrorType Ret = NxParameters::getParameterHandle(long_name, handle);
	if (Ret != ERROR_NONE)
	{
		return(Ret);
	}

	size_t offset;
	void* ptr;

	getVarPtr(handle, ptr, offset);

	if (ptr == NULL)
	{
		return(ERROR_INDEX_OUT_OF_RANGE);
	}

	return(ERROR_NONE);
}

NxParameterized::ErrorType CachedOverlaps::getParameterHandle(const char* long_name, Handle& handle)
{
	ErrorType Ret = NxParameters::getParameterHandle(long_name, handle);
	if (Ret != ERROR_NONE)
	{
		return(Ret);
	}

	size_t offset;
	void* ptr;

	getVarPtr(handle, ptr, offset);

	if (ptr == NULL)
	{
		return(ERROR_INDEX_OUT_OF_RANGE);
	}

	return(ERROR_NONE);
}

void CachedOverlaps::getVarPtr(const Handle& handle, void*& ptr, size_t& offset) const
{
	ptr = getVarPtrHelper(&ParamLookupTable[0], const_cast<CachedOverlaps::ParametersStruct*>(&parameters()), handle, offset);
}


/* Dynamic Handle Indices */

void CachedOverlaps::freeParameterDefinitionTable(NxParameterized::Traits* traits)
{
	if (!traits)
	{
		return;
	}

	if (!mBuiltFlag) // Double-checked lock
	{
		return;
	}

	NxParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);

	if (!mBuiltFlag)
	{
		return;
	}

	for (physx::PxU32 i = 0; i < NumParamDefs; ++i)
	{
		ParamDefTable[i].~DefinitionImpl();
	}

	traits->free(ParamDefTable);

	mBuiltFlag = false;
}

#define PDEF_PTR(index) (&ParamDefTable[index])

void CachedOverlaps::buildTree(void)
{

	physx::PxU32 allocSize = sizeof(NxParameterized::DefinitionImpl) * NumParamDefs;
	ParamDefTable = (NxParameterized::DefinitionImpl*)(mParameterizedTraits->alloc(allocSize));
	memset(static_cast<void*>(ParamDefTable), 0, allocSize);

	for (physx::PxU32 i = 0; i < NumParamDefs; ++i)
	{
		NX_PARAM_PLACEMENT_NEW(ParamDefTable + i, NxParameterized::DefinitionImpl)(*mParameterizedTraits);
	}

	// Initialize DefinitionImpl node: nodeIndex=0, longName=""
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[0];
		ParamDef->init("", TYPE_STRUCT, "STRUCT", true);






	}

	// Initialize DefinitionImpl node: nodeIndex=1, longName="isCached"
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[1];
		ParamDef->init("isCached", TYPE_BOOL, NULL, true);

#ifdef NX_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[2];
		static Hint* HintPtrTable[2] = { &HintTable[0], &HintTable[1], };
		HintTable[0].init("longDescription", "Whether or not chunk overlaps are cached at a particular hierarchy depth.  In this\n					case, overlap really means adjacency.", true);
		HintTable[1].init("shortDescription", "Whether or not chunk overlaps are cached at a particular hierarchy depth", true);
		ParamDefTable[1].setHints((const NxParameterized::Hint**)HintPtrTable, 2);

#endif /* NX_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=2, longName="overlaps"
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[2];
		ParamDef->init("overlaps", TYPE_ARRAY, NULL, true);

#ifdef NX_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[2];
		static Hint* HintPtrTable[2] = { &HintTable[0], &HintTable[1], };
		HintTable[0].init("longDescription", "The overlaps at a particular hierarchy depth.  This is the set of adjacencies,\n					stored as pairs of chunk indices, for chunks at a the given hierarchy depth.", true);
		HintTable[1].init("shortDescription", "The overlaps at a particular hierarchy depth", true);
		ParamDefTable[2].setHints((const NxParameterized::Hint**)HintPtrTable, 2);

#endif /* NX_PARAMETERIZED_HIDE_DESCRIPTIONS */




		ParamDef->setArraySize(-1);
	}

	// Initialize DefinitionImpl node: nodeIndex=3, longName="overlaps[]"
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[3];
		ParamDef->init("overlaps", TYPE_STRUCT, "IntPair", true);

#ifdef NX_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[2];
		static Hint* HintPtrTable[2] = { &HintTable[0], &HintTable[1], };
		HintTable[0].init("longDescription", "The overlaps at a particular hierarchy depth.  This is the set of adjacencies,\n					stored as pairs of chunk indices, for chunks at a the given hierarchy depth.", true);
		HintTable[1].init("shortDescription", "The overlaps at a particular hierarchy depth", true);
		ParamDefTable[3].setHints((const NxParameterized::Hint**)HintPtrTable, 2);

#endif /* NX_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=4, longName="overlaps[].i0"
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[4];
		ParamDef->init("i0", TYPE_I32, NULL, true);

#ifdef NX_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "First element", true);
		ParamDefTable[4].setHints((const NxParameterized::Hint**)HintPtrTable, 1);

#endif /* NX_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=5, longName="overlaps[].i1"
	{
		NxParameterized::DefinitionImpl* ParamDef = &ParamDefTable[5];
		ParamDef->init("i1", TYPE_I32, NULL, true);

#ifdef NX_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "Second element", true);
		ParamDefTable[5].setHints((const NxParameterized::Hint**)HintPtrTable, 1);

#endif /* NX_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// SetChildren for: nodeIndex=0, longName=""
	{
		static Definition* Children[2];
		Children[0] = PDEF_PTR(1);
		Children[1] = PDEF_PTR(2);

		ParamDefTable[0].setChildren(Children, 2);
	}

	// SetChildren for: nodeIndex=2, longName="overlaps"
	{
		static Definition* Children[1];
		Children[0] = PDEF_PTR(3);

		ParamDefTable[2].setChildren(Children, 1);
	}

	// SetChildren for: nodeIndex=3, longName="overlaps[]"
	{
		static Definition* Children[2];
		Children[0] = PDEF_PTR(4);
		Children[1] = PDEF_PTR(5);

		ParamDefTable[3].setChildren(Children, 2);
	}

	mBuiltFlag = true;

}
void CachedOverlaps::initStrings(void)
{
}

void CachedOverlaps::initDynamicArrays(void)
{
	overlaps.buf = NULL;
	overlaps.isAllocated = true;
	overlaps.elementSize = sizeof(IntPair_Type);
	overlaps.arraySizes[0] = 0;
}

void CachedOverlaps::initDefaults(void)
{

	freeStrings();
	freeReferences();
	freeDynamicArrays();
	isCached = bool(false);

	initDynamicArrays();
	initStrings();
	initReferences();
}

void CachedOverlaps::initReferences(void)
{
}

void CachedOverlaps::freeDynamicArrays(void)
{
	if (overlaps.isAllocated && overlaps.buf)
	{
		mParameterizedTraits->free(overlaps.buf);
	}
}

void CachedOverlaps::freeStrings(void)
{
}

void CachedOverlaps::freeReferences(void)
{
}

} // namespace destructible
} // namespace apex
} // namespace physx