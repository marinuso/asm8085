// Test if binary file matches assembly

#define MEMSZ 0x10000
#define B(x) ((x)%MEMSZ)

#define BIN_FILE_TEST(name) \
TEST(file_##name \
, /*startup*/ \
    unsigned char *match = NULL; \
    unsigned char *outbin = NULL; \
    struct line *input = NULL; \
    struct asmstate *state = NULL; \
    FILE *matchfile = NULL; \
    size_t filesize; \
    size_t outsize; \
    size_t i; \
, /*shutdown*/ \
    free(match); \
    free(outbin); \
    free_asmstate(state);\
    if(input) free_line(input, TRUE); \
    if(matchfile) fclose(matchfile); \
, /*test*/ \
{ \
    /* memory */ \
    match = malloc(MEMSZ); \
    outbin = malloc(MEMSZ); \
    /* file names */ \
    char *binfile = "test_inputs/" #name ".bin"; \
    char *asmfile = "test_inputs/" #name ".asm"; \
    /* read the binary file to match the assembly output to */ \
    if (!(matchfile = fopen(binfile, "r"))) FAIL("could not open binary input file %s", binfile); \
    filesize = fread(match, sizeof(char), MEMSZ, matchfile); \
    /* parse, assemble, etc. the asm input file */ \
    state = init_asmstate(); \
    if (!(input = assemble(state, asmfile))) FAIL("assembly failed on %s", asmfile); \
    if (!complete(state, input)) FAIL("complete() failed"); \
    outsize = make_binary(input, outbin); \
    /* see if they match and fail if they don't */ \
    if (filesize != outsize) FAIL("size does not match - %zu != %zu", filesize, outsize); \
    for (i=0; i<filesize; i++) { \
        if (outbin[i] != match[i]) { \
            FAIL("mismatch at %zx: \n\t  asm: %x %x %x %x\n\tmatch: %x %x %x %x", \
                i, \
                outbin[B(i)], outbin[B(i+1)], outbin[B(i+2)], outbin[B(i+3)], \
                match[B(i)],  match[B(i+1)],  match[B(i+2)],  match[B(i+3)] \
            ); \
        } \
    } \
})

BIN_FILE_TEST(bytetest)
BIN_FILE_TEST(labeltest)
        
                
    
    
    