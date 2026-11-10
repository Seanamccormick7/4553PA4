#ifndef PRAH
#define PRAH

void pra_init(int frames, char *alg, int *pages, int count);
int pra_get_frame(int pos, int *evicted_page);
void pra_used(int page, int pos);

#endif
