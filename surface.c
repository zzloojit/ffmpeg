#include <SDL.h>
#include <pthread.h>
#include "surface.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int pipe_fd = -1;
static SDL_Surface* screen;
static SDL_Overlay* overlay;

int connection (const char* host, const char* port)
{
  static const int on = 1, off = 0;
  int rc = 0, sock = 0;
  struct addrinfo ai, *res, *e;

  memset (&ai, 0, sizeof (ai));
  ai.ai_flags = AI_PASSIVE |AI_ADDRCONFIG;
  ai.ai_socktype = SOCK_STREAM;
  ai.ai_family = PF_UNSPEC;
  
  rc = getaddrinfo(host, port, &ai, &res);
  
  if (rc != 0) {
    assert (0);
    return -1;
  }
  
  for (e = res; e != NULL; e = e->ai_next) {
    sock = socket (e->ai_family, e->ai_socktype, e->ai_protocol);
    if (sock < 0) {
      continue;
    }
    int r = connect(sock, e->ai_addr, e->ai_addrlen);
    if (r == 0) {
      return sock;
    }
    close(sock);
  }
  return -1;
}

int read_safe(int fd, void* buf, ssize_t size)
{
  int n_rd = 0;
  while (n_rd < size) {
    int len = read (fd, buf + n_rd, size - n_rd);
    if (len == -1) {
      if (errno != EINTR) {
	return -1;
      }
      continue;
    }
    n_rd += len;
  }
  return n_rd;
}
int write_safe (int fd, void* buf, ssize_t size)
{
  int n_wd = 0;
  while (n_wd < size) {
    int len = write (fd, buf + n_wd, size - n_wd);
    if (len == -1) {
      if (errno != EINTR) {
	return -1;
      }
      continue;
    }
    n_wd += len;
  }
  return n_wd;
}

void update_frame (char* buf, SDL_Surface* screen)
{

  SDL_Surface *image = SDL_CreateRGBSurfaceFrom (buf, 1280, 800, 32,
						1280 * 4, 0, 0, 0, 255);
  SDL_Rect rec;
  SDL_Rect srec;
  
  srec.x = 0, srec.y = 0, srec.w = 1280, srec.h = 800;
  rec = srec;
      
  SDL_BlitSurface(image, &srec, screen, &rec);
  SDL_UpdateRect(screen, 0, 0, 1280, 800);
}

int poll_loop (int fd) 
{
  int pkt_size = 0;
  int k = 0, j = 0;
  char* buff = NULL;
  char* frame = NULL;
  int wd = open("/tmp/remote.mp4", O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if (wd == -1) {
    fprintf(stderr, "errno is %s\n", strerror(errno));
    assert (0);
  }
  init_decode();
  while (1) {
    k = read_safe(fd, &pkt_size, 4);
    if (k == -1){
      return -1;
    }
  
    fprintf(stderr, "read head size is %d\n", pkt_size);
    buff = malloc (pkt_size);
    j = read_safe (fd, buff, pkt_size);

    if (j == -1) {
      return -1;
    }
    frame = decode_frame(buff, pkt_size);
    assert (frame);
    update_frame (frame , screen);
    k = write_safe (wd, buff, pkt_size);
    if (k == -1) {
      assert (0);
    }
    fprintf(stderr, "read buf size is %d\n", j);
    free(buff);
  }  
}

void* surface_work(void* args)
{
  client_args* arg = (client_args*)args;
  int csock = connection (arg->hostname, arg->port);
  pipe_fd = arg->send;
  read (pipe_fd, &screen , sizeof (screen));
  
  if (csock == -1) {
    assert (0 && "connect to host failed");
  }
  poll_loop (csock);
  return NULL;
}

void surface_start(void* args)
{
  pthread_t tid;
  int ret = pthread_create(&tid, NULL, surface_work, args);
  assert (ret == 0);
  pthread_detach(tid);
}

