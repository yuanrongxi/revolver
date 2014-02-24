#ifndef __GETTIMEOFDAY_H
#define __GETTIMEOFDAY_H
#ifdef WIN32
//ÉêÃ÷ÒıÓÃ
int gettimeofday (struct timeval *tv, void *tz);
#endif
#endif
