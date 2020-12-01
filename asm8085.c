#include "asm8085.h"

char startdir[PATH_MAX]; // this stores the directory in which the command is run

void help() {
    
    printf("asm8085 v" VERSION " (build " BUILD ")\n\n");
    printf("usage: asm8085 -h | [-o output] [-l file] source\n");
    printf("\t-h       \tShow help\n");
    printf("\t-o <file>\tSet output file\n");
    printf("\t-l <file>\tWrite listing\n");
    
    exit(0);
}

// Replace extension by '.bin'
char *make_bin_file(const char *fname) {
    char *bin, *dot, *slash; 
    bin = copy_string(fname);
    bin = realloc(bin, strlen(bin)+5); // make sure there is room

    // Find last slash and dot
    slash = strrchr(bin, '/');
    dot = strrchr(bin, '.');
    
    if (dot == NULL || slash > dot) {
        // No dot, or slash after dot: append ".bin" 
        strcat(bin, ".bin");
    } else {
        // Dot, replace extension by ".bin"
        strcpy(dot, ".bin");
    }
    
    return bin;
}

int main(int argc, char **argv) {
    int c;
    char *inp=NULL, *outp=NULL, *list=NULL; 
    unsigned char *mem;
    FILE *outf, *listf; 
    size_t outsize;
    
    // Allocate 64K for binary output
    if ((mem = malloc(65536)) == NULL) {
        fprintf(stderr, "memory allocation failure.\n");
        exit(1);
    }
    
    // Store current directory
    if (getcwd(startdir, PATH_MAX) == NULL) {
        fprintf(stderr, "cannot get wd: %s\n", strerror(errno));
        exit(1);
    }
    
    // Handle arguments
    while((c = getopt(argc, argv, "ho:l:")) != -1) {
        switch(c) {
            case '?':
                if (optopt == 'o' || optopt == 'l') {
                    fprintf(stderr, "-%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option: -%c\n", optopt);
                }
                
                exit(1);
            
            case 'h': help(); break;
            case 'o': outp = optarg; break;
            case 'l': list = optarg; break;
        }
    }
    
    if (optind != argc-1) {
        fprintf(stderr, "asm8085: no source file given\n");
        exit(1);
    }
    
    inp = argv[optind];
    
    // If no output file is given, change the input extension into '.bin'
    if (outp == NULL) outp = make_bin_file(inp);
    
    // Try to assemble the file. 
    struct asmstate *state = init_asmstate();
    
    struct line *lines = assemble(state, inp);
    if (lines == NULL) exit(1);
    
    if (!complete(state, lines)) exit(2);
    
    // Restore the old working directory
    if (chdir(startdir) == -1) {
        fprintf(stderr, "cannot restore wd: %s\n", strerror(errno));
        exit(1);
    }
    
    // Write the binary file
    if ((outf = fopen(outp, "w")) == NULL) {
        fprintf(stderr, "cannot open %s for writing: %s\n", outp, strerror(errno));
        exit(1);
    }
    
    outsize = make_binary(lines, mem);
    if (fwrite(mem, 1, outsize, outf) != outsize) {
        fprintf(stderr, "write error: %s\n", strerror(errno));
        exit(1);
    }
    
    fclose(outf);
    
    // Write the listing if the user wanted one
    if (list != NULL) {
        if ((listf = fopen(list, "w")) == NULL) {
            fprintf(stderr, "cannot open %s for writing: %s\n", list, strerror(errno));
            exit(1);
        }
        
        write_listing(listf, state, lines);
        fclose(listf);
    }
    
    return 0;
    
}