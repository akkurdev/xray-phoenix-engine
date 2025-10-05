#include "stdafx.h"
#include "SoundRender_Core.h"
#include "SoundRenderSource.h"
#include "SoundRender_Emitter.h"
#include "OalSoundRenderTarget.h"

void CSoundRender_Emitter::fill_data(u8* _dest, u32 offset, u32 size)
{
    u32 line_size = SoundRender->cache.LineSize();
    u32 line = offset / line_size;

    // prepare for first line (it can be unaligned)
    u32 line_offs = offset - line * line_size;
    u32 line_amount = line_size - line_offs;

    while (size)
    {
        // cache access
        if (SoundRender->cache.Request(*RenderSource()->Cache(), line))
        {
            RenderSource()->Decompress(line, m_renderTarget->OggFile());
        }

        // fill block
        u32 blk_size = _min(size, line_amount);
        u8* ptr = (u8*)SoundRender->cache.GetDataById(*RenderSource()->Cache(), line);
        CopyMemory(_dest, ptr + line_offs, blk_size);

        // advance
        line++;
        size -= blk_size;
        _dest += blk_size;
        offset += blk_size;
        line_offs = 0;
        line_amount = line_size;
    }
}

void CSoundRender_Emitter::fill_block(void* ptr, u32 size)
{
    // Msg			("stream: %10s - [%X]:%d, p=%d, t=%d",*source->fname,ptr,size,position,source->dwBytesTotal);
    LPBYTE dest = LPBYTE(ptr);
    u32 dwBytesTotal = get_bytes_total();

    if ((get_cursor(true) + size) > dwBytesTotal)
    {
        // We are reaching the end of data, what to do?
        switch (m_state)
        {
        case EmitterState::Playing: { // Fill as much data as we can, zeroing remainder
            if (get_cursor(true) >= dwBytesTotal)
            {
                // ??? We requested the block after remainder - just zero
                Memory.mem_fill(dest, 0, size);
            }
            else
            {
                // Calculate remainder
                u32 sz_data = dwBytesTotal - get_cursor(true);
                u32 sz_zero = (get_cursor(true) + size) - dwBytesTotal;
                VERIFY(size == (sz_data + sz_zero));
                fill_data(dest, get_cursor(false), sz_data);
                Memory.mem_fill(dest + sz_data, 0, sz_zero);
            }
            move_cursor(size);
        }
        break;
        case EmitterState::PlayingLooped: {
            u32 hw_position = 0;
            do
            {
                u32 sz_data = dwBytesTotal - get_cursor(true);
                u32 sz_write = _min(size - hw_position, sz_data);
                fill_data(dest + hw_position, get_cursor(true), sz_write);
                hw_position += sz_write;
                move_cursor(sz_write);
                set_cursor(get_cursor(true) % dwBytesTotal);
            } while (0 != (size - hw_position));
        }
        break;
        default: FATAL("SOUND: Invalid emitter state"); break;
        }
    }
    else
    {
        u32 bt_handle = m_ownerData->handle->BytesCount();
        if (get_cursor(true) + size > m_handleCursor + bt_handle)
        {
            R_ASSERT(m_ownerData->fn_attached[0].size());

            u32 rem = 0;
            if ((m_handleCursor + bt_handle) > get_cursor(true))
            {
                rem = (m_handleCursor + bt_handle) - get_cursor(true);

#ifdef DEBUG
                Msg("reminder from prev source %d", rem);
#endif // #ifdef DEBUG
                fill_data(dest, get_cursor(false), rem);
                move_cursor(rem);
            }
#ifdef DEBUG
            Msg("recurce from next source %d", size - rem);
#endif // #ifdef DEBUG
            fill_block(dest + rem, size - rem);
        }
        else
        {
            // Everything OK, just stream
            fill_data(dest, get_cursor(false), size);
            move_cursor(size);
        }
    }
}

ISoundRenderSource* CSoundRender_Emitter::RenderSource()
{
    return m_ownerData->handle;
}

ISoundRenderTarget* CSoundRender_Emitter::RenderTarget()
{
    return m_renderTarget;
}

u32 CSoundRender_Emitter::get_bytes_total() const
{
    return m_ownerData->dwBytesTotal;
}

float CSoundRender_Emitter::get_length_sec() const
{
    return m_ownerData->get_length_sec();
}
