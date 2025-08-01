#ifndef __REDIS_MOD_H__
#define __REDIS_MOD_H__

#include <sys/types.h>
#include <glib.h>
#include <sys/types.h>
#include <hiredis/hiredis.h>

#include "compat.h"
#include "socket.h"
#include "helpers.h"
#include "call.h"
#include "str.h"

#define REDIS_RESTORE_NUM_THREADS 4

enum redis_role {
	MASTER_REDIS_ROLE = 0,
	SLAVE_REDIS_ROLE = 1,
	ANY_REDIS_ROLE = 2,
};

enum redis_state {
	REDIS_STATE_DISCONNECTED = 0,
	REDIS_STATE_CONNECTED,
};

enum event_base_action {
	EVENT_BASE_ALLOC = 0,
	EVENT_BASE_FREE,
	EVENT_BASE_LOOPBREAK,
};

enum subscribe_action {
	SUBSCRIBE_KEYSPACE = 0,
	UNSUBSCRIBE_KEYSPACE,
	UNSUBSCRIBE_ALL,
};




struct redis {
	endpoint_t	endpoint;
	char		host[64];
	const char	*hostname; /* can be a hostname or IP address */
	enum redis_role	role;

	redisContext	*ctx;
	int		db;
	const char	*auth;
	mutex_t		lock;
	unsigned int	pipeline;

	int		state;
	int		no_redis_required;
	int		consecutive_errors;
	int64_t		restore_tick_us;
	int		current_db;

	struct event_base		*async_ev;
	struct redisAsyncContext	*async_ctx;
	mutex_t				async_lock;
	GQueue				async_queue;
	int64_t				async_last;

	bool		update_resolve;
};

struct redis_hash {
	GHashTable *ht;
};

struct redis_list {
	unsigned int len;
	struct redis_hash *rh;
	void **ptrs;
};


extern struct redis		*rtpe_redis;
extern struct redis		*rtpe_redis_write;
extern struct redis		*rtpe_redis_write_disabled;
extern struct redis		*rtpe_redis_notify;


void redis_notify_loop(void *d);
void redis_delete_async_loop(void *d);


struct redis *redis_new(const endpoint_t *, int, const char *, const char *, enum redis_role, int, bool);
struct redis *redis_dup(const struct redis *r, int db);
void redis_close(struct redis *r);
int redis_restore(struct redis *, bool foreign, int db);
void redis_update_onekey(call_t *c, struct redis *r);
void redis_delete(call_t *, struct redis *);
void redis_wipe(struct redis *);
int redis_async_event_base_action(struct redis *r, enum event_base_action);
int redis_notify_subscribe_action(struct redis *r, enum subscribe_action action, int keyspace);
int redis_set_timeout(struct redis* r, int64_t timeout);
int redis_reconnect(struct redis* r);


#endif
