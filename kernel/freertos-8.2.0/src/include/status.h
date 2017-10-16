typedef struct status_list_tbl {
	const char *name;               /* identifier string */
	const char *desc;
	unsigned short	id;
	unsigned int format;		/* 1 = Decimal, 2 = hex */
	unsigned int datatype;		/* 0 = char, 1 = unsigned char, 2 = short, 3 = unsigned short, 4 = int, 5 = unsigned int */
	int (*value)(unsigned short);
} status_list_t;
