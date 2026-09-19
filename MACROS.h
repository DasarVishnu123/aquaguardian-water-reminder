//MACROS.h
#define WRITEBYTE(WORD,STARTBIT,BYTE)\
        WORD=((WORD&(u32)~0xFF<<STARTBIT)|(BYTE<<STARTBIT));
#define READBIT(WORD,BIT) (((WORD) >> (BIT)) & 1)
#define READNIBBLE(data, pos) (((data) >> (pos)) & 0x0F)
#define WRITENIBBLE(data, pos, val) \
    (data = (data & ~(0x0F << pos)) | ((val & 0x0F) << pos))


