#ifndef SURFACE_H
#define SURFACE_H
typedef struct {
  char* hostname;
  char* port;
  int   send;
}client_args;

typedef struct {
  unsigned short width;
  unsigned short height;
  unsigned int   bpp;
}surface_type;

#endif
