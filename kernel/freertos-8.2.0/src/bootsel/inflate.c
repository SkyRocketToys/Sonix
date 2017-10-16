/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
   [The history has been moved to the file ChangeLog.]
 */

/*
   Inflate deflated (PKZIP's method 8 compressed) data.  The compression
   method searches for as much of the current string of bytes (up to a
   length of 258) in the previous 32K bytes.  If it doesn't find any
   matches (of at least length 3), it codes the next byte.  Otherwise, it
   codes the length of the matched string and its distance backwards from
   the current position.  There is a single Huffman code that codes both
   single bytes (called "literals") and match lengths.  A second Huffman
   code codes the distance information, which follows a length code.  Each
   length or distance code actually represents a base value and a number
   of "extra" (sometimes zero) bits to get to add to the base value.  At
   the end of each deflated block is a special end-of-block (EOB) literal/
   length code.  The decoding process is basically: get a literal/length
   code; if EOB then done; if a literal, emit the decoded byte; if a
   length then get the distance and emit the referred-to bytes from the
   sliding window of previously emitted data.

   There are (currently) three kinds of inflate blocks: stored, fixed, and
   dynamic.  The compressor deals with some chunk of data at a time, and
   decides which method to use on a chunk-by-chunk basis.  A chunk might
   typically be 32K or 64K.  If the chunk is uncompressible, then the
   "stored" method is used.  In this case, the bytes are simply stored as
   is, eight bits per byte, with none of the above coding.  The bytes are
   preceded by a count, since there is no longer an EOB code.

   If the data is compressible, then either the fixed or dynamic methods
   are used.  In the dynamic method, the compressed data is preceded by
   an encoding of the literal/length and distance Huffman codes that are
   to be used to decode this block.  The representation is itself Huffman
   coded, and so is preceded by a description of that code.  These code
   descriptions take up a little space, and so for small blocks, there is
   a predefined set of codes, called the fixed codes.  The fixed method is
   used if the block codes up smaller that way (usually for quite small
   chunks), otherwise the dynamic method is used.  In the latter case, the
   codes are customized to the probabilities in the current block, and so
   can code it much better than the pre-determined fixed codes.
 
   The Huffman codes themselves are decoded using a mutli-level table
   lookup, in order to maximize the speed of decoding plus the speed of
   building the decoding tables.  See the comments below that precede the
   lbits and dbits tuning parameters.
 */


/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarly, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */

// #ifdef RCSID
// static char rcsid[] = "$Id: inflate.c,v 0.14 1993/06/10 13:27:04 jloup Exp $";
// #endif

//#include <sys/types.h>

//#include "tailor.h"

// #if defined(STDC_HEADERS) || !defined(NO_STDLIB_H)
// //#  include <stdlib.h>
// #endif
/* Standard includes. */
#include "gzip.h"
#define slide window





/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */
struct huft {
  uch e;                /* number of extra bits or operation */
  uch b;                /* number of bits in this code or subcode */
  union {
    ush n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};


/* Function prototypes */
int huft_build OF((unsigned *, unsigned, unsigned, ush *, ush *,
                   struct huft **, int *));
int huft_free OF((struct huft *));
int inflate_codes OF((struct huft *, struct huft *, int, int));
int inflate_stored OF((void));
int inflate_fixed OF((void));
int inflate_dynamic OF((void));
int inflate_block OF((volatile int *));
int inflate OF((void));


/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */
/* unsigned wp;             current position in slide */
#define wp outcnt
#define flush_output(w) (wp=(w),flush_window())

/* Tables for deflate from PKZIP's appnote.txt. */
volatile static unsigned border[] = {    /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
volatile static ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
        /* note: see note #13 above about the 258 in this list. */
volatile static ush cplext[] = {         /* Extra bits for literal codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
volatile static ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
volatile static ush cpdext[] = {         /* Extra bits for distance codes */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};



/* Macros for inflate() bit peeking and grabbing.
   The usage is:
   
        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the beginning of a
   routine that uses these macros from a global bit buffer and count.

   If we assume that EOB will be the longest code, then we will never
   ask for bits with NEEDBITS that are beyond the end of the stream.
   So, NEEDBITS should not read any more bytes than are needed to
   meet the request.  Then no bytes need to be "returned" to the buffer
   at the end of the last block.

   However, this assumption is not true for fixed blocks--the EOB code
   is 7 bits, but the other literal/length codes can be 8 or 9 bits.
   (The EOB code is shorter than other codes because fixed blocks are
   generally short.  So, while a block always has an EOB, many other
   literal/length codes have a significantly lower probability of
   showing up at all.)  However, by making the first table have a
   lookup of seven bits, the EOB code will be found in that first
   lookup, and so will not require that too many bits be pulled from
   the stream.
 */

volatile ulg bb;                         /* bit buffer */
volatile unsigned bk;                    /* bits in bit buffer */

volatile ush mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#ifdef CRYPT
  uch cc;
#  define NEXTBYTE() \
     (decrypt ? (cc = get_byte(), zdecode(cc), cc) : get_byte())
#else
#  define NEXTBYTE()  (uch)get_byte()
#endif

#define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}


/*
   Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   is not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.  Subsequent tables are also less than or equal to
   those sizes.  These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */


volatile int lbits = 9;          /* bits in base literal/length lookup table */
volatile int dbits = 6;          /* bits in base distance lookup table */


/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */


volatile unsigned hufts;         /* track memory usage */

//volatile unsigned char locmem[1048576];
volatile unsigned int locmem_indx = 0;


/* delezue add */
volatile unsigned char inbuf[INBUFSIZ +INBUF_EXTRA];
volatile unsigned char outbuf[INBUFSIZ +INBUF_EXTRA];
volatile unsigned short d_buf[DIST_BUFSIZE];
volatile unsigned char window[2L*WSIZE];
volatile unsigned short tab_prefix[1L<<BITS];

    /* local variables */

//int ascii = 0;        /* convert end-of-lines to local OS conventions */
volatile int to_stdout = 0;    /* output to stdout (-c) */
//int decompress = 0;   /* decompress (-d) */
//int force = 0;        /* don't ask questions, compress links (-f) */
volatile int no_name = -1;     /* don't save or restore the original file name */
//int no_time = -1;     /* don't save or restore the original file time */
//int recursive = 0;    /* recurse through directories (-r) */
volatile int list = 0;         /* list the file contents (-l) */
//int verbose = 0;      /* be verbose (-v) */
//int quiet = 0;        /* be very quiet (-q) */
//int do_lzw = 0;       /* generate output compatible with old compress (-Z) */
volatile int test = 0;         /* test .gz file integrity */
//int foreground;       /* set if program run in foreground */
//char *progname;       /* program name */
//int maxbits = BITS;   /* max bits per code for LZW */
volatile int method = DEFLATED;/* compression method */
//int level = 6;        /* compression level */
//int exit_code = OK;   /* program exit code */
//int save_orig_name;   /* set if original name must be saved */
//int last_member;      /* set for .zip and .Z files */
volatile int part_nb;          /* number of parts in .gz file */
//long time_stamp;      /* original time stamp (modification time) */
//long ifile_size;      /* input file size, -1 for devices (debug only) */
//char *env;            /* contents of GZIP env variable */
//char **args = NULL;   /* argv pointer if GZIP env variable defined */
//char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
//int  z_len;           /* strlen(z_suffix) */


volatile long bytes_in;             /* number of input bytes */
volatile long bytes_out;            /* number of output bytes */
volatile long total_in = 0;         /* input bytes for all files */
volatile long total_out = 0;        /* output bytes for all files */
//char ifname[MAX_PATH_LEN]; /* input file name */
volatile char ofname[64];
//char ofname[MAX_PATH_LEN]; /* output file name */
//int  remove_ofname = 0;    /* remove output file on error */
//struct stat istat;         /* status for input file */
//volatile unsigned int  ifd;                  /* input file descriptor -- inp ddr address */   
//volatile unsigned int  ofd;                  /* output file descriptor -- otp ddr address */
volatile unsigned char *ifd;
volatile unsigned char *ofd;
volatile unsigned int  infilesize;
volatile unsigned char  *endfileaddr;
volatile unsigned int insize;           /* valid bytes in inbuf */
volatile unsigned int inptr;            /* index of next byte to be processed in inbuf */
volatile unsigned int outcnt;           /* bytes in output buffer */


volatile long header_bytes;   /* number of bytes in gzip header */

#define EXTHDR 16               /* size of extended local header, inc sig */

#define get_char(c) ascii2pascii(get_byte())
#define outl(addr, value)       (*((volatile unsigned int *)(addr)) = value)

volatile unsigned char inpfile[45]=
{
0x1f,0x8b,0x8,0x8,0x60,0x9b,0xf,0x56,0x0,0x3,0x31,0x36,0x58,0x33,0x32,0x5f,
0x41,0x2e,0x74,0x78,0x74,0x0,0x73,0x74,0x44,0x5,0xbc,0x5c,0x8e,0xa3,0x22,0xa3,
0x22,0x44,0x8a,0x0,0x0,0xd2,0x58,0x57,0x12,0x3e,0x2,0x0,0x0,
};

volatile unsigned char otpfile[574];


volatile unsigned int gl_err_flag = 0; 


int locmemcmp (char *istr,char *ostr, unsigned int n)
{
  unsigned int i;

    for (i=0;i<n;i++) {
      if (istr[i] != ostr[i])
        return 1;
    }
    return 0;
}


int ascii2pascii(c)
  int c;
{
  return (c ? (c | 0x80) : '\0');
}

int check_zipfile(in)
    int in;   /* input file descriptors */
{
    uch *h = inbuf + inptr; /* first local header */

//    ifd = in;

//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);
    /* Check validity of local header, and skip name and extra fields */
    inptr += LOCHDR + SH(h + LOCFIL) + SH(h + LOCEXT);

//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

  //   if (inptr > insize || LG(h) != LOCSIG) {
  // fprintf(stderr, "\n%s: %s: not a valid zip file\n",
  //   progname, ifname);
  // exit_code = ERROR;
  // return ERROR;
  //   }
  //   method = h[LOCHOW];
  //   if (method != STORED && method != DEFLATED) {
  // fprintf(stderr,
  //   "\n%s: %s: first entry not deflated or stored -- use unzip\n",
  //   progname, ifname);
  // exit_code = ERROR;
  // return ERROR;
  //   }

  //   /* If entry encrypted, decrypt and validate encryption header */
  //   if ((decrypt = h[LOCFLG] & CRPFLG) != 0) {
  // fprintf(stderr, "\n%s: %s: encrypted file -- use unzip\n",
  //   progname, ifname);
  // exit_code = ERROR;
  // return ERROR;
  //   }

    /* Save flags for unzip() */
    // ext_header = (h[LOCFLG] & EXTFLG) != 0;
    // pkzip = 1;

    /* Get ofname and time stamp from local header (to be done) */
    return OK;
}




local int get_method(in)
    int in;        /* input file descriptor */
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */

//  fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

    /* If --force and --stdout, zcat == cat, so do not complain about
     * premature end of file: use try_byte instead of get_byte.
     */
  //   if (force && to_stdout) {
  //     fprintf (stderr,"%s:%d\n",__func__, __LINE__);
  // magic[0] = (char)try_byte();
  // magic[1] = (char)try_byte();
  /* If try_byte returned EOF, magic[1] == 0xff */
    // } else {
  //    fprintf (stderr,"%s:%d: inptr=%x,insize=%x\n",__func__, __LINE__,inptr,insize);
  magic[0] = (char)get_byte();
  magic[1] = (char)get_byte();
    // }

//fprintf (stderr,"%s:%d: inptr=%x,insize=%x\n",__func__, __LINE__,inptr,insize);

    method = -1;                 /* unknown yet */
    part_nb++;                   /* number of parts in gzip file */
    header_bytes = 0;
    // last_member = RECORD_IO;
    /* assume multiple members in gzip file except for record oriented I/O */

    if (locmemcmp(magic, GZIP_MAGIC, 2) == 0
        || locmemcmp(magic, OLD_GZIP_MAGIC, 2) == 0) {

          //    fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);
              method = (int)get_byte();
  // if (method != DEFLATED) {
  //     fprintf(stderr,
  //       "%s: %s: unknown method %d -- get newer version of gzip\n",
  //       progname, ifname, method);
  //     exit_code = ERROR;
  //     return -1;
  // }

  // fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

  //work = unzip;
  flags  = (uch)get_byte();

 // fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

  // if ((flags & ENCRYPTED) != 0) {
  //     fprintf(stderr,
  //       "%s: %s is encrypted -- get newer version of gzip\n",
  //       progname, ifname);
  //     exit_code = ERROR;
  //     return -1;
  // }
  // if ((flags & CONTINUATION) != 0) {
  //     fprintf(stderr,
  //    "%s: %s is a a multi-part gzip file -- get newer version of gzip\n",
  //       progname, ifname);
  //     exit_code = ERROR;
  //     if (force <= 1) return -1;
  // }
  // if ((flags & RESERVED) != 0) {
  //     fprintf(stderr,
  //       "%s: %s has flags 0x%x -- get newer version of gzip\n",
  //       progname, ifname, flags);
  //     exit_code = ERROR;
  //     if (force <= 1) return -1;
  // }
  stamp  = (ulg)get_byte();
  stamp |= ((ulg)get_byte()) << 8;
  stamp |= ((ulg)get_byte()) << 16;
  stamp |= ((ulg)get_byte()) << 24;


  // if (stamp != 0 && !no_time) time_stamp = stamp;

  (void)get_byte();  /* Ignore extra flags for the moment */
  (void)get_byte();  /* Ignore OS type for the moment */

 // fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

       if ((flags & CONTINUATION) != 0) {
             unsigned part = (unsigned)get_byte();
              part |= ((unsigned)get_byte())<<8;
    //   if (verbose) {
    // fprintf(stderr,"%s: %s: part number %u\n",
    //   progname, ifname, part);
    //   }
        }
//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

      if ((flags & EXTRA_FIELD) != 0) {
           unsigned len = (unsigned)get_byte();
           len |= ((unsigned)get_byte())<<8;
    //   if (verbose) {
    // fprintf(stderr,"%s: %s: extra field of %u bytes ignored\n",
    //   progname, ifname, len);
    //   }
      while (len--) (void)get_byte();
      }

//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);
      /* Get original file name if it was truncated */
        if ((flags & ORIG_NAME) != 0) {
          if (no_name || (to_stdout && !list) || part_nb > 1) {
               /* Discard the old name */
                char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
                do {c=get_byte();} while (c != 0);
           } else {
           /* Copy the base name. Keep a directory prefix intact. */
                // char *p = basename(ofname);
                // char *base = p;
                  char *p = ofname;

            for (;;) {
              *p = (char)get_char();
                if (*p++ == '\0') break;
                // if (p >= ofname+sizeof(ofname)) {
                // error("corrupted input -- file name too large");
                // }
               }

                /* If necessary, adapt the name to local OS conventions: */
                // if (!list) {
                //    MAKE_LEGAL_NAME(base);
                //   if (base) list=0; /* avoid warning about unused variable */
                // }
            } /* no_name || to_stdout */
        } /* ORIG_NAME */

//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);
            /* Discard file comment if any */
           if ((flags & COMMENT) != 0) {
                while (get_char() != 0) /* null */ ;
            }

            if (part_nb == 1) {
                 //header_bytes = inptr + 2*sizeof(long); /* include crc and size */
                  header_bytes = inptr + 2*8;
            }

          //  fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);
  } 
  else if (locmemcmp(magic, PKZIP_MAGIC, 2) == 0 && inptr == 2
  && locmemcmp((char*)inbuf, PKZIP_MAGIC, 4) == 0) {
        /* To simplify the code, we support a zip file when alone only.
         * We are thus guaranteed that the entire local header fits in inbuf.
         */
          inptr = 0;
        //  work = unzip;
          check_zipfile(in);
         // if (check_zipfile(in) != OK) return -1;
          /* check_zipfile may get ofname from the local header */
          // last_member = 1;

        //  fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

  } 
  // else if (memcmp(magic, PACK_MAGIC, 2) == 0) {
  //         work = unpack;
  //         method = PACKED;

  // } else if (memcmp(magic, LZW_MAGIC, 2) == 0) {
  //         work = unlzw;
  //         method = COMPRESSED;
  //         last_member = 1;
  // } else if (memcmp(magic, LZH_MAGIC, 2) == 0) {
  //         work = unlzh;
  //         method = LZHED;
  //         last_member = 1;
  // } else if (force && to_stdout && !list) { /* pass input unchanged */
  //         method = STORED;
  //         work = copy;
  //         inptr = 0;
  //         last_member = 1;
  // }

//fprintf (stderr,"%s:%d: inptr=%x\n",__func__, __LINE__,inptr);

  //   if (method >= 0) return method;


  //   if (part_nb == 1) {
  // fprintf(stderr, "\n%s: %s: not in gzip format\n", progname, ifname);
  // exit_code = ERROR;
  // return -1;
  //   } else {
  // WARN((stderr, "\n%s: %s: decompression OK, trailing garbage ignored\n",
  //       progname, ifname));
  // return -2;
  //   }
}

static void print_hex_digit(unsigned char v)
{
    if (v < 10) {
        BSLDEUG('0'+v);
    } else {
        BSLDEUG('A'+(v-10));
    }
}

static void print_hex32(ulg v)
{
    unsigned char *b = (unsigned char *)&v;
    unsigned i;
    BSLDEUG('[');
    for (i=0; i<4; i++) {
        print_hex_digit((b[3-i]>>4)&0x0F);
        print_hex_digit(b[3-i]&0x0F);
    }
    BSLDEUG(']');
}

int gzip (volatile unsigned int inpf_addr, volatile unsigned int otpf_addr, volatile unsigned int otpf_sz)
{
    ulg orig_crc = 0;       /* original crc */
    ulg orig_len = 0;       /* original uncompressed length */
    int n;
    uch buf[EXTHDR];        /* extended local header */
    unsigned int i;
    volatile unsigned int img_sz = 0;
    volatile unsigned int img_tag = 0;
    int err_flag = 0;

    gl_err_flag = 0;

#if 0
    FILE *fp;
    FILE *ofp;
    unsigned int  fileLen;
    unsigned char*  buffer;
    unsigned char*  otbuffer;
    int ofp_size = 0;

   // ofp=fopen("dres.bin","wb");
    //fp = fopen("rtos.bin.gz", "rb");
     
    fp = fopen(argv[1], "rb");
     ofp=fopen(argv[2],"wb");
    ofp_size = atoi(argv[3]);

    //Get file length
    fseek(fp, 0, SEEK_END);
    fileLen=ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer=(char *)malloc(fileLen+1);
    fread(buffer, fileLen, 1, fp);
    fclose(fp);

    otbuffer=(char *)malloc(ofp_size+1);

    fprintf(stderr, "fileLen=%x\n",fileLen);

    ifd = buffer;
    infilesize = fileLen;
    ofd = otbuffer;
#else
    // TEST PAT1
//     ifd = &inpfile[0];       // inp addr   
//     ofd = &otpfile[0];       // otp addr
 //   ifd = (volatile unsigned char *)inpfile;  
   //  ofd = (volatile unsigned char *)otpfile;
    // infilesize = 45;         // inp image size/

    // TEST PAT2
  //  ifd = (volatile unsigned char *)Test_iFile_Address;  
    // ofd = (volatile unsigned char *)Test_oFile_Address;
    // infilesize = Test_iFile_Size;         // inp image size/

    // DGZIP INF
  ifd = (volatile unsigned char *)inpf_addr;  
  ofd = (volatile unsigned char *)otpf_addr;
  infilesize = (volatile unsigned int) otpf_sz;         // otpf image size
#endif

  endfileaddr = ifd + infilesize;

  locmem_indx = 0;
  err_flag = 0;

    // print_msg("gzip\n");

     BSLDEUG('G')
     BSLDEUG('=')
    img_sz = *((volatile unsigned int*) (inpf_addr - 4));

    if (img_sz > 0x400000) {
      BSLDEUG('x')
      return (1);
    }


    img_tag = *((volatile unsigned int*) (inpf_addr + img_sz - 8)); 
  

    // if (img_sz != (real_sz -4)) {
    //   BSLDEUG('x')
    //   return (1);
    //  }

     if (img_tag != 0x12345678) {
      BSLDEUG('X')
      return (1);
     }

  //  print_msg("ifd=%x,ofd=%x,%x,%x\n",ifd,ofd,endfileaddr,infilesize);

    // for (i=0;i<32;i++)
    //   fprintf(stderr, "%x\n",ifd[i]);

/*     test_pin = (char *)0x01d00000;
    test_pin = (volatile unsigned char *)inbuf;*/
     // test_pin = endfileaddr;

//for (i=0;i<0x13;i++)
  //  (void)get_byte(); 

  get_method(1);

     updcrc(NULL, 0);           /* initialize crc */

     BSLDEUG('i')
    inflate();
     BSLDEUG('I')

     for (n = 0; n < 8; n++) {
          buf[n] = (uch)get_byte(); /* may cause an error if EOF */
     }
     orig_crc = LG(buf);
     orig_len = LG(buf+4);


    if (orig_crc != updcrc(outbuf, 0)) {
     // error("invalid compressed data--crc error");
   //   print_msg("invalid compressed data--crc error\n");
       BSLDEUG('C')
       err_flag = 1;
    }
    if (orig_len != (ulg)bytes_out) {
     // error("invalid compressed data--length error");
    //  print_msg("invalid compressed data--length error\n");
       BSLDEUG('L')
       err_flag = 1;
    }


  outl(0x98700004, 0x0);
  outl(0x9870000C, 0x0);
  outl(0xFFFF4004, 0x0);



  //  print_msg("crc=%x , len=%x\n",orig_crc,orig_len);
    print_hex32(orig_crc);
    print_hex32(orig_len);
    

//for test
#if 0   
fwrite(otbuffer,sizeof(char),orig_len,ofp);
fclose(ofp);

free(buffer);
free(otbuffer);  
#endif

    if (err_flag) {
      return (1);
    }
    else {
      BSLDEUG('S')
      return (0);
    }
}

void *locmalloc(unsigned int size)
{
    unsigned char *retptr;

    retptr = (unsigned char *) (configHEAP_CB_SIZE + locmem_indx);

    //retptr = &locmem[locmem_indx];
    locmem_indx += size;

   // print_msg("retptr=%x\n",retptr);

    return (retptr);
}   

// void locfree()
// {
// }

int huft_build(b, n, s, d, e, t, m)
unsigned *b;            /* code lengths in bits (all assumed <= BMAX) */
unsigned n;             /* number of codes (assumed <= N_MAX) */
unsigned s;             /* number of simple-valued codes (0..s-1) */
ush *d;                 /* list of base values for non-simple codes */
ush *e;                 /* list of extra bits for non-simple codes */
struct huft **t;        /* result: starting table */
int *m;                 /* maximum lookup bits, returns actual */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
volatile  unsigned a;                   /* counter for codes of length k */
volatile  unsigned c[BMAX+1];           /* bit length count table */
volatile  unsigned f;                   /* i repeats in table every f entries */
volatile  int g;                        /* maximum code length */
volatile  int h;                        /* table level */
volatile  register unsigned i;          /* counter, current code */
volatile  register unsigned j;          /* counter */
volatile  register int k;               /* number of bits in current code */
volatile  int l;                        /* bits per table (returned in m) */
volatile  register unsigned *p;         /* pointer into c[], b[], or v[] */
volatile  register struct huft *q;      /* points to current table */
volatile  struct huft r;                /* table entry for structure assignment */
volatile  struct huft *u[BMAX];         /* table stack */
volatile  unsigned v[N_MAX];            /* values in order of bit length */
volatile  register int w;               /* bits before this table == (l * h) */
volatile  unsigned x[BMAX+1];           /* bit offsets, then code stack */
volatile  unsigned *xp;                 /* pointer into x */
volatile  int y;                        /* number of dummy codes added */
volatile  unsigned z;                   /* number of entries in current table */
volatile  unsigned int mzo;

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Generate counts for each bit length */
#if 0
 memzero(c, sizeof(c));
#else




//  for (mzo=0; mzo<(sizeof(c)); mzo++){
 for (mzo=0; mzo<(0x11); mzo++){
    c[mzo] = 0;
  }
#endif


//print_msg ("%s:%d \n", __func__, __LINE__);

  p = b;  i = n;
  do {
    //Tracecv(*p, (stderr, (n-i >= ' ' && n-i <= '~' ? "%c %d\n" : "0x%x %d\n"), 
	    //n-i, *p));

    c[*p]++;                    /* assume all entries <= BMAX */
    p++;                      /* Can't combine with above line (Solaris bug) */
  } while (--i);

//print_msg ("%s:%d \n", __func__, __LINE__);

  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (struct huft *)NULL;
    *m = 0;
    return 0;
  }

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((unsigned)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((unsigned)l > i)
    l = i;
  *m = l;

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;                 /* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = -l;                       /* bits decoded == (l * h) */
  u[0] = (struct huft *)NULL;   /* just to keep compilers happy */
  q = (struct huft *)NULL;      /* ditto */
  z = 0;                        /* ditto */

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l)
      {
        h++;
        w += l;                 /* previous table always l bits */

        /* compute minimum size table less than or equal to l bits */
        z = (z = g - w) > (unsigned)l ? l : z;  /* upper limit on table size */
        if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          while (++j < z)       /* try smaller tables up to z bits */
          {
            if ((f <<= 1) <= *++xp)
              break;            /* enough codes to use up j bits */
            f -= *xp;           /* else deduct codes from patterns */
          }
        }
        z = 1 << j;             /* table entries for j-bit table */

       // fprintf(stderr, "%d , %d\n",z, (sizeof(struct huft))); 

        /* allocate and link in new table */
#if 0        
        if ((q = (struct huft *)malloc((z + 1)*sizeof(struct huft))) ==
            (struct huft *)NULL)
        {
          if (h)
            huft_free(u[0]);
          return 3;             /* not enough memory */
        }
#else
        //if ((q = (struct huft *)locmalloc((z + 1)*sizeof(struct huft))) ==
          //  (struct huft *)NULL)
    #if 0
       if ((q = (struct huft *)locmalloc((z + 1)*0x10) ==
            (struct huft *)NULL))
        {
          if (h)
            huft_free(u[0]);
          return 3;             /* not enough memory */
        }
    #else
      q = (struct huft *)locmalloc((z + 1)*0x10);
    #endif  
#endif
    //    print_msg("q=%x\n",q);


        hufts += z + 1;         /* track memory usage */
        *t = q + 1;             /* link to list for huft_free() */
        *(t = &(q->v.t)) = (struct huft *)NULL;
        u[h] = ++q;             /* table starts after link */

        /* connect to last table, if there is one */
        if (h)
        {
          x[h] = i;             /* save pattern for backing up */
          r.b = (uch)l;         /* bits to dump before this table */
          r.e = (uch)(16 + j);  /* bits in this table */
          r.v.t = q;            /* pointer to this table */
          j = i >> (w - l);     /* (get around Turbo C bug) */
          u[h-1][j] = r;        /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);
      if (p >= v + n)
        r.e = 99;               /* out of values--invalid code */
      else if (*p < s)
      {
        r.e = (uch)(*p < 256 ? 16 : 15);    /* 256 is end-of-block code */
        r.v.n = (ush)(*p);             /* simple code is just the value */
	p++;                           /* one compiler does not like *p++ */
      }
      else
      {
        r.e = (uch)e[*p - s];   /* non-simple--look up in lists */
        r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
      {
        h--;                    /* don't need to update q */
        w -= l;
      }
    }
  }

//print_msg ("%s:%d \n", __func__, __LINE__);

  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}



int huft_free(t)
struct huft *t;         /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  #if 0
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *)NULL)
  {
    q = (--p)->v.t;
    free((char*)p);
    p = q;
  } 
  #endif
  return 0;
}


int inflate_codes(tl, td, bl, bd)
struct huft *tl, *td;   /* literal/length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */
{
volatile  register unsigned e;  /* table entry flag/number of extra bits */
volatile  unsigned n, d;        /* length and index for copy */
volatile  unsigned w;           /* current window position */
volatile  struct huft *t;       /* pointer to table entry */
volatile  unsigned ml, md;      /* masks for bl and bd bits */
volatile  register ulg b;       /* bit buffer */
volatile  register unsigned k;  /* number of bits in bit buffer */

  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */

  /* inflate the coded data */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];

//BSLDEUG('+')

   // print_msg("bkw = %x,%x,%x,%x,%x,%x,%x\n",b,k,w,ml,md,bl,bd);
  for (;;)                      /* do until end of block */
  {

   // print_msg("bkwt = %x,%x,%x,%x,%x,%x\n",b,k,w,ml,md,t);
  NEEDBITS(( unsigned)bl)

  if (gl_err_flag) {
    return 1;
  }



 #if 0 
    if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
      do {
        if (e == 99)
          return 1;
        DUMPBITS(t->b)
        print_msg("bkwt- = %x,%x,%x,%x,%x,%x\n",b,k,w,ml,md,t);
        e -= 16;
        NEEDBITS(e)
      } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
#else
   //   print_msg("--%x,%x,%x\n",tl,b,ml);
      t = tl + ((unsigned)b & ml);
     // print_msg("t = %x\n",t);
      e = t->e;
     // print_msg("e = %x\n",e);
      if (e > 16) {
        do {
        if (e == 99)
          return 1;
        DUMPBITS(t->b)
       // print_msg("bkwt- = %x,%x,%x,%x,%x,%x\n",b,k,w,ml,md,t);
        e -= 16;
        NEEDBITS(e)

        if (gl_err_flag) {
          return 1;
        }

        t = t->v.t + ((unsigned)b & mask_bits[e]);
        e = t->e;
        }while(e > 16);
      }
#endif

    DUMPBITS(t->b)
   // print_msg("bkwt-- = %x,%x,%x,%x,%x,%x\n",b,k,w,ml,md,t);

    if (e == 16)                /* then it's a literal */
    {
      slide[w++] = (uch)t->v.n;
     // Tracevv((stderr, "%c", slide[w-1]));
      if (w == WSIZE)
      {
        flush_output(w);
        w = 0;
      }
    }
    else                        /* it's an EOB or a length */
    {
      /* exit if end of block */
      if (e == 15)
        break;

      /* get length of block to copy */
      NEEDBITS(e)
      n = t->v.n + ((unsigned)b & mask_bits[e]);
      DUMPBITS(e);

      /* decode distance of block to copy */
      NEEDBITS(( unsigned)bd)
#if 0      
      if ((e = (t = td + ((unsigned)b & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
#else
        ////////////////
        t = td + ((unsigned)b & md);
        e = t->e;
        if (e > 16) {
          do {
              if (e == 99)
              return 1;

              DUMPBITS(t->b)
              e -= 16;
              NEEDBITS(e)

              if (gl_err_flag) {
                return 1;
              }

              t = t->v.t + ((unsigned)b & mask_bits[e]);
              e = t->e;
          }while(e > 16);
        }
#endif
        ///////////////

      DUMPBITS(t->b)
      NEEDBITS(e)
      d = w - t->v.n - ((unsigned)b & mask_bits[e]);
      DUMPBITS(e)
    //  Tracevv((stderr,"\\[%d,%d]", w-d, n));

      /* do the copy */
      do {

        if (gl_err_flag) {
          return 1;
        }

        #if 0
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);

        #else
      //////////////////// 
        if ((d &= WSIZE-1) > w ) {
            e = WSIZE - d;
        } 
        else {
            e = WSIZE - w;
        }

        if (e > n) {
          e = n;
        }
        
        n -= e;
      #endif


#if !defined(NOMEMCPY) && !defined(DEBUG)
        // if (w - d >= e)         /* (this test assumes unsigned comparison) */
        // {
        //   memcpy(slide + w, slide + d, e);
        //   w += e;
        //   d += e;

        //    fprintf(stderr, "delezue 1-1-0-2\n");
        // }
        // else                      /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
          do {
            slide[w++] = slide[d++];
          } while (--e);

        if (w == WSIZE)
        {
          flush_output(w);
          w = 0;
        }
      } while (n);
    }
  }

//  BSLDEUG('-')


  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;

  /* done */
  return 0;
}


#if 1
int inflate_stored()
/* "decompress" an inflated type 0 (stored) block. */
{
#if 1  
volatile  unsigned n;           /* number of bytes in block */
volatile  unsigned w;           /* current window position */
volatile  register ulg b;       /* bit buffer */
volatile  register unsigned k;  /* number of bits in bit buffer */


  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */

  /* go to byte boundary */
  n = k & 7;
  DUMPBITS(n);


  /* get the length and its complement */
  NEEDBITS(16)
  n = ((unsigned)b & 0xffff);
  DUMPBITS(16)
  NEEDBITS(16)
  if (n != (unsigned)((~b) & 0xffff))
    return 1;                   /* error in compressed data */
  DUMPBITS(16)


  /* read and output the compressed data */
  while (n--)
  {
    NEEDBITS(8)
    slide[w++] = (uch)b;
    if (w == WSIZE)
    {
      flush_output(w);
      w = 0;
    }
    DUMPBITS(8)
  }


  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;
  return 0;
#endif  
}



int inflate_fixed()
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
#if 1  
volatile  int i;                /* temporary variable */
volatile  struct huft *tl;      /* literal/length code table */
volatile  struct huft *td;      /* distance code table */
volatile  int bl;               /* lookup bits for tl */
volatile  int bd;               /* lookup bits for td */
volatile  unsigned l[288];      /* length list for huft_build */

//  print_msg ("%s:%d \n", __func__, __LINE__);
  /* set up literal table */
  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)          /* make a complete, but wrong code set */
    l[i] = 8;
  bl = 7;

//BSLDEUG('A') 
 // print_msg ("%s:%d \n", __func__, __LINE__);

  if ((i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl)) != 0)
    return i;

//print_msg ("tltd = %x,%x,%x,%x\n",tl,td,bl,bd);
//BSLDEUG('B') 

  /* set up distance table */
  for (i = 0; i < 30; i++)      /* make an incomplete code set */
    l[i] = 5;
  bd = 5;
  if ((i = huft_build(l, 30, 0, cpdist, cpdext, &td, &bd)) > 1)
  {
    huft_free(tl);
    return i;
  }

//print_msg ("tltd = %x,%x,%x,%x\n",tl,td,bl,bd);
//BSLDEUG('C') 

  /* decompress until an end-of-block code */
  if (inflate_codes(tl, td, bl, bd))
    return 1;
//print_msg ("%s:%d \n", __func__, __LINE__);
//BSLDEUG('D') 

  /* free the decoding tables, return */
  huft_free(tl);
  huft_free(td);
  return 0;
#endif  
}

#endif

int inflate_dynamic()
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
volatile  int i;                /* temporary variables */
volatile  unsigned j;
volatile  unsigned l;           /* last length */
volatile  unsigned m;           /* mask for bit lengths table */
volatile  unsigned n;           /* number of lengths to get */
  struct huft *tl;      /* literal/length code table */
  struct huft *td;      /* distance code table */
volatile  int bl;               /* lookup bits for tl */
volatile  int bd;               /* lookup bits for td */
volatile  unsigned nb;          /* number of bit length codes */
volatile  unsigned nl;          /* number of literal/length codes */
volatile  unsigned nd;          /* number of distance codes */
#ifdef PKZIP_BUG_WORKAROUND
volatile  unsigned ll[288+32];  /* literal/length and distance code lengths */
#else
volatile  unsigned ll[286+30];  /* literal/length and distance code lengths */
#endif
volatile  register ulg b;       /* bit buffer */
volatile  register unsigned k;  /* number of bits in bit buffer */


  /* make local bit buffer */
  b = bb;
  k = bk;

//fprintf (stderr,"%s:%d: inptr=%x,insize=%x\n",__func__, __LINE__,inptr,insize);

  /* read in table lengths */
  NEEDBITS(5)
  nl = 257 + ((unsigned)b & 0x1f);      /* number of literal/length codes */
  DUMPBITS(5)
  NEEDBITS(5)
  nd = 1 + ((unsigned)b & 0x1f);        /* number of distance codes */
  DUMPBITS(5)
  NEEDBITS(4)
  nb = 4 + ((unsigned)b & 0xf);         /* number of bit length codes */
  DUMPBITS(4)
#ifdef PKZIP_BUG_WORKAROUND
  if (nl > 288 || nd > 32)
#else
  if (nl > 286 || nd > 30)
#endif
    return 1;                   /* bad lengths */


  /* read in bit-length-code lengths */
  for (j = 0; j < nb; j++)
  {
    NEEDBITS(3)
    ll[border[j]] = (unsigned)b & 7;
    DUMPBITS(3)
  }
  for (; j < 19; j++)
    ll[border[j]] = 0;


  /* build decoding table for trees--single level, 7 bit lookup */
  bl = 7;
  if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0)
  {
    if (i == 1)
      huft_free(tl);
    return i;                   /* incomplete code set */
  }


  /* read in literal and distance code lengths */
  n = nl + nd;
  m = mask_bits[bl];
  i = l = 0;
  while ((unsigned)i < n)
  {
    NEEDBITS((unsigned)bl)
    j = (td = tl + ((unsigned)b & m))->b;
    DUMPBITS(j)
    j = td->v.n;
    if (j < 16)                 /* length of code in bits (0..15) */
      ll[i++] = l = j;          /* save last length in l */
    else if (j == 16)           /* repeat last length 3 to 6 times */
    {
      NEEDBITS(2)
      j = 3 + ((unsigned)b & 3);
      DUMPBITS(2)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = l;
    }
    else if (j == 17)           /* 3 to 10 zero length codes */
    {
      NEEDBITS(3)
      j = 3 + ((unsigned)b & 7);
      DUMPBITS(3)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    }
    else                        /* j == 18: 11 to 138 zero length codes */
    {
      NEEDBITS(7)
      j = 11 + ((unsigned)b & 0x7f);
      DUMPBITS(7)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    }
  }


  /* free decoding table for trees */
  huft_free(tl);


  /* restore the global bit buffer */
  bb = b;
  bk = k;


  /* build the decoding tables for literal/length and distance codes */
  bl = lbits;
  if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0)
  {
    if (i == 1) {
      // fprintf(stderr, " incomplete literal tree\n");
      huft_free(tl);
    }
    return i;                   /* incomplete code set */
  }
  bd = dbits;
  if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0)
  {
    if (i == 1) {
      // fprintf(stderr, " incomplete distance tree\n");
#ifdef PKZIP_BUG_WORKAROUND
      i = 0;
    }
#else
      huft_free(td);
    }
    huft_free(tl);
    return i;                   /* incomplete code set */
#endif
  }


  /* decompress until an end-of-block code */
  if (inflate_codes(tl, td, bl, bd))
    return 1;


  /* free the decoding tables, return */
  huft_free(tl);
  huft_free(td);
  return 0;
}



int inflate_block(e)
volatile int *e;                 /* last block flag */
/* decompress an inflated block */
{
volatile  unsigned t;           /* block type */
volatile  register ulg b;       /* bit buffer */
volatile  register unsigned k;  /* number of bits in bit buffer */


  /* make local bit buffer */
  b = bb;
  k = bk;


 // print_msg("inptr = %x,%x\n",inptr,insize);
//print_msg("b = %x,%x\n",b,k);  

  /* read in last block bit */
  NEEDBITS(1)

//fprintf(stderr, "b = %x,%x\n",b,k);

  *e = (int)b & 1;
  DUMPBITS(1)


  /* read in block type */
  NEEDBITS(2)
  t = (unsigned)b & 3;
  DUMPBITS(2)


  /* restore the global bit buffer */
  bb = b;
  bk = k;

 //BSLDEUG('T')

//  print_msg("t= %x\n",t);

  /* inflate that block type */
  if (t == 2)
    return inflate_dynamic();
  if (t == 0)
    return inflate_stored();
  if (t == 1)
    return inflate_fixed();


  /* bad block type */
  return 2;
}



int inflate()
/* decompress an inflated entry */
{
volatile  int e;                /* last block flag */
volatile  int r;                /* result code */
volatile  unsigned h;           /* maximum struct huft's malloc'ed */


  /* initialize window, bit buffer */
  wp = 0;
  bk = 0;
  bb = 0;


  /* decompress until the last block */
  h = 0;
  do {
    hufts = 0;
    if ((r = inflate_block(&e)) != 0)
      return r;
    if (hufts > h)
      h = hufts;
  } while (!e);

  /* Undo too much lookahead. The next read will be byte aligned so we
   * can discard unused bits in the last meaningful byte.
   */
  while (bk >= 8) {
    bk -= 8;
    inptr--;
  }

  /* flush out slide */
  flush_output(wp);


  /* return success */
// #ifdef DEBUG
//   fprintf(stderr, "<%u> ", h);
// #endif /* DEBUG */
  return 0;
}

