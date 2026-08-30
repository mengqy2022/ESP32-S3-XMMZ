idf_component_register(
    SRCS
        "src/bitstream.c"
        "src/buffers.c"
        "src/dct32.c"
        "src/dequant.c"
        "src/dqchan.c"
        "src/huffman.c"
        "src/hufftabs.c"
        "src/imdct.c"
        "src/mp3dec.c"
        "src/mp3tabs.c"
        "src/polyphase.c"
        "src/scalfact.c"
        "src/stproc.c"
        "src/subband.c"
        "src/trigtabs.c"
        "src/utils/helix_memory.cpp"
    INCLUDE_DIRS "src" "src/utils"
    REQUIRES log
)
