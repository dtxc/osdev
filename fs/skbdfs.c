#include <string.h>
#include <hw/ata.h>
#include <asm/io.h>
#include <mm/kheap.h>
#include <fs/skbdfs.h>

uint8_t mode_calc(uint8_t user, uint8_t kernel) {
    return user << 3 | kernel;
}

/*
    finds the next free block in the filesystem.
    returns null if there are no free blocks available

    iblk: starting block
*/
static uint32_t find_next_free_block(block_t *dest, uint32_t iblk) {
    uint64_t offset = iblk * BLOCK_SIZE;
    uint32_t fs_size = 1048576;

    block_t *blk = (block_t *) kmalloc(BLOCK_SIZE);
    // fseek(fp, offset, SEEK_SET);

    while (offset < fs_size) {
        memset(blk, 0, BLOCK_SIZE);
        ata_read_bytes(offset, BLOCK_SIZE, (uint8_t *) blk);

        if (blk->attributes & BLOCK_FREE) {
            if (dest != NULL) {
                memcpy(dest, blk, BLOCK_SIZE);
            }
            kfree(blk);
            return offset / BLOCK_SIZE;
        }

        offset += BLOCK_SIZE;
    }

    kfree(blk);
    return 0;
}

static block_t *get_block_by_index(uint32_t iblk) {
    uint64_t offset = iblk * BLOCK_SIZE;
    block_t *blk = (block_t *) kmalloc(BLOCK_SIZE);

    ata_read_bytes(offset, BLOCK_SIZE, (uint8_t *) blk);

    return blk;
}

static uint32_t find_node(char *path, int type, int parent, node_t *dst) {
    char **pwd_split;

    // path + 1: skip initial '/'
    int nsplits = split_string(path + 1, '/', &pwd_split);
    if (parent) nsplits--;

    block_t *blk = get_block_by_index(1);
    node_t *node = (node_t *) kmalloc(sizeof(node_t));

    memcpy(node, blk->data, sizeof(node_t));
    uint32_t *children = (uint32_t *) (blk->data + sizeof(node_t));

    int found = 0;
    uint32_t block_offset = 1;
    node_t *temp = (node_t *) kmalloc(sizeof(node_t));
    block_t *b;
    for (int i = 0; i < nsplits; i++) {
        found = 0;
        for (int j = 0; j < node->size; j++) { // iterate through each subdirectory of the parent.
            // possible data corruption
            if (*children == 0) return 0;

            b = get_block_by_index(*children);
            // assert(b->attributes & BLOCK_METADATA); // block has to contain a node
            memcpy(temp, b->data, sizeof(node_t));
            // assert(temp->magic == NODE_MAGIC);

            int target_type = (i == nsplits-1) ? type : FS_DIR;
            if (temp->flags & target_type) {
                if (strcmp(temp->name, pwd_split[i]) == 1) {
                    found = 1;
                    block_offset = *children;
                    memcpy(node, temp, sizeof(node_t));
                    memcpy(blk, b, BLOCK_SIZE);

                    children = (uint32_t *) (blk->data + sizeof(node_t));
                    kfree(b);
                    break;
                }
            }

            children++;
            kfree(b);
        }

        // node was not found.
        if (found == 0) {
            return 0;
        }
    }

    if (parent) nsplits++;
    for (int i = 0; i < nsplits; i++) {
        kfree(pwd_split[i]);
    }

    memcpy(dst, node, sizeof(node_t));
    kfree(node);
    kfree(temp);
    kfree(pwd_split);

    return block_offset;
}

node_t *mknode(char *path, int type) {
    node_t *node = (node_t *) kmalloc(sizeof(node_t));
    uint32_t block_offset = find_node(path, FS_DIR, 1, node);
    if (block_offset == 0) {
        kfree(node);
        return NULL;
    }

    char **temp;
    int nsplits = split_string(path + 1, '/', &temp);
    char *name = (char *) kmalloc(32);

    strcpy(name, temp[nsplits-1]);

    for (int i = 0; i < nsplits; i++) {
        kfree(temp[i]);
    }
    kfree(temp);

    block_t *blk = get_block_by_index(block_offset);
    memcpy(node, blk->data, sizeof(node_t));

    uint32_t parent_size = node->size;
    uint32_t parent_off = node->first_block;

    block_t *block = (block_t *) kmalloc(BLOCK_SIZE);
    uint32_t r = find_next_free_block( block, block_offset);
    // assert(r > 0);

    memcpy(blk->data + sizeof(node_t) + node->size * 4, &r, 4);
    node->size++;
    memcpy(blk->data, node, sizeof(node_t));

    ata_write_bytes(parent_off * BLOCK_SIZE, BLOCK_SIZE, (uint8_t *) blk);
    
    memset(blk, 0, BLOCK_SIZE);
    memset(node, 0, sizeof(node_t));

    node->magic = NODE_MAGIC;
    node->first_block = r;
    node->flags |= type;
    if (type == FS_FILE) node->mode = 63; // rwx rwx
    else if (type == FS_DIR) node->mode = 54; // rw- rw-
    else if (type == FS_CHARDEV) node->mode = 38; // r-- rw-
    node->size = 0;
    strcpy(node->name, name);

    blk->attributes |= BLOCK_METADATA;
    blk->next = 0;
    memcpy(blk->data, node, sizeof(node_t));
    ata_write_bytes(r * BLOCK_SIZE, BLOCK_SIZE, (uint8_t *) blk);

    kfree(blk);
    return node;
}

// if both append and write is selected, write will be executed
file_t *fopen(char *path, uint8_t mode) {
    if (mode == 0) return NULL;

    node_t *node = (node_t *) kmalloc(sizeof(node_t));
    file_t *file = (file_t *) kmalloc(sizeof(file_t));
    uint32_t off = find_node(path, FS_FILE, 0, node);

    if (mode & FMODE_W) {
        if (off == 0) { // file does not exist
            kfree(node); // free old node
            node = mknode(path, FS_FILE);
        }

        if ((node->mode & MODE_W) == 0) {
            kfree(node);
            kfree(file);
            return NULL; // not writeable
        }

        file->node = node;
        file->mode = mode;
        file->is_locked = 0;
        file->ptr_local = 0;
        file->ptr_global = BLOCK_SIZE * node->first_block;
    } else if (mode & FMODE_A) { // append
        if (off == 0) { // file does not exist
            kfree(node);
            node = mknode(path, FS_FILE);
        }

        if ((node->mode & MODE_W) == 0) {
            kfree(node);
            kfree(file);
            return NULL;
        }

        // we need to find the last block to set the file pointer
        block_t *block = get_block_by_index(node->first_block);
        int next = node->first_block;
        while (block->next != 0) {
            next = block->next;
            kfree(block);

            block = get_block_by_index(block->next);
        }

        file->node = node;
        file->mode = mode;
        file->is_locked = 0;
        file->ptr_local = node->size;
        file->ptr_global = next * BLOCK_SIZE + node->size % BLOCK_SIZE;
        if (next == node->first_block) file->ptr_global += sizeof(node_t);
    } else if (mode & FMODE_R) {
        if (off == 0) {
            kfree(node);
            kfree(file);
            return NULL;
        }

        file->node = node;
        file->mode = mode;
        file->is_locked = 0;
        file->ptr_local = 0;
        file->ptr_global = BLOCK_SIZE * node->first_block;
    }

    return file;
}

int fread(file_t *file, uint32_t size, uint8_t *buffer) {
    if ((file->mode & FMODE_R) == 0) {
        return -1;
    }

    // wait for the file to be unlocked
    while (file->is_locked != 0);

    file->is_locked = 1;

    // in the first block of the file there's both metadata and data
    int offset = file->ptr_local + sizeof(node_t);

    int remaining_size = (file->node->size + sizeof(node_t)) - offset;
    if (remaining_size <= 0) {
        serial_printf("1\n");
        return -1; // invalid seek/reached EOF
    } else if (remaining_size < size) {
        size = remaining_size;
    }

    // find seek pointer in current block.
    int nblocks_real = offset / BLOCK_DATA_SIZE; // get number of real blocks.
    int off_crt_block = offset - nblocks_real * BLOCK_DATA_SIZE;

    block_t *crt_block = get_block_by_index(file->ptr_global / BLOCK_SIZE);

    // read blocks
    // remaining data = BLOCK_DATA_SIZE - off_crt_block
    if (size <= BLOCK_DATA_SIZE - off_crt_block) {
        memcpy(buffer, crt_block->data + off_crt_block, size);

        file->ptr_local += size;
        file->ptr_global += size;
        return 0;
    }

    // size > current block
    memcpy(buffer, crt_block->data + off_crt_block, BLOCK_DATA_SIZE - off_crt_block);
    buffer += BLOCK_DATA_SIZE - off_crt_block;
    size -= BLOCK_DATA_SIZE - off_crt_block;

    int temp = size / BLOCK_DATA_SIZE; // number of whole blocks to read
    int next = crt_block->next;
    for (int i = 0; i < temp; i++) {
        if (crt_block->next == 0) {
            serial_printf("2\n");
            return -1; // I/O error
        }

        // find next block
        kfree(crt_block);
        crt_block = get_block_by_index(next);
        memcpy(buffer, crt_block->data, BLOCK_DATA_SIZE);

        next = crt_block->next;
        buffer += BLOCK_DATA_SIZE;
        size -= BLOCK_DATA_SIZE;
        file->ptr_local += BLOCK_DATA_SIZE;
    }

    // we do not need the entire block.
    if (size > 0) {
        if (crt_block->next == 0) {
            serial_printf("3\n");
            return -1; // I/O error
        }

        int next = crt_block->next;
        kfree(crt_block);
        crt_block = get_block_by_index(next);
        memcpy(buffer, crt_block->data, size);
    }

    file->ptr_global = BLOCK_SIZE * next + size;
    file->ptr_local += size;
    file->is_locked = 0;

    kfree(crt_block);
    return 0;
}

int fwrite(file_t *file, uint32_t size, uint8_t *buffer) {
    if ((file->mode & FMODE_W) == 0) { // append mode is not implemented.
        return -1; // file is readonly
    }

    if (size == 0) return 0;

    while (file->is_locked != 0);
    file->is_locked = 1;

    int offset = file->ptr_local + sizeof(node_t);
    int nblocks_real = offset / BLOCK_DATA_SIZE; // get number of real blocks.
    int off_crt_block = offset - nblocks_real * BLOCK_DATA_SIZE;

    block_t *crt_block = get_block_by_index(file->ptr_global / BLOCK_SIZE);

    // set new size
    if (file->ptr_local + size > file->node->size) {
        file->node->size += file->ptr_local + size - file->node->size;
    }

    // update node size
    if (file->ptr_global / BLOCK_SIZE == file->node->first_block) {
        // current block is the first one
        memcpy(crt_block->data, file->node, sizeof(node_t));
    } else {
        // find first block
        block_t *first_blk = get_block_by_index(file->node->first_block);
        memcpy(first_blk->data, file->node, sizeof(node_t));

        ata_write_bytes(file->node->first_block * BLOCK_SIZE, BLOCK_SIZE, (uint8_t *) first_blk);
        kfree(first_blk);
    }

    // write single block
    if (size <= BLOCK_DATA_SIZE - off_crt_block) {
        memcpy(crt_block->data + off_crt_block, buffer, size);

        ata_write_bytes(
            (file->ptr_global / BLOCK_SIZE) * BLOCK_SIZE,
            BLOCK_SIZE, (uint8_t *) crt_block
        );

        file->ptr_global += size;
        file->ptr_local += size;
        
        kfree(crt_block);

        file->is_locked = 0;
        return 0;
    }

    // first block
    memcpy(crt_block->data + off_crt_block, buffer, BLOCK_DATA_SIZE - off_crt_block);

    // for efficiency reasons, all the blocks that belong to a file will be placed after the previous file block.
    uint32_t next_idx = find_next_free_block(NULL, file->ptr_global / BLOCK_SIZE);
    crt_block->next = next_idx;

    ata_write_bytes(
        (file->ptr_global / BLOCK_SIZE) * BLOCK_SIZE,
        BLOCK_SIZE, (uint8_t *) crt_block
    );


    uint8_t buff[1];
    buff[0] = 0;

    // set block as not free, so its not detected by find_next_free_block
    ata_write_bytes(next_idx * BLOCK_SIZE + 4, 1, buff);
    kfree(crt_block);

    buffer += BLOCK_DATA_SIZE - off_crt_block;
    size -= BLOCK_DATA_SIZE - off_crt_block;

    int c = 0;
    int temp = size / BLOCK_DATA_SIZE;
    for (int i = 0; i < temp; i++) {
        crt_block = get_block_by_index(next_idx);

        c = next_idx; // save current index to calculate offset later.
        next_idx = find_next_free_block(NULL, next_idx);

        crt_block->attributes = 0; // remove BLOCK_FREE flag.
        crt_block->next = (size > 0) ? next_idx : 0;
        memcpy(crt_block->data, buffer, BLOCK_DATA_SIZE);

        ata_write_bytes(c * BLOCK_SIZE, BLOCK_SIZE, (uint8_t *) crt_block);

        // mark next block as not free.
        ata_write_bytes(next_idx * BLOCK_SIZE + 4, 1, buff); 

        buffer += BLOCK_DATA_SIZE;
        size -= BLOCK_DATA_SIZE;

        kfree(crt_block);
    }

    // write last block
    if (size > 0) {
        crt_block = get_block_by_index(next_idx);

        crt_block->attributes = 0;
        crt_block->next = 0;
        memcpy(crt_block->data, buffer, size);

        ata_write_bytes(next_idx * BLOCK_SIZE, BLOCK_SIZE, (uint8_t *) crt_block);
    }

    file->is_locked = 0;
    return 0;
}
