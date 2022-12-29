//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2022)
//
//  Project #1:
//
//  September 6, 2022
//
//  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//  Jaehoon Shim (mattjs@snu.ac.kr)
//  IlKueon Kang (kangilkueon@snu.ac.kr)
//  Wookje Han (gksdnrwp@snu.ac.kr)
//  Jinsol Park (jinsolpark@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------

typedef unsigned char u8;

/* TODO: Implement this function */
inline void bitwrite(u8* dst, int idx, u8 val, int len)
{ // assuming len > 0
    int idx1, idx2;
    u8 mask1 = 0xff, mask2 = 0xff;
    idx2 = idx + len, idx1 = idx2 & 0x7ffffff8;

    if (idx1 == idx2)
    { // 1 byte simple
        mask1 >>= (8 - len);
        *(dst + (idx >> 3)) &= ~mask1; 
        *(dst + (idx >> 3)) |= val & mask1;
    }
    else if (idx1 > idx)
    { // 2 byte
        // byte #N
        mask1 >>= (idx & 7);
        *(dst + (idx >> 3)) &= ~mask1;
        *(dst + (idx >> 3)) |= (val >> (idx2 & 7)) & mask1;
        // byte #N+1
        mask2 <<= (8 - (idx2 & 7));
        *(dst + (idx2 >> 3)) &= ~mask2;
        *(dst + (idx2 >> 3)) |= (val << (8 - (idx2 & 7))) & mask2;
    }
    else
    { // 1 byte complex
        mask1 >>= (idx & 7);
        mask2 >>= (idx2 & 7);
        mask1 &= ~mask2;
        *(dst + (idx >> 3)) &= ~mask1;
        *(dst + (idx >> 3)) |= (val << (8 - (idx2 & 7))) & mask1;
    }
}

inline u8 filter(const u8* src, int width, int i, int j) {
    int avg = 0;

    if (i == 0)
        if (j == 0) return *src;
        else return *(src + j) - *(src + j-1);
    else if (j == 0)
        return *(src + i * width) - *(src + (i-1) * width);
    else
    {
        avg = (int)*(src + (i-1) * width + (j-1))
        + *(src + (i-1) * width + j)
        + *(src + i * width + (j-1));
        avg /= 3;

        return *(src + i * width + j) - (u8)avg;
    }
}

int encode(const u8 *src, int width, int height, u8 *result)
{
    // bitindex, base, max
    int idx = 0;
    u8 base = *src, max = base, delta;
    int n_bits;

    if (width == 0 || height == 0)
        return 0;

    for (int i = 0; i < height; i++)
    {
        base = 0xff, max = 0x00;
        // phase 1
        for (int j = 0; j < width; j++)
        {
            delta = filter(src, width, i, j);
            base = base > delta ? delta : base;
            max = max < delta ? delta : max;
        }
        // phase 2 : calculate n_bits
        delta = max - base;
        if (delta & 0x80) n_bits = 8;
        else if (delta & 0x40) n_bits = 7;
        else if (delta & 0x20) n_bits = 6;
        else if (delta & 0x10) n_bits = 5;
        else if (delta & 0x08) n_bits = 4;
        else if (delta & 0x04) n_bits = 3;
        else if (delta & 0x02) n_bits = 2;
        else if (delta & 0x01) n_bits = 1;
        else n_bits = 0;
        // base : 8 bits
        bitwrite(result, idx, base, 8);
        idx += 8;
        // n_bits : 4 bits
        bitwrite(result, idx, n_bits, 4);
        idx += 4;
        // deltas : n(i) bits x width
        if (n_bits != 0)
        {
            for (int j = 0; j < width; j++)
            {
                delta = filter(src, width, i, j) - base;
                bitwrite(result, idx, delta, n_bits);
                idx += n_bits;
            }
        }
    }
    // padding
    while (idx & 7)
        bitwrite(result, idx++, 0, 1);
    
    return idx >> 3;
}
