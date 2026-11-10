#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pra.h"

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define TLB_SIZE 16
#define MAX_ADDRESSES 10000

int tlb_page[TLB_SIZE];
int tlb_frame[TLB_SIZE];
int tlb_next = 0;

int page_table[NUM_PAGES];

signed char memory[NUM_PAGES * PAGE_SIZE];
FILE *backing;

int addresses[MAX_ADDRESSES];
int pages[MAX_ADDRESSES];
int count = 0;

int tlb_lookup(int page)
{
    int i;
    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb_page[i] == page) {
            return tlb_frame[i];
        }
    }
    return -1;
}

void tlb_add(int page, int frame)
{
    tlb_page[tlb_next] = page;
    tlb_frame[tlb_next] = frame;
    tlb_next = (tlb_next + 1) % TLB_SIZE;
}

void tlb_remove(int page)
{
    int i;
    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb_page[i] == page) {
            tlb_page[i] = -1;
        }
    }
}

void read_page(int page, int frame)
{
    fseek(backing, page * PAGE_SIZE, SEEK_SET);
    fread(&memory[frame * PAGE_SIZE], 1, PAGE_SIZE, backing);
}

void read_addresses(char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Could not open %s\n", filename);
        exit(1);
    }
    while (count < MAX_ADDRESSES && fscanf(f, "%d", &addresses[count]) == 1) {
        pages[count] = (addresses[count] & 0xFFFF) >> 8;
        count++;
    }
    fclose(f);
}

int main(int argc, char *argv[])
{
    int frames = 256;
    char *alg = "fifo";
    int faults = 0;
    int tlb_hits = 0;
    int i;

    if (argc < 2) {
        printf("Usage: ./memsimulator <reference-sequence-file.txt> <FRAMES> <PRA>\n");
        return 1;
    }
    if (argc > 2) {
        frames = atoi(argv[2]);
    }
    if (argc > 3) {
        alg = argv[3];
    }
    if (frames <= 0 || frames > 256) {
        printf("FRAMES must be between 1 and 256\n");
        return 1;
    }
    if (strcmp(alg, "fifo") != 0 && strcmp(alg, "lru") != 0 && strcmp(alg, "opt") != 0) {
        printf("PRA must be fifo, lru, or opt\n");
        return 1;
    }

    read_addresses(argv[1]);

    backing = fopen("BACKING_STORE.bin", "rb");
    if (backing == NULL) {
        printf("Could not open BACKING_STORE.bin\n");
        return 1;
    }

    for (i = 0; i < TLB_SIZE; i++) {
        tlb_page[i] = -1;
    }
    for (i = 0; i < NUM_PAGES; i++) {
        page_table[i] = -1;
    }
    pra_init(frames, alg, pages, count);

    for (i = 0; i < count; i++) {
        int page = pages[i];
        int offset = addresses[i] & 0xFF;
        int frame;
        int evicted;
        int physical;

        frame = tlb_lookup(page);
        if (frame != -1) {
            tlb_hits++;
        } else {
            frame = page_table[page];
            if (frame == -1) {
                faults++;
                frame = pra_get_frame(i, &evicted);
                if (evicted != -1) {
                    page_table[evicted] = -1;
                    tlb_remove(evicted);
                }
                read_page(page, frame);
                page_table[page] = frame;
            }
            tlb_add(page, frame);
        }
        pra_used(page, i);

        physical = frame * PAGE_SIZE + offset;
        printf("Virtual address: %d Physical address: %d Value: %d\n",
               addresses[i], physical, memory[physical]);
    }

    printf("Translated addresses: %d\n", count);
    printf("Page faults: %d\n", faults);
    printf("Page fault rate: %.2f%%\n", 100.0 * faults / count);
    printf("TLB hits: %d\n", tlb_hits);
    printf("TLB hit rate: %.2f%%\n", 100.0 * tlb_hits / count);

    fclose(backing);
    return 0;
}
