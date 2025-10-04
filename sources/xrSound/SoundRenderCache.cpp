#include "stdafx.h"
#include "SoundRenderCache.h"

SoundRenderCache::SoundRenderCache() : 
    m_data(nullptr),
    m_totalSize(0),
    m_lineSize(0),
    m_linesCount(0),
    m_statsHit(0),
    m_statsMiss(0),
    m_storage(nullptr),
    m_frontLine(nullptr),
    m_backLine(nullptr)
{
}

void* SoundRenderCache::GetDataById(CacheTable& table, uint32_t id)
{
    id %= table.Size;
    return m_storage[table.Lines[id]].Data;
}

uint32_t SoundRenderCache::TotalSize()
{
    return m_totalSize;
}

uint32_t SoundRenderCache::LineSize()
{
    return m_lineSize;
}

uint32_t SoundRenderCache::LinesCount()
{
    return m_linesCount;
}

uint32_t SoundRenderCache::StatsHits()
{
    return m_statsHit;
}

uint32_t SoundRenderCache::StatsMiss()
{
    return m_statsMiss;
}

void SoundRenderCache::Initialize(uint32_t totalSize, uint32_t bytesPerLine)
{
    // use twice the requisted memory (to avoid bad configs)
    totalSize *= 2;

    // calc
    m_lineSize = bytesPerLine;
    m_linesCount = ((totalSize * 1024) / bytesPerLine + 1);
    m_totalSize = m_linesCount * m_lineSize;

    R_ASSERT(m_linesCount < CAT_FREE);
    Msg("Sound cache: %d kb, %d lines, %d bpl", m_totalSize / 1024, m_linesCount, m_lineSize);

    // alloc structs
    m_data = xr_alloc<uint8_t>(m_totalSize);
    m_storage = xr_alloc<CacheLine>(m_linesCount);

    // format
    Format();
}

void SoundRenderCache::Destroy()
{
    Disconnect();
    xr_free(m_data);
    xr_free(m_storage);

    m_frontLine = nullptr;
    m_backLine = nullptr;
    m_totalSize = 0;
    m_lineSize = 0;
    m_linesCount = 0;
}

void SoundRenderCache::CreateTable(CacheTable& table, uint32_t size)
{
    table.Size = size / m_lineSize;
    if (size % m_lineSize)
    {
        table.Size += 1;
    }

    uint32_t allocSize = (table.Size & 1) 
        ? table.Size + 1 
        : table.Size;

    table.Lines = xr_alloc<uint16_t>(allocSize);
    memset(table.Lines, 0xff, allocSize * sizeof uint16_t); // fill32
}

void SoundRenderCache::DestroyTable(CacheTable& table)
{
    xr_free(table.Lines);
    table.Size = 0;
}

bool SoundRenderCache::Request(CacheTable& table, uint32_t id)
{
    // 1. check if cached version available
    id %= table.Size;
    uint16_t& ptr = table.Lines[id];

    if (CAT_FREE != ptr)
    {
        // cache line exists - change it's priority and return
        m_statsHit++;
        CacheLine* line = m_storage + ptr;
        MoveToTop(line);
        return false;
    }

    // 2. purge oldest item + move it to top
    m_statsMiss++;
    MoveToTop(m_backLine);

    if (m_frontLine->Loopback)
    {
        *m_frontLine->Loopback = CAT_FREE;
        m_frontLine->Loopback = nullptr;
    }

    // 3. associate
    ptr = m_frontLine->Id;
    m_frontLine->Loopback = &ptr;

    // 4. fill with data
    return true;
}

void SoundRenderCache::Purge()
{
    Disconnect();
    Format();
}

void SoundRenderCache::Disconnect()
{
    // disconnect from CATs
    for (uint32_t it = 0; it < m_linesCount; it++)
    {
        CacheLine* line = m_storage + it;
        if (line->Loopback)
        {
            *line->Loopback = CAT_FREE;
            line->Loopback = nullptr;
        }
    }
}

void SoundRenderCache::Format()
{
    // format structs
    for (uint32_t it = 0; it < m_linesCount; it++)
    {
        CacheLine* line = m_storage + it;        

        line->Previous = (0 == it) 
            ? nullptr 
            : m_storage + it - 1;

        line->Next = ((m_linesCount - 1) == it) 
            ? nullptr 
            : m_storage + it + 1;

        line->Data = m_data + it * m_lineSize;
        line->Loopback = nullptr;
        line->Id = uint16_t(it);
    }

    // start-end
    m_frontLine = m_storage + 0;
    m_backLine = m_storage + m_linesCount - 1;
}

void SoundRenderCache::MoveToTop(CacheLine* line)
{
    VERIFY(line);

    if (line == m_frontLine)
    {
        // already at top
        return;
    }

    // track end
    if (line == m_backLine)
    {
        m_backLine = m_backLine->Previous;
    }

    // cut
    CacheLine* previous = line->Previous;
    CacheLine* next = line->Next;

    if (previous)
    {
        previous->Next = next;
    }

    if (next)
    {
        next->Previous = previous;
    }

    // register at top
    line->Previous = nullptr;
    line->Next = m_frontLine;

    // track begin
    m_frontLine->Previous = line;
    m_frontLine = line;

    // internal verify
    VERIFY(m_frontLine->Previous == nullptr);
    VERIFY(m_backLine->Next == nullptr);
}

void SoundRenderCache::ClearStatistic()
{
    m_statsHit = 0;
    m_statsMiss = 0;
}
