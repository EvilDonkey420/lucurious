/**
* The MIT License (MIT)
*
* Copyright (c) 2019-2020 Vincent Davis Jr.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <lucom.h>
#include "../../include/vkcomp/types.h"
#include "../../include/drm/types.h"

#define BLOCK_SIZE sizeof(dlu_mem_block_t)

/**
* Struct that stores block metadata
* Using linked list to keep track of memory allocated
* next     | points to next memory block
* size     | allocated memory size
* abytes   | available bytes left in block
* addr     | Current address of the block
* saddr    | Starting address of the block where data is assigned
* prv_addr | Address of the previous block
*/
typedef struct mblock {
  struct mblock *next;
  size_t size;
  size_t abytes;
  void *addr;
  void *saddr;
  void *prv_addr;
} dlu_mem_block_t;

/**
* Globals used to keep track of memory blocks
* sstart_addr_priv: Keep track of first allocated private small block address
* large_block_priv: A struct to keep track of one large allocated private block
* small_block_priv: A linked list for smaller allocated private block
* sstart_addr_shared: Keep track of first allocated small shared block address
* large_block_shared: A struct to keep track of one large allocated shared block
* small_block_shared: linked list for smaller allocated shared blocks
*/
static void *sstart_addr_priv = NULL;
static dlu_mem_block_t *large_block_priv = NULL;
static dlu_mem_block_t *small_block_priv = NULL;

static void *sstart_addr_shared = NULL;
static dlu_mem_block_t *large_block_shared = NULL;
static dlu_mem_block_t *small_block_shared = NULL;

/**
* Helps in ensuring one does not waste cycles in context switching
* First check if sub-block was allocated and is currently free
* If block not free, sub allocate more from larger memory block
*/
static dlu_mem_block_t *get_free_block(dlu_block_type type, size_t bytes) {
  dlu_mem_block_t *current = NULL;
  size_t abytes = 0;

  /**
  * This allows for O(1) allocation
  * If next block doesn't exists use large_block_priv/shared->saddr address
  * else set current to next block (which would be the block waiting to be allocated)
  */
  switch(type) {
    case DLU_SMALL_BLOCK_PRIV:
      abytes = large_block_priv->abytes;
      current = (!small_block_priv->next) ? sstart_addr_priv : small_block_priv->next;
      break;
    case DLU_SMALL_BLOCK_SHARED:
      abytes = large_block_shared->abytes;
      current = (!small_block_shared->next) ? sstart_addr_shared : small_block_shared->next;
      break;
    default: break;
  }

  /* An extra check, although this should never be NULL */
  if (!current) return NULL;

  if (abytes >= bytes) {
    /* current block thats about to be allocated set few metadata */
    dlu_mem_block_t *block = current->addr;
    block->size = bytes;
    /* Put saddr at an address that doesn't contain metadata */
    block->saddr = BLOCK_SIZE + current->addr;

    /**
    * Set next blocks metadata
    * This is written this way because one needs the return address
    * to be the starting address of the next block not the current one.
    * Basically offset the memory address. Thus, allocating space.
    */
    block = current->addr + BLOCK_SIZE + bytes;

    /* set next block meta data */
    block->addr = block;
    block->next = NULL;
    block->size = 0;
    block->abytes = 0;
    block->saddr = NULL;
    block->prv_addr = current->addr;

    /* Decrement larger block available memory */
    switch(type) {
      case DLU_SMALL_BLOCK_SHARED: large_block_shared->abytes -= (BLOCK_SIZE + bytes); break;
      case DLU_SMALL_BLOCK_PRIV: large_block_priv->abytes -= (BLOCK_SIZE + bytes); break;
      default: break;
    }

    return block;
  }

  return NULL;
}

static dlu_mem_block_t *alloc_mem_block(dlu_block_type type, size_t bytes) {
  dlu_mem_block_t *block = NULL;

  /* Allows for zeros to be written into values of bytes allocated */
  int fd = open("/dev/zero", O_RDWR);
  if (fd == NEG_ONE) {
    dlu_log_me(DLU_DANGER, "[x] open: %s", strerror(errno));
    goto finish_alloc_mem_block;
  }

  /* Can only allocate up to 8GB, 2^33, or 1ULL << 33 */
  int flags = (type == DLU_LARGE_BLOCK_SHARED) ? MAP_SHARED : MAP_PRIVATE;
  block = mmap(NULL, BLOCK_SIZE + bytes, PROT_READ | PROT_WRITE, flags | MAP_ANONYMOUS, fd, 0);
  if (block == MAP_FAILED) {
    dlu_log_me(DLU_DANGER, "[x] mmap: %s", strerror(errno));
    goto finish_alloc_mem_block;
  }

  block->next = NULL;
  block->size = block->abytes = bytes;

  /* Put saddr at an address that doesn't contain metadata */
  block->addr = block;
  block->saddr = BLOCK_SIZE + block;
finish_alloc_mem_block:
  if (close(fd) == NEG_ONE)
    dlu_log_me(DLU_DANGER, "[x] close: %s", strerror(errno));
  return block;
}

/**
* This function is reserve for one time use. Only used when allocating space for struct members
* It works similiar to how sbrk works. Basically it creates a new block of memory, but it returns
* the ending address of the previous block.
*/
void *dlu_alloc(dlu_block_type type, size_t bytes) {
  dlu_mem_block_t *nblock = NULL;

  /**
  * This will create large block of memory
  * Then create a linked list of smaller blocks from the larger one
  * Retrieve last used memory block
  * O(1) appending to end of linked-list
  * reset linked list to starting address of linked list
  */

  /* TODO: Possible rewrite of switch statement */
  switch (type) {
    case DLU_LARGE_BLOCK_PRIV:
      /* If large block allocated don't allocate another one */
      if (large_block_priv) { PERR(DLU_ALREADY_ALLOC, 0, NULL); return NULL; }

      nblock = alloc_mem_block(type, bytes);
      if (!nblock) return NULL;
      large_block_priv = nblock;

      /**
      * Set small block allocation addr to address that
      * doesn't include larger block metadata
      */
      small_block_priv = sstart_addr_priv = nblock->saddr;
      small_block_priv->addr = small_block_priv;
      break;
    case DLU_SMALL_BLOCK_PRIV:
      /* If large block not allocated return NULL until allocated */
      if (!large_block_priv) return NULL;

      nblock = get_free_block(type, bytes);
      if (!nblock) return NULL;

      /* set small block list to address of the previous block in the list */
      small_block_priv = nblock->prv_addr;
      small_block_priv->next = nblock;

      /* Move back to previous block (for return status) */
      nblock = small_block_priv;
      break;
    case DLU_LARGE_BLOCK_SHARED:
      /* If large block allocated don't allocate another one */
      if (large_block_shared) { PERR(DLU_ALREADY_ALLOC, 0, NULL); return NULL; }

      nblock = alloc_mem_block(type, bytes);
      if (!nblock) return NULL;
      large_block_shared = nblock;

      /**
      * Set small block allocation addr to address that
      * doesn't include larger block metadata
      */
      small_block_shared = sstart_addr_shared = nblock->saddr;
      small_block_shared->addr = small_block_shared;
      break;
    case DLU_SMALL_BLOCK_SHARED:
      /* If large block not allocated return NULL until allocated */
      if (!large_block_shared) return NULL;

      nblock = get_free_block(type, bytes);
      if (!nblock) return NULL;

      /* set small block list to address of the previous block in the list */
      small_block_shared = nblock->prv_addr;
      small_block_shared->next = nblock;

      /* Move back to previous block (for return status) */
      nblock = small_block_shared;
      break;
    default: break;
  }

  return nblock->saddr;
}

bool dlu_otma(dlu_block_type type, dlu_otma_mems ma) {
  size_t size = 0;

  if (type == DLU_SMALL_BLOCK_PRIV || type == DLU_SMALL_BLOCK_SHARED) {
    PERR(DLU_OP_NOT_PERMITED, 0, NULL);
    return false;
  }

  if (large_block_priv) { PERR(DLU_ALREADY_ALLOC, 0, NULL); return false; }
  if (large_block_shared) { PERR(DLU_ALREADY_ALLOC, 0, NULL); return false; }

  /* This allows for exact byte allocation. Resulting in no fragmented memory */
  size += (ma.inta_cnt) ? (BLOCK_SIZE + (ma.inta_cnt * sizeof(int))) : 0;
  size += (ma.cha_cnt ) ? (BLOCK_SIZE + (ma.cha_cnt  * sizeof(char))) : 0;
  size += (ma.fla_cnt  ) ? (BLOCK_SIZE + (ma.fla_cnt   * sizeof(float))) : 0;
  size += (ma.fla_cnt  ) ? (BLOCK_SIZE + (ma.dba_cnt  * sizeof(double))) : 0;

  size += (ma.vkcomp_cnt     ) ? (BLOCK_SIZE + (ma.vkcomp_cnt * sizeof(vkcomp))) : 0;
  size += (ma.vkext_props_cnt) ? (BLOCK_SIZE + (ma.vkext_props_cnt * sizeof(VkExtensionProperties))) : 0;
  size += (ma.vk_layer_cnt) ? (BLOCK_SIZE + (ma.vk_layer_cnt * sizeof(VkLayerProperties))) : 0;

  size += (ma.si_cnt ) ? (BLOCK_SIZE + (ma.si_cnt * sizeof(struct _swap_chain_buffers))) : 0;
  size += (ma.si_cnt ) ? (BLOCK_SIZE + (ma.si_cnt * sizeof(struct _synchronizers))) : 0;
  size += (ma.scd_cnt) ? (BLOCK_SIZE + (ma.scd_cnt* sizeof(struct _sc_data))) : 0;

  size += (ma.gp_cnt ) ? (BLOCK_SIZE + (ma.gp_cnt * sizeof(VkPipeline))) : 0;
  size += (ma.gpd_cnt) ? (BLOCK_SIZE + (ma.gpd_cnt * sizeof(struct _gp_data))) : 0;

  size += (ma.si_cnt  ) ? (BLOCK_SIZE + (ma.si_cnt * sizeof(VkCommandBuffer))) : 0;
  size += (ma.cmdd_cnt) ? (BLOCK_SIZE + (ma.cmdd_cnt * sizeof(struct _cmd_data))) : 0;

  size += (ma.bd_cnt) ? (BLOCK_SIZE + (ma.bd_cnt * sizeof(struct _buff_data))) : 0;

  size += (ma.desc_cnt) ? (BLOCK_SIZE + (ma.desc_cnt * sizeof(VkDescriptorSet))) : 0;
  size += (ma.desc_cnt) ? (BLOCK_SIZE + (ma.desc_cnt * sizeof(VkDescriptorSetLayout))) : 0;
  size += (ma.dd_cnt  ) ? (BLOCK_SIZE + (ma.dd_cnt * sizeof(struct _desc_data))) : 0;

  size += (ma.td_cnt ) ? (BLOCK_SIZE + (ma.td_cnt * sizeof(struct _text_data))) : 0;

  size += (ma.pd_cnt) ? (BLOCK_SIZE + (ma.pd_cnt * sizeof(struct _pd_data))) : 0;
  size += (ma.ld_cnt) ? (BLOCK_SIZE + (ma.ld_cnt * sizeof(struct _ld_data))) : 0;

  size += (ma.drmc_cnt) ? (BLOCK_SIZE + (ma.drmc_cnt * sizeof(dlu_drm_core))) : 0;
  size += (ma.dod_cnt ) ? (BLOCK_SIZE + (ma.dod_cnt * sizeof(struct _output_data))) : 0;

  size += (ma.dob_cnt) ? (BLOCK_SIZE + (ma.dob_cnt * sizeof(struct _drm_buff_data))) : 0;

  if (!dlu_alloc(type, size)) return false;

  return true;
}

bool dlu_otba(dlu_data_type type, void *addr, uint32_t index, uint32_t arr_size) {
  switch (type) {
    case DLU_SC_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->sc_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _sc_data));
        if (!app->sc_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->sc_data[i].ldi = UINT32_MAX;

        app->sdc = arr_size; return true;
      }
    case DLU_GP_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->gp_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _gp_data));
        if (!app->gp_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->gp_data[i].ldi = UINT32_MAX;

        app->gdc = arr_size; return true;
      }
    case DLU_CMD_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->cmd_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _cmd_data));
        if (!app->cmd_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->cmd_data[i].ldi = UINT32_MAX;

        app->cdc = arr_size; return true;
      }
    case DLU_BUFF_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->buff_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _buff_data));
        if (!app->buff_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->buff_data[i].ldi = UINT32_MAX;

        app->bdc = arr_size; return true;
      }
    case DLU_DESC_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->desc_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _desc_data));
        if (!app->desc_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->gp_data[i].ldi = UINT32_MAX;

        app->ddc = arr_size; return true;
      }
    case DLU_TEXT_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->text_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _text_data));
        if (!app->text_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate ldi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->gp_data[i].ldi = UINT32_MAX;

        app->tdc = arr_size; return true;
      }
    case DLU_PD_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->pd_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _pd_data));
        if (!app->pd_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* need for dlu_create_queue_families(3) */
        for (uint32_t i = 0; i < arr_size; i++) {
          app->pd_data[i].gfam_idx = UINT32_MAX;
          app->pd_data[i].cfam_idx = UINT32_MAX;
          app->pd_data[i].tfam_idx = UINT32_MAX;
        }

        app->pdc = arr_size; return true;
      }
    case DLU_LD_DATA:
      {
        vkcomp *app = (vkcomp *) addr;
        app->ld_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _ld_data));
        if (!app->ld_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Populate pdi for error checking */
        for (uint32_t i = 0; i < arr_size; i++)
          app->ld_data[i].pdi = UINT32_MAX;

        app->ldc = arr_size; return true;
      }
    case DLU_SC_DATA_MEMS:
      {
        vkcomp *app = (vkcomp *) addr;
        /**
        * Don't want to stick to minimum because one would have to wait on the
        * driver to complete internal operations before one can acquire another
        * image to render to. So it's recommended to add one to minImageCount
        */
        arr_size += 1;

        /* Allocate SwapChain Buffers (VkImage, VkImageView, VkFramebuffer) */
        app->sc_data[index].sc_buffs = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _swap_chain_buffers));
        if (!app->sc_data[index].sc_buffs) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Allocate CommandBuffers */
        app->cmd_data[index].cmd_buffs = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(VkCommandBuffer));
        if (!app->cmd_data[index].cmd_buffs) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        /* Allocate Semaphores */
        app->sc_data[index].syncs = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _synchronizers));
        if (!app->sc_data[index].syncs) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }
        app->sc_data[index].sic = arr_size; return true;
      }
    case DLU_DESC_DATA_MEMS:
      {
        vkcomp *app = (vkcomp *) addr;

        app->desc_data[index].layouts = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(VkDescriptorSetLayout));
        if (!app->desc_data[index].layouts) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        app->desc_data[index].desc_set = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(VkDescriptorSet));
        if (!app->desc_data[index].desc_set) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        app->desc_data[index].dlsc = arr_size; return true;
      }
    case DLU_GP_DATA_MEMS:
      {
        vkcomp *app = (vkcomp *) addr;
        app->gp_data[index].graphics_pipelines = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(VkPipeline));
        if (!app->gp_data[index].graphics_pipelines) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }
        app->gp_data[index].gpc = arr_size; return true;
      }
    case DLU_DEVICE_OUTPUT_DATA:
      {
        dlu_drm_core *core = (dlu_drm_core *) addr;
        core->output_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _output_data));
        if (!core->output_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }
        core->odc = arr_size; return true;
      }
    case DLU_DEVICE_OUTPUT_BUFF_DATA:
      {
        dlu_drm_core *core = (dlu_drm_core *) addr;
        core->buff_data = dlu_alloc(DLU_SMALL_BLOCK_PRIV, arr_size * sizeof(struct _drm_buff_data));
        if (!core->buff_data) { PERR(DLU_ALLOC_FAILED, 0, NULL); return false; }

        for (uint32_t i = 0; i < arr_size; i++) {
          core->buff_data[i].fb_id = UINT32_MAX;
          core->buff_data[i].odid = UINT32_MAX;
          for (uint32_t j = 0; j < 4; j++)
            core->buff_data[i].dma_buf_fds[j] = NEG_ONE;
        }

        core->odbc = arr_size; return true;
      }
    default: break;
  }

  /* They somehow passed compiler checks and failed here */
  return false;
}

/**
* Releasing memory in this case means to
* unmap all virtual pages (remove page tables)
*/
void dlu_release_blocks() {
  if (large_block_priv) {
    if (munmap(large_block_priv, BLOCK_SIZE + large_block_priv->size) == NEG_ONE) {
      dlu_log_me(DLU_DANGER, "[x] munmap: %s", strerror(errno));
      return;
    }
    large_block_priv = NULL;
  }

  if (large_block_shared) {
    if (munmap(large_block_shared, BLOCK_SIZE + large_block_shared->size) == NEG_ONE) {
      dlu_log_me(DLU_DANGER, "[x] munmap: %s", strerror(errno));
      return;
    }
    large_block_shared = NULL;
  }
}

/* This is an INAPI_CALL */
void dlu_print_mb(dlu_block_type type) {
  dlu_mem_block_t *current = (type == DLU_SMALL_BLOCK_SHARED) ? sstart_addr_shared : sstart_addr_priv;
  while (current->next) {
    dlu_log_me(DLU_INFO, "current block = %p, next block = %p, block size = %d, saddr = %p",
                          current, current->next, current->size, current->saddr);
    current = current->next;
  }
}
