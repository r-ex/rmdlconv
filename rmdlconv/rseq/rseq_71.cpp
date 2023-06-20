// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include "mdl/studio.h"
#include "versions.h"

/*
	Type:    RSEQ
	Version: 7.1
	Game:    Apex Legends Seasons 7-8

	Files: .rseq
*/

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

#define FILEBUFSIZE (32 * 1024 * 1024)

//
// ConvertRSEQFrom71To7
// Purpose: converts rseq data from version 7.1 (seasons 7-8) to version 7 (seasons 0-6)
//
void ConvertRSEQFrom71To7(char* buf, char* externalbuf, const std::string& filePath)
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
	ConvertSequenceAnims((char*)oldSeqDesc, externalbuf, (char*)pSeq, (int*)input.getPtr(), pSeq->groupsize[0] * pSeq->groupsize[1], numBones);

	//this goes last
	input.seek(oldSeqDesc->unkOffset, rseekdir::beg);
	g_model.seqV7()->unkOffset = ConvertSequenceUnknown((r5::unkseqdata_t*)input.getPtr(), pSeq->unkCount);

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN4(g_model.pData);

	out.write(g_model.pBase, g_model.pData - g_model.pBase);

	delete[] g_model.pBase;

	printf("Done!\n");
}