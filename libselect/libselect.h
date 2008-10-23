
/**
 * \file   libselect.h
 * \brief  Multiple client select library
 * \author Antoine Fraboulet
 * \date   2006
 **/

#ifndef LIBSELECT_H
#define LIBSELECT_H

#define LIBSELECT_READ   0x01
#define LIBSELECT_WRITE  0x02
#define LIBSELECT_EXCEPT 0x04
#define LIBSELECT_CLOSE  0x08

typedef void (*libselect_callback)(int fd, uint64_t flags, void *ptr);

/**
 * libselect_init is called from main() just after option parsing 
 * and before machine_create() so that devices can register
 * their own handlers
 */
int libselect_init(void);

/**
 * libselect_close is called from main() after machine_delete() and before 
 * logger_close()
 */
int libselect_close(void);

/**
 * libselect_register_fifo is called from devices
 * returns : 0=ok else error
 */
int libselect_register_fifo(int fd, unsigned int fifo_size);

/**
 * libselect_register_signal
 * returns : 0=ok else error
 */
int libselect_register_signal(int fd, unsigned int signal);

/**
 * libselect_add_callback
 * returns : 0=ok else error
 */
int libselect_add_callback(int fd, libselect_callback callback, void *ptr);

/**
 * libselect_del_callback
 * returns : 0=ok else error
 */
int libselect_del_callback(int fd);

/**
 * libselect_unregister is called from devices
 */
int libselect_unregister(int fd);

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

/**
 * libselect_read is called from devices as a non blocking read,
 * this function only returns value according to select() results
 */
uint32_t libselect_read(int fd, uint8_t *data, uint32_t size);

/**
 * libselect_read_flush
 * returns : number of bytes read
 */
uint32_t libselect_read_flush(int fd);

#endif
