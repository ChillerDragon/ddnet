#include <base/system.h>
#include <cstdint>

#include "game/generated/protocol7.h"
#include "snapshot.h"

void CSnapshotBuilder::Init7(const CSnapshot *pSnapshot)
{
	// the method is called Init7 because it is only used for 0.7 support
	// but the snap we are building is a 0.6 snap
	m_Sixup = false;

	if(pSnapshot->m_DataSize + sizeof(CSnapshot) + pSnapshot->m_NumItems * sizeof(int) * 2 > CSnapshot::MAX_SIZE || pSnapshot->m_NumItems > CSnapshot::MAX_ITEMS)
	{
		// key and offset per item
		dbg_assert(m_DataSize + sizeof(CSnapshot) + m_NumItems * sizeof(int) * 2 < CSnapshot::MAX_SIZE, "too much data");
		dbg_assert(m_NumItems < CSnapshot::MAX_ITEMS, "too many items");
		dbg_msg("sixup", "demo recording failed on invalid snapshot");
		m_DataSize = 0;
		m_NumItems = 0;
		return;
	}

	m_DataSize = pSnapshot->m_DataSize;
	m_NumItems = pSnapshot->m_NumItems;
	mem_copy(m_aOffsets, pSnapshot->Offsets(), sizeof(int) * m_NumItems);
	mem_copy(m_aData, pSnapshot->DataStart(), m_DataSize);


	unsigned char aDbgSnap[CSnapshot::MAX_SIZE];
	CSnapshot *pDbgSnap = (CSnapshot *)aDbgSnap;
	Finish(pDbgSnap);

	bool Match = false;
	for(int i = 0; i < m_NumItems; i++)
	{
		const CSnapshotItem *pDstItem = pDbgSnap->GetItem(i);
		if(pDstItem->Type() == protocol7::NETEVENTTYPE_SOUNDWORLD)
		{
			Match = true;
			break;
		}
	}

	if(!Match)
		return;

	dbg_msg("snap", "num_items=%d", m_NumItems);
	for(int i = 0; i < m_NumItems; i++)
	{
		const CSnapshotItem *pDstItem = pDbgSnap->GetItem(i);

		dbg_msg("snap", " ");
		dbg_msg("snap", "  dst->Type=%d", pDstItem->Type());
		dbg_msg("snap", "  src->GetItemSize(%d)=%d", i, pSnapshot->GetItemSize(i));
		dbg_msg("snap", "  dst->GetItemSize(%d)=%d", i, pDbgSnap->GetItemSize(i));
		dbg_msg("snap", "  offset[%d]=%d", i, m_aOffsets[i]);


		if(i == m_NumItems -1)
		{
			dbg_msg("snap", "  (last) size=%lu", (m_DataSize - m_aOffsets[i]) - sizeof(CSnapshotItem));
		}
		else
		{
			dbg_msg("snap", "  size=%lu", (m_aOffsets[i + 1] - m_aOffsets[i]) - sizeof(CSnapshotItem));
		}
	}

	pDbgSnap->DebugDump();
}
