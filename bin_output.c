#include "bin_output.h"

// Given a list of assembled lines, make binary output. 
// It is assumed that the buffer is at least 64K big.
 
size_t make_binary(const struct line *lines, unsigned char *buf) {
    const struct line *line;
    size_t idx = 0;
    
    for (line = lines; line != NULL; line=line->next_line) {
        if (line->needs_process) FATAL_ERROR("unprocessed line was passed in");
        if (line->n_bytes == 0) continue;
        
        // this is a fatal error, because the assembler is supposed to catch this
        // beforehand and give a nicer error message.
        if (idx+line->n_bytes > (1<<16)) FATAL_ERROR("tried to output more than 64k");
        memcpy(buf+idx, line->bytes, line->n_bytes);
        idx += line->n_bytes;
    }
        
    return idx; 
}
