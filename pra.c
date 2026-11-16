#include <string.h>
#include <pra.h>

#define MAX_FRAMES 256

static int total_frames; // total frames when program runs
static int next_free_frame; // next unused frame
static int fifo_spot; // where idx fifo is currently at
static int frame_page[MAX_FRAMES];
static int pending_frame; // holds the chosen frame until pra_used can get its data

// setup page replacement (in case we need to implement LRU used but for now I will just implement FIFO - Adriel)
void pra_init(int frames, char *alg, int*pages, int count) {
    int i;

    (void)alg;
    (void)pages;
    (void)count;

    total_frames = frames;
    next_free_frame = 0;
    fifo_spot = 0;
    pending_frame = -1;

    // mark every frame as "empty"
    for (i = 0; i < MAX_FRAMES; i++) {
        frame_page[i] = -1;
    }

}

// return an unused frame, if not unused frame exists evict the oldest page (this is using our FIFO algorithm)
int pra_get_frame(int pos, int *evicted_page) {
    int frame;

    (void)pos;

    // check if there is a frame free
    if (next_free_frame < total_frames) {
        frame = next_free_frame;
        next_free_frame ++;

        *evicted_page = -1;
    } 

    //otherwise use FIFO and use that frame spot
    else {
        frame = fifo_spot;
        *evicted_page = frame_page[frame];

        fifo_spot = (fifo_spot + 1) % total_frames;
    }

    // memsim should load req page into next frame
    pending_frame = frame;

    return frame;
}

void pra_used(int page, int pos) {

    (void)pos;

    if (pending_frame != -1) {
        frame_page[pending_frame] = page;
        pending_frame = -1;
    }
}


