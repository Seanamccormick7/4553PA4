#include <string.h>
#include "pra.h"

#define MAX_FRAMES 256

static int total_frames; // total frames when program runs
static int next_free_frame; // next unused frame
static int fifo_spot; // where idx fifo is currently at
static int frame_page[MAX_FRAMES];
static int pending_frame; // holds the chosen frame until pra_used can get its data
static char algorithm[4];
static int last_used[MAX_FRAMES];


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

    // put users choice of algorithm into our global varible algorithm 
    strcpy(algorithm, alg);

    // mark every frame as "empty" and mark the last_used array as "none are last used"
    for (i = 0; i < MAX_FRAMES; i++) {
        frame_page[i] = -1;
        last_used[i] = -1;
    }

}

// return an unused frame, if not unused frame exists evict the oldest page (this is using our FIFO algorithm)
int pra_get_frame(int pos, int *evicted_page) {
    int frame;
    int i;

    // check if there is a frame free
    if (next_free_frame < total_frames) {
        frame = next_free_frame;
        next_free_frame ++;

        *evicted_page = -1;
    } 

    else if (strcmp(algorithm, "lru") == 0) {
        // find the frame that page was used the least recently
        frame = 0;

        // traverse and find the lowest last_used position
        for (i = 1; i < total_frames; i++) {
            if (last_used[i] < last_used[frame]) {
                frame = i;
            }
        }
        *evicted_page = frame_page[frame];
    }

    // frame that was laoded earliest
    else if (strcmp(algorithm, "fifo") == 0) {
        frame = fifo_spot;
        *evicted_page = frame_page[frame];

        fifo_spot = (fifo_spot + 1) % total_frames;
    }

    pending_frame = frame;
    return frame;
}

void pra_used(int page, int pos) {
    int i;

    // when a page fault loads page into pending frame, reset pending to none (in our case -1)
    if (pending_frame != -1) {
        frame_page[pending_frame] = page;
        last_used[pending_frame] = pos;
        pending_frame = -1;
        return;
    }

    //if we are using lru, we have to record the page that was last used
    if (strcmp(algorithm, "lru") == 0) {
        for (i = 0; i < total_frames; i++) {
            if (frame_page[i] == page) {
                last_used[i] = pos;
                break;
            }
        }
    }
    
}


