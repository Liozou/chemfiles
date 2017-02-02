#include <chemfiles.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    // [example]
    CHFL_FRAME* frame = chfl_frame(42);

    uint64_t atoms = 0;
    chfl_frame_atoms_count(frame, &atoms);
    assert(atoms == 42);

    chfl_frame_remove(frame, 37);
    chfl_frame_remove(frame, 30);
    chfl_frame_remove(frame, 15);

    chfl_frame_atoms_count(frame, &atoms);
    assert(atoms == 39);

    chfl_frame_free(frame);
    // [example]
    return 0;
}
