#include "stdafx.h"
#include "soundrender_cache.h"

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

    uint32_t allocSize = (table.Size & 1) ? table.Size + 1 : table.Size;
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
    u16& ptr = table.Lines[id];

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

        line->Previous = (0 == it) ? nullptr : m_storage + it - 1;
        line->Next = ((m_linesCount - 1) == it) ? nullptr : m_storage + it + 1;
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













//CSoundRender_Cache::CSoundRender_Cache()
//{
//    data = NULL;
//    c_storage = NULL;
//    c_begin = NULL;
//    c_end = NULL;
//    _total = 0;
//    _line = 0;
//    _count = 0;
//}

//void CSoundRender_Cache::move2top(cache_line* line)
//{
//    VERIFY(line);
//    if (line == c_begin)
//        return; // already at top
//
//    // track end
//    if (line == c_end)
//        c_end = c_end->prev;
//
//    // cut
//    cache_line* prev = line->prev;
//    cache_line* next = line->next;
//    if (prev)
//        prev->next = next;
//    if (next)
//        next->prev = prev;
//
//    // register at top
//    line->prev = NULL;
//    line->next = c_begin;
//
//    // track begin
//    c_begin->prev = line;
//    c_begin = line;
//
//    // internal verify
//    VERIFY(c_begin->prev == NULL);
//    VERIFY(c_end->next == NULL);
//}

//BOOL CSoundRender_Cache::request(cache_cat& cat, u32 id)
//{
//    // 1. check if cached version available
//    id %= cat.size;
//    //.	R_ASSERT		(id<cat.size);
//    u16& cptr = cat.table[id];
//    if (CAT_FREE != cptr)
//    {
//        // cache line exists - change it's priority and return
//        _stat_hit++;
//        cache_line* L = c_storage + cptr;
//        move2top(L);
//        return FALSE;
//    }
//
//    // 2. purge oldest item + move it to top
//    _stat_miss++;
//    move2top(c_end);
//    if (c_begin->loopback)
//    {
//        *c_begin->loopback = CAT_FREE;
//        c_begin->loopback = NULL;
//    }
//
//    // 3. associate
//    cptr = c_begin->id;
//    c_begin->loopback = &cptr;
//
//    // 4. fill with data
//    return TRUE;
//}

//void CSoundRender_Cache::initialize(u32 _total_kb_approx, u32 bytes_per_line)
//{
//    // use twice the requisted memory (to avoid bad configs)
//    _total_kb_approx *= 2;
//
//    // calc
//    _line = bytes_per_line;
//    _count = ((_total_kb_approx * 1024) / bytes_per_line + 1);
//    _total = _count * _line;
//    R_ASSERT(_count < CAT_FREE);
//    Msg("* sound : cache: %d kb, %d lines, %d bpl", _total / 1024, _count, _line);
//
//    // alloc structs
//    data = xr_alloc<u8>(_total);
//    c_storage = xr_alloc<cache_line>(_count);
//
//    // format
//    format();
//}

//void CSoundRender_Cache::disconnect()
//{
//    // disconnect from CATs
//    for (u32 it = 0; it < _count; it++)
//    {
//        cache_line* L = c_storage + it;
//        if (L->loopback)
//        {
//            *L->loopback = CAT_FREE;
//            L->loopback = NULL;
//        }
//    }
//}

//void CSoundRender_Cache::format()
//{
//    // format structs
//    for (u32 it = 0; it < _count; it++)
//    {
//        cache_line* L = c_storage + it;
//        L->prev = (0 == it) ? NULL : c_storage + it - 1;
//        L->next = ((_count - 1) == it) ? NULL : c_storage + it + 1;
//        L->data = data + it * _line;
//        L->loopback = NULL;
//        L->id = u16(it);
//    }
//
//    // start-end
//    c_begin = c_storage + 0;
//    c_end = c_storage + _count - 1;
//}

//void CSoundRender_Cache::purge()
//{
//    disconnect(); // disconnect from CATs
//    format(); // format
//}

//void CSoundRender_Cache::destroy()
//{
//    disconnect();
//    xr_free(data);
//    xr_free(c_storage);
//    c_begin = NULL;
//    c_end = NULL;
//    _total = 0;
//    _line = 0;
//    _count = 0;
//}

//void CSoundRender_Cache::cat_create(cache_cat& cat, u32 bytes)
//{
//    cat.size = bytes / _line;
//    if (bytes % _line)
//        cat.size += 1;
//    u32 allocsize = (cat.size & 1) ? cat.size + 1 : cat.size;
//    cat.table = xr_alloc<u16>(allocsize);
//    memset(cat.table, 0xff, allocsize * sizeof u16); // fill32
//}

//void CSoundRender_Cache::cat_destroy(cache_cat& cat)
//{
//    xr_free(cat.table);
//    cat.size = 0;
//}
