#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>
#include "os.h"

uint64_t page_table_query(uint64_t pt, uint64_t vpn);
int validation_bit(uint64_t address);
int masking(uint64_t address, int level);
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    uint64_t* address = phys_to_virt((pt << 12)); //first address points to the page table root
    int level = 0;
    for (int i = 0; i < 5; ++i) {
        level = masking(vpn,i); //gets the specific index of each level
        // below- how to cast void* to uint64 struct, used in the previous line.
        // https://stackoverflow.com/questions/15505154/cast-void-pointer-to-uint64-t-array-in-c
        if (!validation_bit(address[level])){
            return NO_MAPPING; //valid bit in address is off
        }
        if (i <= 3){
            address = phys_to_virt(address[level] - 1); //if we got here than valid bit = 1,
            // and we need to go to the next page level
        }
    }
    return (address[level] >> 12); // return value is the ppn there for needs to be shifted.
}

/* this function returns the least significant bit */
int validation_bit(uint64_t address){
    return (address & 1);
}

/* this function split the vpn by levels */
int masking(uint64_t address, int level){
    uint64_t base = 0x1ff; //hex representation of 111111111 in base 2 (9 ones)
    return  (address >> (36 - 9 * level)) & base;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    if (ppn == NO_MAPPING){ // destroy virtual memory mappings in a page table
        uint64_t* address = phys_to_virt((pt << 12)); //first address points to the page table root
        int level = 0;
        for (int i = 0; i < 5; ++i) {
            level = masking(vpn,i); //gets the specific index of each level
            // below- how to cast void* to uint64 struct, used in the previous line.
            // https://stackoverflow.com/questions/15505154/cast-void-pointer-to-uint64-t-array-in-c
            if (!validation_bit(address[level])){
                return; // this pte is not valid and we are done
            }
            if (i <= 3){
                address = phys_to_virt(address[level] - 1); //if we got here than valid bit = 1,
                // and we need to go to the next page level
            }
        }
        //if we got here than there is mapping, and we are on the last level
        address[level] -= 1 ; // we just need to change the  valid bit to 0.
        return;
    }

    else{ //create virtual memory mappings in a page table
        uint64_t* address = phys_to_virt((pt << 12)); //first address points to the page table root
        int level = 0;
        for (int i = 0; i < 5; ++i) {
            level = masking(vpn,i); //gets the specific index of each level
            // below- how to cast void* to uint64 struct, used in the previous line.
            // https://stackoverflow.com/questions/15505154/cast-void-pointer-to-uint64-t-array-in-c
            if (!validation_bit(address[level])){ //need to create physical page for this vpn
                uint64_t new_address = alloc_page_frame();
                new_address = (new_address << 12);
                new_address += 1;
                address[level] = new_address;
            }
            if (i <= 3){
                address = phys_to_virt(address[level] - 1); //if we got here than valid bit = 1,
                // and we need to go to the next page level
            }
        }
        address[level] = ((ppn << 12) + 1);
        return;
    }
}

