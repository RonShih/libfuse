/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001  Miklos Szeredi (mszeredi@inf.bme.hu)

    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#include <linux/fuse.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>

#define MAX_CLEARED 256

/**
 * A Fuse connection.
 *
 * This structure is created, when the client device is opened, and is
 * destroyed, when the client device is closed _and_ the filesystem is
 * umounted.
 */
struct fuse_conn {
	/** The superblock of the mounted filesystem */
	struct super_block *sb;
	
	/** The opened client device */
	struct file *file;

	/** The client wait queue */
	wait_queue_head_t waitq;

	/** The list of pending requests */
	struct list_head pending;

	/** The list of requests being processed */
	struct list_head processing;

	/** The request id */
	int reqctr;
};

/**
 * A request to the client
 */
struct fuse_req {
	/** The request list */
	struct list_head list;

	/** The request ID */
	int unique;

	/** The opcode */
	enum fuse_opcode opcode;
	
	/** The request input size */
	unsigned int insize;

	/** The request input */
	char *in;
	
	/** The maximum request output size */
	unsigned int outsize;

	/** The request output */
	char *out;

	/** The request wait queue */
	wait_queue_head_t waitq;
};


#define INO_FC(inode) ((struct fuse_conn *) (inode)->i_sb->u.generic_sbp)
#define DEV_FC(file) ((struct fuse_conn *) (file)->private_data)

struct fuse_in {
	struct fuse_in_header h;
	unsigned int argsize;
	const void *arg;
};

struct fuse_out {
	struct fuse_out_header h;
	unsigned int argsize;
	unsigned int argvar;
	void *arg;
};

#define FUSE_IN_INIT { {0, 0, 0, current->fsuid, current->fsgid}, 0, 0 }
#define FUSE_OUT_INIT { {0, 0}, 0, 0, 0 }


/**
 * The proc entry for the client device ("/proc/fs/fuse/dev")
 */
extern struct proc_dir_entry *proc_fuse_dev;

/**
 * The lock to protect fuses structures
 */
extern spinlock_t fuse_lock;


/**
 * Get a filled in inode
 */
struct inode *fuse_iget(struct super_block *sb, ino_t ino,
			struct fuse_attr *attr, int version);


/**
 * Initialise operations on regular file
 */
void fuse_init_file_inode(struct inode *inode);

/**
 * Check if the connection can be released, and if yes, then free the
 * connection structure
 */
void fuse_release_conn(struct fuse_conn *fc);

/**
 * Initialize the client device
 */
int fuse_dev_init(void);

/**
 * Cleanup the client device
 */
void fuse_dev_cleanup(void);

/**
 * Initialize the fuse filesystem 
 */
int fuse_fs_init(void);

/**
 * Cleanup the fuse filesystem
 */
void fuse_fs_cleanup(void);

/**
 * Send a request
 *
 */
void request_send(struct fuse_conn *fc, struct fuse_in *in,
		  struct fuse_out *out);

/*
 * Local Variables:
 * indent-tabs-mode: t
 * c-basic-offset: 8
 */