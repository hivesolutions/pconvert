__kernel void multiplicative(
    __global unsigned char *bottom,
    __global read_only unsigned char *top,
    const unsigned int count
) {
    int index = get_global_id(0) * 4;
    if(index > count - 3) { return; }

    unsigned char rb, gb, bb, ab;
    unsigned char rt, gt, bt, at;
    unsigned char r, g, b, a;

    rb = bottom[index];
    gb = bottom[index + 1];
    bb = bottom[index + 2];
    ab = bottom[index + 3];

    rt = top[index];
    gt = top[index + 1];
    bt = top[index + 2];
    at = top[index + 3];

    float atf = 1.0f * (at / 255.0f);

    r = (uchar) (rb * (1 - atf) + rt * atf);
    g = (uchar) (gb * (1 - atf) + gt * atf);
    b = (uchar) (bb * (1 - atf) + bt * atf);
    a = max(0, min(255, at + ab));

    r = max((uchar) 0, min((uchar) 255, r));
    g = max((uchar) 0, min((uchar) 255, g));
    b = max((uchar) 0, min((uchar) 255, b));

    bottom[index] = r;
    bottom[index + 1] = g;
    bottom[index + 2] = b;
    bottom[index + 3] = a;
}
