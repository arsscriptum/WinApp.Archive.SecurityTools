struct XXX_t{
 unsigned char* stringBuf;
 unsigned long stringLength;
};

#define XXX_TYPE (ULONG)64001
#define XXX_IOCTL_BASE (USHORT)4004

enum {
	XXX_RDTSC = XXX_IOCTL_BASE
};

#define IOCTL_XXX_RDTSC CTL_CODE(XXX_TYPE, XXX_RDTSC, METHOD_BUFFERED, FILE_WRITE_DATA)
