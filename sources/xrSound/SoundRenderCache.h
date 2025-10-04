#pragma once
#include <stdint.h>

constexpr uint32_t CAT_FREE = 0xffff;

// --- just thoughts ---
// 1. LRU scheme
// 2. O(1) constant time access
// 3. O(1) constant time LRU-find (deque-like?)
// 4. fixed-count of blocks will allow efficient(cyclic) implementation of deque
// 5. allow FAT-like formatting for sources
// 7. bi-directional cache availability tracking
// 9. "touch" protocol
// 10. in case of cache-hit we have to move line to mark it used -> list

struct CacheTable
{
    uint16_t* Lines;
    uint32_t Size;
};

struct CacheLine
{
    CacheLine* Previous;
    CacheLine* Next;
    void* Data;
    uint16_t* Loopback;
    uint16_t Id;
};

class SoundRenderCache final
{
public:
    SoundRenderCache();
    ~SoundRenderCache() = default;

    void* GetDataById(CacheTable& table, uint32_t id);

    uint32_t TotalSize();
    uint32_t LineSize();
    uint32_t LinesCount();
    uint32_t StatsHits();
    uint32_t StatsMiss();

    void Initialize(uint32_t totalSize, uint32_t bytesPerLine);
    void Destroy();

    void CreateTable(CacheTable& table, uint32_t size);
    void DestroyTable(CacheTable& table);

    bool Request(CacheTable& table, uint32_t id);
    void Purge();
    void Disconnect();
    void Format();
    void MoveToTop(CacheLine* line);    
    void ClearStatistic();

private:
    uint8_t* m_data;
    uint32_t m_totalSize;
    uint32_t m_lineSize;
    uint32_t m_linesCount;

    uint32_t m_statsHit;
    uint32_t m_statsMiss;

    CacheLine* m_storage;
    CacheLine* m_frontLine;
    CacheLine* m_backLine;
};
