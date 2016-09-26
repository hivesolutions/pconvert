__kernel void source_over(
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

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);
    float af = abf + atf * (1.0f - abf);

    r = af == 0.0f ? 0 : (uchar) ((rb * abf + rt * atf * (1.0f - abf)) / af);
    g = af == 0.0f ? 0 : (uchar) ((gb * abf + gt * atf * (1.0f - abf)) / af);
    b = af == 0.0f ? 0 : (uchar) ((bb * abf + bt * atf * (1.0f - abf)) / af);
    a = max((uchar) 0, min((uchar) 255, (uchar) (af * 255.0f)));

    r = max((uchar) 0, min((uchar) 255, r));
    g = max((uchar) 0, min((uchar) 255, g));
    b = max((uchar) 0, min((uchar) 255, b));

    bottom[index] = r;
    bottom[index + 1] = g;
    bottom[index + 2] = b;
    bottom[index + 3] = a;
}
