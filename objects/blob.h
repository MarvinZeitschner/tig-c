#include "strbuf.h"
#include "object.h"

struct blob {
    struct object object;
    unsigned long size;
    struct strbuf data;
};
