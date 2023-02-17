// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "versions.h"

/*
	Type:    RSEQ
	Version: 7.1
	Game:    Apex Legends Seasons 7-8

	Files: .rseq
*/

//
// ConvertStudioHdr
// Purpose: transfers data from the existing mstudioseqdesc_t struct to the new one (seasons 0-14)
//
void ConvertSequenceHdr(r5::v8::mstudioseqdesc_t* out, r5::v8::mstudioseqdesc_t* oldDesc)
{
	out->baseptr = oldDesc->baseptr; // should alwalts be start of file

	out->flags = oldDesc->flags;
	out->actweight = oldDesc->actweight;
	out->numevents = oldDesc->numevents;

	out->bbmin = oldDesc->bbmin;
	out->bbmax = oldDesc->bbmax;

	out->numblends = oldDesc->numblends;

	out->groupsize[0] = oldDesc->groupsize[0];
	out->groupsize[1] = oldDesc->groupsize[1];
	out->paramindex[0] = oldDesc->paramindex[0];
	out->paramindex[1] = oldDesc->paramindex[1];
	out->paramstart[0] = oldDesc->paramstart[0];
	out->paramstart[1] = oldDesc->paramstart[1];
	out->paramend[0] = oldDesc->paramend[0];
	out->paramend[1] = oldDesc->paramend[1];
	out->paramparent = oldDesc->paramparent;

	out->fadeintime = oldDesc->fadeintime;
	out->fadeouttime = oldDesc->fadeouttime;

	out->localentrynode = oldDesc->localentrynode;
	out->localexitnode = oldDesc->localexitnode;
	out->nodeflags = oldDesc->nodeflags;

	out->entryphase = oldDesc->entryphase;
	out->exitphase = oldDesc->exitphase;

	out->lastframe = oldDesc->lastframe;

	out->nextseq = oldDesc->nextseq;
	out->pose = oldDesc->pose;

	out->numikrules = oldDesc->numikrules;
	out->numautolayers = oldDesc->numautolayers;
	out->numiklocks = oldDesc->numiklocks;

	//out->keyvaluesize = oldDesc.keyvaluesize; // honestly unsure how this functions overall and if it exists in respawn made sequences to begin with

	out->numactivitymodifiers = oldDesc->numactivitymodifiers;

	out->ikResetMask = oldDesc->ikResetMask;
	out->unk1 = oldDesc->unk1;

	out->unkcount = oldDesc->unkcount;
}

//
// ConvertSequenceEvents
// Purpose: transfers data from the existing mstudioevent_t struct to the new one (seasons 0-8)
//
int ConvertSequenceEvents(r5::v8::mstudioevent_t* pOldEvents, int numEvents)
{
	printf("converting %i events...\n", numEvents);

	int index = g_model.pData - g_model.pBase;

	for (int eventIdx = 0; eventIdx < numEvents; eventIdx++)
	{
		r5::v8::mstudioevent_t* oldEvent = &pOldEvents[eventIdx];
		r5::v8::mstudioevent_t* newEvent = reinterpret_cast<r5::v8::mstudioevent_t*>(g_model.pData);

		newEvent->cycle = oldEvent->cycle;
		newEvent->event = oldEvent->event;
		newEvent->type = oldEvent->type;

		memcpy_s(&newEvent->options, 256, &oldEvent->options, 256);

		AddToStringTable((char*)newEvent, &newEvent->szeventindex, STRING_FROM_IDX(oldEvent, oldEvent->szeventindex));

		g_model.pData += sizeof(r5::v8::mstudioevent_t);
	}

	ALIGN4(g_model.pData);

	return index;
}

//
// ConvertSequenceActMods
// Purpose: transfers data from the existing mstudioactivitymodifier_t struct to the new one (seasons 0-14)
//
int ConvertSequenceActMods(r1::mstudioactivitymodifier_t* pOldActMods, int numActMods)
{
	printf("converting %i activitymodifiers...\n", numActMods);

	int index = g_model.pData - g_model.pBase;

	for (int actModIdx = 0; actModIdx < numActMods; actModIdx++)
	{
		r1::mstudioactivitymodifier_t* oldActivityModifier = &pOldActMods[actModIdx];
		r1::mstudioactivitymodifier_t* newActivityModifier = reinterpret_cast<r1::mstudioactivitymodifier_t*>(g_model.pData);

		AddToStringTable((char*)newActivityModifier, &newActivityModifier->sznameindex, STRING_FROM_IDX(oldActivityModifier, oldActivityModifier->sznameindex));
		newActivityModifier->negate = oldActivityModifier->negate;

		g_model.pData += sizeof(r1::mstudioactivitymodifier_t);

		ALIGN4(g_model.pData);
	}

	return index;
}

//
// ConvertSequenceUnknown
// Purpose: transfer data from unkseqdata_t if used
//
int ConvertSequenceUnknown(r5::v8::unkseqdata_t* pOldUnknown, int numUnknown)
{
	printf("converting %i unknown data...\n", numUnknown);

	int index = g_model.pData - g_model.pBase;

	for (int unkIdx = 0; unkIdx < numUnknown; unkIdx++)
	{
		r5::v8::unkseqdata_t* oldUnknown = &pOldUnknown[unkIdx];
		r5::v8::unkseqdata_t* newUnknown = reinterpret_cast<r5::v8::unkseqdata_t*>(g_model.pData);

		newUnknown->unkfloat = oldUnknown->unkfloat;
		newUnknown->unk = oldUnknown->unk;
		newUnknown->unkfloat1 = oldUnknown->unkfloat1;
		newUnknown->unkfloat2 = oldUnknown->unkfloat2;
		newUnknown->unkfloat3 = oldUnknown->unkfloat3;
		newUnknown->unkfloat4 = oldUnknown->unkfloat4;

		g_model.pData += sizeof(r5::v8::unkseqdata_t);
	}

	return index;
}

int ConvertAnimation(char* pOldAnimIndex, r5::v8::mstudioanimdesc_t* pNewAnimDesc, int numBones)
{
	int index = g_model.pData - (char*)pNewAnimDesc;

	if ((pNewAnimDesc->flags & STUDIO_ALLZEROS) || !(pNewAnimDesc->flags & STUDIO_ANIM_UNK))
		return index;

	memcpy(g_model.pData, pOldAnimIndex, numBones / 2);
	g_model.pData += numBones / 2;

	ALIGN2(g_model.pData);

	return index;
}

//
// ConvertSequenceAnims
// Purpose: converts a rseq v7.1 animation to a rseq v7 animation
//
void ConvertSequenceAnims(char* pOldSeq, char* pNewSeq, int* pOldAnimIndexes, int numAnims, int numBones)
{
	char* pAnimIndexStart = g_model.pData;

	std::vector<int*> animIndexes;

	printf("converting %i animations...\n", numAnims);

	for (int currentAnimIdx = 0; currentAnimIdx < numAnims; currentAnimIdx++)
	{
		int* animIndex = reinterpret_cast<int*>(g_model.pData);
		g_model.pData += sizeof(int);

		animIndexes.push_back(animIndex);
	}

	for (auto animIndex : animIndexes)
	{
		int currentAnimIdx = ((char*)animIndex - pAnimIndexStart) / sizeof(int);

		r5::v121::mstudioanimdesc_t* oldAnimDesc = PTR_FROM_IDX(r5::v121::mstudioanimdesc_t, pOldSeq, pOldAnimIndexes[currentAnimIdx]);
		r5::v8::mstudioanimdesc_t* newAnimDesc = reinterpret_cast<r5::v8::mstudioanimdesc_t*>(g_model.pData);

		*animIndex = (char*)newAnimDesc - (char*)pNewSeq;

		newAnimDesc->baseptr = oldAnimDesc->baseptr;

		AddToStringTable((char*)newAnimDesc, &newAnimDesc->sznameindex, STRING_FROM_IDX(oldAnimDesc, oldAnimDesc->sznameindex));

		newAnimDesc->fps = oldAnimDesc->fps;
		newAnimDesc->flags = oldAnimDesc->flags;
		newAnimDesc->numframes = oldAnimDesc->numframes;
		newAnimDesc->nummovements = oldAnimDesc->nummovements;
		newAnimDesc->numikrules = oldAnimDesc->numikrules;
		newAnimDesc->sectionframes = oldAnimDesc->sectionframes;
		
		g_model.pData += sizeof(r5::v8::mstudioanimdesc_t);


		if (newAnimDesc->sectionframes)
		{
			Error("incomplete");
		}
		else
		{
			ALIGN16(g_model.pData);
			newAnimDesc->animindex = ConvertAnimation(PTR_FROM_IDX(char, oldAnimDesc, oldAnimDesc->animindex), newAnimDesc, numBones);
		}
	}
}

#define FILEBUFSIZE (32 * 1024 * 1024)

//
// ConvertRSEQFrom71To7
// Purpose: converts rseq data from version 7.1 (seasons 7-8) to version 7 (seasons 0-6)
//
void ConvertRSEQFrom71To7(char* buf, const std::string& filePath)
{
	TIME_SCOPE(__FUNCTION__);

	rmem input(buf);

	r5::v8::mstudioseqdesc_t* oldSeqDesc = input.get<r5::v8::mstudioseqdesc_t>();

	std::string newSeqPath = ChangeExtension(filePath, "rseq_conv");
	std::ofstream out(newSeqPath, std::ios::out | std::ios::binary);

	int numBones = (oldSeqDesc->activitymodifierindex - oldSeqDesc->weightlistindex) / 4;

	// allocate temp file buffer
	g_model.pBase = new char[FILEBUFSIZE] {};
	g_model.pData = g_model.pBase;

	// convert mdl hdr
	r5::v8::mstudioseqdesc_t* pSeq = reinterpret_cast<r5::v8::mstudioseqdesc_t*>(g_model.pData);
	ConvertSequenceHdr(pSeq, oldSeqDesc);
	g_model.pHdr = pSeq;
	g_model.pData += sizeof(r5::v8::mstudioseqdesc_t);

	// init string table so we can use 
	BeginStringTable();

	AddToStringTable((char*)pSeq, &pSeq->szlabelindex, STRING_FROM_IDX(buf, oldSeqDesc->szlabelindex));
	AddToStringTable((char*)pSeq, &pSeq->szactivitynameindex, STRING_FROM_IDX(buf, oldSeqDesc->szactivitynameindex));

	// copy these over directly because there is not really a struct.
	if (oldSeqDesc->posekeyindex)
	{
		printf("copying posekeys...\n");
		pSeq->posekeyindex = g_model.pData - g_model.pBase;
		memcpy(g_model.pData, PTR_FROM_IDX(r5::v8::mstudioseqdesc_t, oldSeqDesc, oldSeqDesc->posekeyindex), (pSeq->groupsize[0] + pSeq->groupsize[1]) * sizeof(float)); // doesn't copy correctly??
		g_model.pData += (pSeq->groupsize[0] + pSeq->groupsize[1]) * sizeof(float);
	}

	// copy over data from old events, nothing changed between 7.1 and 7 here.
	input.seek(oldSeqDesc->eventindex, rseekdir::beg);
	g_model.seqV7()->eventindex = ConvertSequenceEvents((r5::v8::mstudioevent_t*)input.getPtr(), oldSeqDesc->numevents);

	// no strings so copy directly.
	printf("copying %i autolayers...\n", pSeq->numautolayers);
	pSeq->autolayerindex = g_model.pData - g_model.pBase;
	memcpy(g_model.pData, PTR_FROM_IDX(r5::v8::mstudioseqdesc_t, oldSeqDesc, oldSeqDesc->autolayerindex), pSeq->numautolayers * sizeof(r5::v8::mstudioautolayer_t)); // doesn't copy correctly??
	g_model.pData += pSeq->numautolayers * sizeof(r5::v8::mstudioautolayer_t);

	// copy float data, no structs again.
	printf("copying weightlist...\n");
	pSeq->weightlistindex = g_model.pData - g_model.pBase;
	memcpy(g_model.pData, PTR_FROM_IDX(r5::v8::mstudioseqdesc_t, oldSeqDesc, oldSeqDesc->weightlistindex), (numBones * sizeof(float))); // doesn't copy correctly??
	g_model.pData += numBones * sizeof(float);

	// these remain unchanged until rseq v11
	input.seek(oldSeqDesc->activitymodifierindex, rseekdir::beg);
	g_model.seqV7()->activitymodifierindex = ConvertSequenceActMods((r1::mstudioactivitymodifier_t*)input.getPtr(), oldSeqDesc->numactivitymodifiers);
	ALIGN4(g_model.pData);

	pSeq->animindexindex = g_model.pData - g_model.pBase;
	input.seek(oldSeqDesc->animindexindex, rseekdir::beg);
	ConvertSequenceAnims((char*)oldSeqDesc, (char*)pSeq, (int*)input.getPtr(), pSeq->groupsize[0] * pSeq->groupsize[1], numBones);

	//this goes last
	input.seek(oldSeqDesc->unkindex, rseekdir::beg);
	g_model.seqV7()->unkindex = ConvertSequenceUnknown((r5::v8::unkseqdata_t*)input.getPtr(), pSeq->unkcount);

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN4(g_model.pData);

	out.write(g_model.pBase, g_model.pData - g_model.pBase);

	delete[] g_model.pBase;

	printf("Done!\n");
}