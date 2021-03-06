/**
 * Copyright (C) 2020-2021 KeyboardSlayer (Jordan Dalcq)
 * 
 * This file is part of Navy.
 * 
 * Navy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Navy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Navy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pmm.h"
#include <libk/debug.h>
#include <stdint.h>
#include <string.h>

static uint8_t *bitmap = NULL;
static uint64_t free_memory = 0;
static uint64_t last_free_page = 0;
static size_t usable_pages;

static void
unset_pmm(uint64_t page_addr)
{
    uint64_t bit = page_addr % 8;
    uint64_t byte = page_addr / 8;

    bitmap[byte] &= ~(1 << bit);
}

static void
set_pmm(uint64_t page_addr)
{
    uint64_t bit = page_addr % 8;
    uint64_t byte = page_addr / 8;

    last_free_page = page_addr;
    bitmap[byte] |= (1 << bit);
}

static uint64_t
is_set_pmm(uint64_t page_addr)
{
    uint64_t bit = page_addr % 8;
    uint64_t byte = page_addr / 8;
    return bitmap[byte] & (1 << bit);
}

void
set_used_pmm(AddrRange range)
{
    size_t i;
    uintptr_t addr;

    assert(is_page_aligned(range));

    for (i = 0; i < range.length / PAGE_SIZE; i += PAGE_SIZE)
    {
        addr = range.base + i;

        if (!is_set_pmm(addr))
        {
            set_pmm(addr / PAGE_SIZE);
            free_memory -= PAGE_SIZE;
        }
    }
}

void
free_pmm(AddrRange range)
{
    size_t i;
    uintptr_t target = (uintptr_t) range.base / PAGE_SIZE;

    for (i = target; i < target + range.length / PAGE_SIZE; i++)
    {
        unset_pmm(i);
    }
}

void
init_pmm(BootInfo *info)
{
    module("PMM");

    size_t i;
    usable_pages = info->usable_pages;

    log_debug(INFO, "Usable pages: %d", usable_pages);

    for (i = 0; i < info->memory_map_size; i++)
    {
        if (info->mmap[i].type == STIVALE2_MMAP_USABLE && info->mmap[i].range.length >= usable_pages)
        {
            bitmap = (uint8_t *) info->mmap[i].range.base;
            log_debug(INFO, "Found mmap for the bitmap");
        }
    }

    assert(bitmap != NULL);
    memset(bitmap, 0xff, usable_pages);

    for (i = 0; i < info->memory_map_size; i++)
    {
        size_t j;

        if (info->mmap[i].type == STIVALE2_MMAP_USABLE)
        {
            for (j = 0; j < info->mmap[i].range.length; j += PAGE_SIZE)
            {
                unset_pmm((info->mmap[i].range.base + j) / PAGE_SIZE);
                free_memory += PAGE_SIZE;
            }
        }
    }

    for (i = 0; i <= usable_pages; i += PAGE_SIZE)
    {
        unset_pmm((i + (uint64_t) bitmap) / PAGE_SIZE);
    }

    log_debug(SUCCESS, "");
}


static size_t
find_free_pages(size_t count)
{
    size_t page_found = 0;
    size_t i;

    for (i = last_free_page; i < usable_pages; i++)
    {
        if (!is_set_pmm(i))
        {
            page_found++;

            if (page_found == count)
            {
                last_free_page = i;
                return i;
            }
        }
        else
        {
            page_found = 0;
        }
    }

    if (last_free_page != 0)
    {
        last_free_page = 0;
        return find_free_pages(count);
    }

    assert(0);
    return 0;
}

void *
alloc_pmm(size_t count)
{
    module_push();
    module("PMM");

    size_t i;
    size_t page = find_free_pages(count);

    for (i = page; i < count+page; i++)
    {
        set_pmm(i);
    }

    module_pop();
    
    return (void *) (page * PAGE_SIZE);
}
