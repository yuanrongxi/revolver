#ifndef __GETTIMEOFDAY_H
#define __GETTIMEOFDAY_H
#ifdef WIN32
//申明引用
int gettimeofday (struct timeval *tv, void *tz);
#endif
#endif
