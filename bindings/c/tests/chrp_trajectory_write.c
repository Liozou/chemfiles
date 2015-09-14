#include "chemharp.h"

// Force NDEBUG to be undefined
#undef NDEBUG

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Read a whole file at once
static char* read_whole_file(const char* name);

const char* expected_content = "4\n"
                               "Written by Chemharp\n"
                               "He 1 2 3\n"
                               "He 1 2 3\n"
                               "He 1 2 3\n"
                               "He 1 2 3\n"
                               "6\n"
                               "Written by Chemharp\n"
                               "He 4 5 6\n"
                               "He 4 5 6\n"
                               "He 4 5 6\n"
                               "He 4 5 6\n"
                               "He 4 5 6\n"
                               "He 4 5 6\n";

int main(){
    float pos[4][3];
    for (unsigned i=0; i<4; i++)
        for (unsigned j=0; j<3; j++)
            pos[i][j] = j + 1;

    CHRP_TOPOLOGY* top = chrp_topology();
    CHRP_ATOM* atom = chrp_atom("He");
    for (unsigned i=0; i<4; i++)
        assert(!chrp_topology_append(top, atom));

    CHRP_FRAME* frame = chrp_frame(0);
    assert(!chrp_frame_set_positions(frame, pos, 4));
    assert(!chrp_frame_set_topology(frame, top));

    CHRP_TRAJECTORY* file = chrp_trajectory_open("test-tmp.xyz", "w");
    assert(!chrp_trajectory_write(file, frame));

    float pos_2[6][3];
    for (unsigned i=0; i<6; i++)
        for (unsigned j=0; j<3; j++)
            pos_2[i][j] = j + 4;

    assert(!chrp_topology_append(top, atom));
    assert(!chrp_topology_append(top, atom));

    assert(!chrp_frame_set_positions(frame, pos_2, 6));
    assert(!chrp_frame_set_topology(frame, top));

    assert(!chrp_atom_free(atom));
    assert(!chrp_topology_free(top));

    assert(!chrp_trajectory_write(file, frame));
    assert(!chrp_trajectory_close(file));

    char* content = read_whole_file("test-tmp.xyz");
    assert(strcmp(content, expected_content) == 0);
    free(content);

    remove("test-tmp.xyz");

    return EXIT_SUCCESS;
}


static char* read_whole_file(const char* name) {
    char *buffer = NULL;
    FILE *file = fopen(name, "rb");

    if (file != NULL){
        fseek(file, 0L, SEEK_END);
        long s = ftell(file);
        rewind(file);
        buffer = (char*)malloc(sizeof(char)*(s+1));

        if (buffer != NULL){
            fread(buffer, s, 1, file);
            buffer[s] = '\0';
        }
    }

    fclose(file);
    return buffer;
}
