#include "scattershot/util.h"
#include "scattershot/settings.h"

char dataMap[8192] = 
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 ...................................................................................................."
"  0 .................."
;


char bssMap[8192] = 
"  0 .............................XXXX...............................X..................................."
"  1 ...................................XX..............................................................."
"  2 ...................................................................................................."
"  3 ........................................................................XX..X................XX..X.."
"  4 ...................................................................................................."
"  5 ...................................................................................................."
"  6 ...................................................................................................."
"  7 ...................................................................................................."
"  8 ...................................................................................................."
"  9 ...................................................................................................."
" 10 ...................................................................................................."
" 11 ...................................................................................................."
" 12 ...................................................................................................."
" 13 ...................................................................................................."
" 14 ...................................................................................................."
" 15 ...................................................................................................."
" 16 ...................................................................................................."
" 17 ....................................................................XX.............................."
" 18 ..................................XXXXXX............................................................"
" 19 ..................................................X................................................."
" 20 ...................................................................................................."
" 21 ...................................................................................................."
" 22 ...................................................................................................."
" 23 ...................................................................................................."
" 24 ...................................................................................................."
" 25 ...................................................................................................."
" 26 ...................................................................................................."
" 27 ...................................................................................................."
" 28 ...................................................................................................."
" 29 ...................................................................................................."
" 30 ...................................................................................................."
" 31 ...................................................................................................."
" 32 ...................................................................................................."
" 33 ...................................................................................................."
" 34 ...................................................................................................."
" 35 ...................................................................................................."
" 36 ...................................................................................................."
" 37 ...................................................................................................."
" 38 ...................................................................................................."
" 39 ...................................................................................................."
" 40 ...................................................................................................."
" 41 ...................................................................................................."
" 42 ...................................................................................................."
" 43 ...................................................................................................."
" 44 ...................................................................................................."
" 45 ...................................................................................................."
" 46 ...................................................................................................."
" 47 ..........................................X........................................................."
" 48 .........................................................................................."
;

unsigned int bssfield[50];

int dataStart, dataLength, bssStart, bssLength;
int gPrint = 1, gLog = 1;
char gProgName[192] = {0};
FILE *gLogFP = NULL;

//not sure if this works
int cp(const char *from, const char *to){
#ifdef _WIN32
    return CopyFile(from, to, 0);
#else
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
#endif
}

void writeFile(char *new_, const char *base, Input *inputs, int offset, int length) {
    //FILE *fp0 = fopen(new, "r");
    //fclose(fp0);
    //if (fp0 != NULL) return;
    //TODO check if new exists
    FILE *fp1 = fopen(base, "r");
    if (!fp1) printf("cant open file 1\n");
    
    FILE *fp2 = fopen(new_, "w");
    if (!fp2) printf("cant open file 2\n");
    int i;

    for (i = 0; i < 0x400 + offset * 4; i++) {
        unsigned char a;
        fread(&a, 1, 1, fp1);
        fwrite(&a, 1, 1, fp2);
    }

    for (i = 0; i < length; i++) {
        Input in;
        in = inputs[i];
        in.b = (in.b >> 8) | (in.b << 8); // Fuck me endianness
        fwrite(&in, sizeof(Input), 1, fp2);
    }

    fclose(fp1);
    fclose(fp2);
}

void copyDll(char *new_, char *base) {
    FILE *fp1 = fopen(base, "r");
    FILE *fp2 = fopen(new_, "w");
    unsigned char a;
    while (fread(&a, 1, 1, fp1)) fwrite(&a, 1, 1, fp2);
    fclose(fp1);
    fclose(fp2);
}


//used to figure out what section of the bss and data sections change
void xorStates(const SaveState *s1, const SaveState *s2, SaveState *s3, int tid) {
    char *a = (char *)s1->data;
    char *b = (char *)s2->data;
    char *c = (char *)s3->data;
    for (int i = 0; i < dataLength; i++) {
        c[i] |= b[i] ^ a[i];
    }
    a = (char *)s1->bss;
    b = (char *)s2->bss;
    c = (char *)s3->bss;
    int found = 0;
    for (int i = 0; i < bssLength; i++) {
        if (!found && !c[i] && (b[i] ^ a[i])) {
            printf("New at off %d!\n", i / 1000);
            found = 1;
        }
        if (i % 1000 == 0) found = 0;
        c[i] |= b[i] ^ a[i];
    }
}

//used to figure out what section of the bss and data sections change
void orStates(SaveState *s1, SaveState *s2) {
    char *a = (char *)s1->data;
    char *b = (char *)s2->data;
    for (int i = 0; i < dataLength; i++) {
        a[i] = b[i] = b[i] | a[i];
    }
    a = (char *)s1->bss;
    b = (char *)s2->bss;
    for (int i = 0; i < bssLength; i++) {
        a[i] = b[i] = b[i] | a[i];
    }
}


void printfQ(const char* format, ...) {
    va_list args;
    va_start(args, format);
    if (gPrint) vprintf(format, args);
    if (gLog) {
        if (!gLogFP) {
            char logName[256] = {0};
            sprintf(logName, "%s_log.txt", gProgName);
            gLogFP = fopen(logName, "a");
        }
        vfprintf(gLogFP, format, args);
    }
    va_end(args);
}

void flushLog() {
    fflush(gLogFP);
    //fclose(gLogFP);
    //gLogFP = NULL;
}

//fifd: only called by xoro_r
static inline uint32_t rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

//fifd: random number generation utility
uint32_t xoro_r(uint64_t *s) {
	const uint32_t s0 = *s >> 32;
	uint32_t s1 = (uint32_t)(*s) ^ s0;
	*s = (uint64_t)(rotl(s0, 26) ^ s1 ^ (s1 << 9)) << 32 | rotl(s1, 13);
	return rotl(s0 * 0x9E3779BB, 5) * 5;
}

void getDllInfo(char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if (!file)
    {
        printf("file: %s\n",file_name);
        perror("Failed to open file");
        exit(1);
    }
    Elf64_Ehdr header;
    fread(&header, sizeof(header), 1, file);

    fseek(file, header.e_shoff, SEEK_SET);
    Elf64_Shdr sectionHeaders[header.e_shnum];
    fread(sectionHeaders, sizeof(Elf64_Shdr), header.e_shnum, file);

    Elf64_Shdr *strtabSection = &sectionHeaders[header.e_shstrndx];
    char *strtab = (char*) malloc(strtabSection->sh_size);
    fseek(file, strtabSection->sh_offset, SEEK_SET);
    fread(strtab, strtabSection->sh_size, 1, file);

    for (int i = 0; i < header.e_shnum; ++i)
    {
        Elf64_Shdr *section = &sectionHeaders[i];
        const char *name = strtab + section->sh_name;
        uintptr_t address = section->sh_addr;
        size_t length = section->sh_size;
        if (strcmp(name, ".data") == 0)
        {
            dataStart = address;
            dataLength = length;
        }
        if (strcmp(name, ".bss") == 0)
        {
            bssStart = address;
            bssLength = length;
        }
    }

    fclose(file);
    free(strtab);
}

void allocState(SaveState *s) {
    s->data = calloc(dataLength, 1);
    s->bss  = calloc(bssLength, 1);
}

void allocStateSmall(SaveState *s) {
    s->data = calloc(10000, 1);
    s->bss  = calloc(bssLength, 1);
}

void freeState(SaveState *s) {
    free(s->data);
    free(s->bss);
    s->data = s->bss = NULL;
}

void load(void* hDLL, SaveState *s) {
    memcpy((char *)hDLL + dataStart, (char *)s->data, dataLength);
    memcpy((char *)hDLL + bssStart, (char *)s->bss, bssLength);
}

void riskyLoad(void* hDLL, SaveState *s) {
    memcpy((char *)hDLL + dataStart + 0, (char *)s->data + 0, 100*1000);
    memcpy((char *)hDLL + dataStart + 25 * 100* 1000, (char *)s->data + 25 * 100*1000, 100000);

    memcpy((char *)hDLL + bssStart + 0, (char *)s->bss + 0, 6 * 100000);
    memcpy((char *)hDLL + bssStart + 17 * 100000, (char *)s->bss + 17 * 100000, 6 * 100000);
    memcpy((char *)hDLL + bssStart + 47 * 100000, (char *)s->bss + 47 * 100000, 100000);
}

void riskyLoadJ(void* hDLL, SaveState *s) {
    memcpy((char *)hDLL + dataStart + 0, (char *)s->data + 0, 100* 1000);
    memcpy((char *)hDLL + dataStart + 20 * 100* 1000, (char *)s->data + 20 * 100* 1000, 100000);

    memcpy((char *)hDLL + bssStart + 0, (char *)s->bss + 0, 6 * 100000);
    memcpy((char *)hDLL + bssStart + 17 * 100000, (char *)s->bss + 17 * 100000, 6 * 100000);
    memcpy((char *)hDLL + bssStart + 47 * 100000, (char *)s->bss + 47 * 100000, 100000);
}

void initFields() {
    int run = 0;
    int inx = 0;
    char lastc = '.';
    for (int i = 0; i < strlen(bssMap); i++) {
        char c = bssMap[i];
        if (c == '.' || c == 'X') {
            if(c == lastc) {
                run++;
            }
            else {
                bssfield[inx++] = run;
                run = 1;
                lastc = c;
            }
        }
    }
    bssfield[inx] = -1;
}

void printFields() {
    int inx = 0;
    while(bssfield[inx] != -1) {
        printf("%d-",bssfield[inx++]);
    }
    printf("\n");
}

void riskyLoad3(void* hDLL, SaveState *s) {
    int inx = 0;
    int off = 0;
    while(bssfield[inx] != -1) {
        int run = bssfield[inx++];
        if(inx % 2 == 0) {
            memcpy((char *)hDLL + bssStart + off*1000, (char *)s->bss + off*1000, run * 1000);
            //printf("memcpy off %d run %d\n", off, run);
        }
        off += run;
    }
}

void riskyLoad2(void* hDLL, SaveState *s) {
    int off = 0;
    int run = 0;
    int total = 0;
    //for (int i = 0; i < strlen(dataMap); i++) {
    //    if (dataMap[i] == 'X') {
    //        run += 1000;
    //    }
    //    if (dataMap[i] == '.') {
    //        if (run > 0) memcpy((char *)hDLL + dataStart + off, (char *)s->data + off, run);
    //        off += run + 1000;
    //        total += run;
    //        run = 0;
    //    }
    //}
    //if (run > 0) memcpy((char *)hDLL + dataStart + off, (char *)s->data + off, run);
    //total += run;
    
    off = 0;
    run = 0;
    for (int i = 0; i < strlen(bssMap); i++) {
        if (bssMap[i] == 'X') {
            run += 1000;
        }
        if (bssMap[i] == '.') {
            if (run > 0) {
                memcpy((char *)hDLL + bssStart + off, (char *)s->bss + off, run);
                //printf("memcpy off %d run %d\n", off, run);
            }
            off += run + 1000;
            total += run;
            run = 0;
        }
    }
    if (run > 0) memcpy((char *)hDLL + bssStart + off, (char *)s->bss + off, run);
    total += run;
    memcpy((char *)hDLL + bssStart + 42*1000+47*100*1000, (char *)s->bss + off, 1000);
}

void save(void* hDLL, SaveState *s) {
    memcpy((char *)s->data, (char *)hDLL + dataStart, dataLength);
    memcpy((char *)s->bss, (char *)hDLL + bssStart, bssLength);
}

int find_sm64so_base(struct dl_phdr_info *info, size_t size, void *arg) {
    soStruct* helper = (soStruct*)arg;
    if (strcmp(info->dlpi_name, helper->fname) == 0)
    {
        helper->base_addr=info->dlpi_addr;
        return 1;
    }
    return 0;
}

//and combine
void print_xored_states(SaveState* xorSlaves, int tid) {
    for (int ii = 0; ii < 2; ii++) { //why twice
        for (int jj = 0; jj < totThreads; jj++) {
            if (jj != tid) orStates(xorSlaves + tid, xorSlaves + jj);
        }
    }
    char *xorDat = (char *)xorSlaves[tid].data;
    char *xorBss = (char *)xorSlaves[tid].bss;
    int foundDiff = 0;
    printfQ("DATA\n");
    for (int ii = 0; ii < dataLength; ii++) {
        if (ii % 100000 == 1) printfQ("\"\n\"%3d ", ii / 100000);
        if (xorDat[ii]) foundDiff = 1;
        if (ii > 0 && ii % 1000 == 0) {
            if (foundDiff) {
                printfQ("X");
            } else {
                printfQ(".");
            }
            foundDiff = 0;
        }
    }
    printfQ("\nBSS\n");
    for (int ii = 0; ii < bssLength; ii++) {
        if (ii % 100000 == 1) printfQ("\"\n\"%3d ", ii / 100000);
        if (xorBss[ii]) foundDiff = 1;
        if (ii > 0 && ii % 1000 == 0) {
            if (foundDiff) {
                printfQ("X");
            } else {
                printfQ(".");
            }
            foundDiff = 0;
        }
    }
}

bool in_quboid(float x, float y, float z, QUBOID* qu) {
    return x <= qu->max_x && y <= qu->max_y && z <= qu->max_z && 
        x >= qu->min_x && y >= qu->min_y && z >= qu->min_z;
}

bool in_triangle(float x, float y, float z, TRIANGLE* tr) {
    if (tr->min_y > y || tr->max_y < y) return false;

    float denom = (tr->z2 - tr->z3) * (tr->x1 - tr->x3) + (tr->x3 - tr->x2) * (tr->z1 - tr->z3);
    float alpha = ((tr->z2 - tr->z3) * (x - tr->x3) + (tr->x3 - tr->x2) * (z - tr->z3)) / denom;
    float beta = ((tr->z3 - tr->z1) * (x - tr->x3) + (tr->x1 - tr->x3) * (z - tr->z3)) / denom;
    float gamma = 1 - alpha - beta;

    return 0 <= alpha && alpha <= 1 && 0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1;

}
