struct APValue {
    void *ptr;
    APValue(APInt *I) : ptr{I} { }
    APValue(APFloat *F) : ptr{F} { }
    APInt asInt() { return reinterpret_cast<APInt *>(ptr); }
    APInt asFloat() { return reinterpret_cast<APFloat *>(ptr); }
    void writeInt(APInt *I) { ptr = I; }
    void writeFloat(APFloat *F) { ptr = F; }
};
