/*
* function:    md5   algorithm
*              this codes can generate md5 of file
*attention:the file must be smaller than 2GB because ftell
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include<ctype.h>

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
//x shifts leftly y bits round
#define RL(x, y) (((x) << (y)) | ((x) >> (32 - (y))))  
//PP(aabbccdd)=ddccbbaa
#define PP(x) (x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)|(x>>24)  

#define FF(a, b, c, d, x, s, ac) a = b + (RL((a + F(b,c,d) + x + ac),s))
#define GG(a, b, c, d, x, s, ac) a = b + (RL((a + G(b,c,d) + x + ac),s))
#define HH(a, b, c, d, x, s, ac) a = b + (RL((a + H(b,c,d) + x + ac),s))
#define II(a, b, c, d, x, s, ac) a = b + (RL((a + I(b,c,d) + x + ac),s))
//len:length of file,flen[2]:initial length of file by way of 64 bits 
unsigned A,B,C,D,a,b,c,d,i,len,flen[2],x[16];  
//core algorithm,64 times
void md5(){                 
  a=A,b=B,c=C,d=D;
  FF (a, b, c, d, x[ 0],  7, 0xd76aa478);
  FF (d, a, b, c, x[ 1], 12, 0xe8c7b756);
  FF (c, d, a, b, x[ 2], 17, 0x242070db);
  FF (b, c, d, a, x[ 3], 22, 0xc1bdceee);
  FF (a, b, c, d, x[ 4],  7, 0xf57c0faf);
  FF (d, a, b, c, x[ 5], 12, 0x4787c62a);
  FF (c, d, a, b, x[ 6], 17, 0xa8304613);
  FF (b, c, d, a, x[ 7], 22, 0xfd469501);
  FF (a, b, c, d, x[ 8],  7, 0x698098d8);
  FF (d, a, b, c, x[ 9], 12, 0x8b44f7af);
  FF (c, d, a, b, x[10], 17, 0xffff5bb1);
  FF (b, c, d, a, x[11], 22, 0x895cd7be);
  FF (a, b, c, d, x[12],  7, 0x6b901122);
  FF (d, a, b, c, x[13], 12, 0xfd987193);
  FF (c, d, a, b, x[14], 17, 0xa679438e);
  FF (b, c, d, a, x[15], 22, 0x49b40821);
  GG (a, b, c, d, x[ 1],  5, 0xf61e2562);
  GG (d, a, b, c, x[ 6],  9, 0xc040b340);
  GG (c, d, a, b, x[11], 14, 0x265e5a51);
  GG (b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
  GG (a, b, c, d, x[ 5],  5, 0xd62f105d);
  GG (d, a, b, c, x[10],  9, 0x02441453);
  GG (c, d, a, b, x[15], 14, 0xd8a1e681); 
  GG (b, c, d, a, x[ 4], 20, 0xe7d3fbc8); 
  GG (a, b, c, d, x[ 9],  5, 0x21e1cde6);
  GG (d, a, b, c, x[14],  9, 0xc33707d6);
  GG (c, d, a, b, x[ 3], 14, 0xf4d50d87);
  GG (b, c, d, a, x[ 8], 20, 0x455a14ed); 
  GG (a, b, c, d, x[13],  5, 0xa9e3e905); 
  GG (d, a, b, c, x[ 2],  9, 0xfcefa3f8); 
  GG (c, d, a, b, x[ 7], 14, 0x676f02d9); 
  GG (b, c, d, a, x[12], 20, 0x8d2a4c8a);
  HH (a, b, c, d, x[ 5],  4, 0xfffa3942);
  HH (d, a, b, c, x[ 8], 11, 0x8771f681);
  HH (c, d, a, b, x[11], 16, 0x6d9d6122);
  HH (b, c, d, a, x[14], 23, 0xfde5380c);
  HH (a, b, c, d, x[ 1],  4, 0xa4beea44);
  HH (d, a, b, c, x[ 4], 11, 0x4bdecfa9);
  HH (c, d, a, b, x[ 7], 16, 0xf6bb4b60);
  HH (b, c, d, a, x[10], 23, 0xbebfbc70);
  HH (a, b, c, d, x[13],  4, 0x289b7ec6);
  HH (d, a, b, c, x[ 0], 11, 0xeaa127fa);
  HH (c, d, a, b, x[ 3], 16, 0xd4ef3085);
  HH (b, c, d, a, x[ 6], 23, 0x04881d05); 
  HH (a, b, c, d, x[ 9],  4, 0xd9d4d039);
  HH (d, a, b, c, x[12], 11, 0xe6db99e5); 
  HH (c, d, a, b, x[15], 16, 0x1fa27cf8); 
  HH (b, c, d, a, x[ 2], 23, 0xc4ac5665); 
  II (a, b, c, d, x[ 0],  6, 0xf4292244);
  II (d, a, b, c, x[ 7], 10, 0x432aff97);
  II (c, d, a, b, x[14], 15, 0xab9423a7); 
  II (b, c, d, a, x[ 5], 21, 0xfc93a039);
  II (a, b, c, d, x[12],  6, 0x655b59c3);
  II (d, a, b, c, x[ 3], 10, 0x8f0ccc92);
  II (c, d, a, b, x[10], 15, 0xffeff47d); 
  II (b, c, d, a, x[ 1], 21, 0x85845dd1);
  II (a, b, c, d, x[ 8],  6, 0x6fa87e4f);
  II (d, a, b, c, x[15], 10, 0xfe2ce6e0);
  II (c, d, a, b, x[ 6], 15, 0xa3014314);
  II (b, c, d, a, x[13], 21, 0x4e0811a1);
  II (a, b, c, d, x[ 4],  6, 0xf7537e82);
  II (d, a, b, c, x[11], 10, 0xbd3af235);
  II (c, d, a, b, x[ 2], 15, 0x2ad7d2bb);
  II (b, c, d, a, x[ 9], 21, 0xeb86d391);
  A += a;
  B += b;
  C += c;
  D += d;
}

char md5_in[512];//md5 container
char *md5_prefix = "MD-5(0x):\n";

//generate md5 and store in array of MD5,then store it in file ".md5"   
int get_md5(char *user_conf, char *filename, int fd)
{
	  FILE *fp;
	  //open file
    if(!(fp=fopen(filename,"rb"))) 
    	{
    		printf("md5:Can not open this file!\n");
    		return 0;
    	}  
    //set the hand of file to the end of file
    fseek(fp, 0, SEEK_END);  
    //ftell returns long,the maximum of file is 2GB,returns -1 if exceeds 2GB
    if((len=ftell(fp))==-1)
    {
      printf("Sorry! Can not calculate files which larger than 2 GB!\n");    
      fclose(fp);
      return 0;
    } 
    //reset the hand of file to the header of file
    rewind(fp);  
    //initialization chaining variable
    A=0x67452301,B=0xefcdab89,C=0x98badcfe,D=0x10325476; 
    //flen by unit of bit
    flen[1]=len/0x20000000;     
    flen[0]=(len%0x20000000)*8;
    //initialization x array
    memset(x,0,64);  
    //read 16 groups of data,4 Bytes is a group
    fread(&x,4,16,fp);  
    //calculate to the tail of file
    for(i=0;i<len/64;i++){    
      md5();
      memset(x,0,64);
      fread(&x,4,16,fp);
    }
    //add bit 1 and 0 at the tail,128:10000000
    ((char*)x)[len%64]=128;  
    if(len%64>55) md5(),memset(x,0,64);
    //add the length of original by unit of bit
    memcpy(x+14,flen,8);    
    md5();
    fclose(fp);
    //put it in MD5 conversely
    sprintf(md5_in,"%08x%08x%08x%08x",PP(A),PP(B),PP(C),PP(D));
    write(fd, "\n", 1);
    write(fd, md5_prefix, strlen(md5_prefix));
    write(fd, md5_in, 32);
    write(fd, "\n", 1);
    
    return 0;
}
