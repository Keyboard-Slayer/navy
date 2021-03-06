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

#ifndef _KERNEL_MEM_PMM_H_
#define _KERNEL_MEM_PMM_H_

#include <libk/bootinfo.h>
#include <libk/range.h>
#include <stdint.h>

void init_pmm(BootInfo *);
void free_pmm(AddrRange);
void *alloc_pmm(size_t);
void set_used_pmm(AddrRange);

#endif /* !_KERNEL_MEM_PMM_H_ */
