#include "stdafx.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
FILE _iob[3] = {
    NULL,
    NULL,
    NULL
};

FILE *__iob_func() {
    _iob[0] = *stdin;
    _iob[1] = *stdout;
    _iob[2] = *stderr;
    return _iob;
}
#endif
