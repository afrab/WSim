
/**
 * \file   libselect.h
 * \brief  Multiple client select library
 * \author Antoine Fraboulet
 * \date   2006
 **/

#ifndef LIBSELECT_H
#define LIBSELECT_H

/***************************************************/
/***************************************************/
/***************************************************/

/*
libselect
  ptty        // io using fifo mems
  
signal:
  GDB         // tcp/ip gdb serial link
  Console     // tcp/ip console link 
  Worldsens   // multicast IP
*/

/***************************************************/
/***************************************************/
/***************************************************/

/**
 * libselect_init is called from main() just after option parsing 
 * and before machine_create() so that devices can register
 * their own handlers
 */
int libselect_init(int ws_mode);

/**
 * libselect_close is called from main() after machine_delete() and before 
 * logger_close()
 */
int libselect_close(void);


/**
 * libselect_update is called from main event loop, pointer is set to 
 * NULL when no handler has been registered.
 */
extern int  (*libselect_update_ptr) (void);

#define LIBSELECT_UPDATE()                    \
do {                                          \
  if (libselect_update_ptr != NULL)           \
    libselect_update_ptr();                   \
} while (0)


/***************************************************/
/***************************************************/
/***************************************************/

#define LIBSELECT_EVT_NONE  0
#define LIBSELECT_EVT_CLOSE 1

typedef int  libselect_id_t;
typedef void (*libselect_callback)(libselect_id_t, uint64_t flags, void *ptr);

libselect_id_t libselect_id_create   (char *name, int flags);
int            libselect_id_is_valid (libselect_id_t id);
int            libselect_id_close    (libselect_id_t id);

/**
 * libselect_register_fd is called from devices
 * returns : 0=ok else error
 */
int libselect_id_register(libselect_id_t id);

/**
 * libselect_unregister is called from devices
 */
int libselect_id_unregister(libselect_id_t id);

/**
 * libselect_add_callback
 * callback function is used on data errors (ie. close())
 * returns : 0=ok else error
 */
int libselect_id_add_callback(libselect_id_t fd, libselect_callback callback, void *ptr);

/**
 * libselect_read is called from devices as a non blocking read,
 * this function only returns value according to select() results
 * returns : size if ok, -1 on error
 */
uint32_t libselect_id_read  (libselect_id_t id, uint8_t *data, uint32_t size);
uint32_t libselect_id_write (libselect_id_t id, uint8_t *data, uint32_t size);

/***************************************************/
/***************************************************/
/***************************************************/

int libselect_fd_register    (int fd, unsigned int signal);
int libselect_fd_unregister  (int fd);

/***************************************************/
/***************************************************/
/***************************************************/

void libselect_state_save    (void);
void libselect_state_restore (void);

/***************************************************/
/***************************************************/
/***************************************************/

#endif

/***************************************************/
/***************************************************/
/***************************************************/
