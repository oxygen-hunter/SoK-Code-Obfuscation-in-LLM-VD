#include <Python.h>
#include <dlfcn.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>

struct hash_ctx {
    struct af_alg_sgl sgl;
    u8 *result;
    struct af_alg_completion completion;
    unsigned int len;
    bool more;
    struct ahash_request req;
};

static int hash_sendmsg(struct kiocb *unused, struct socket *sock,
			struct msghdr *msg, size_t ignored);

static ssize_t hash_sendpage(struct socket *sock, struct page *page,
			     int offset, size_t size, int flags);

static int hash_recvmsg(struct kiocb *unused, struct socket *sock,
			struct msghdr *msg, size_t len, int flags);

static int hash_accept(struct socket *sock, struct socket *newsock, int flags);

static struct proto_ops algif_hash_ops = {
	.family		=	PF_ALG,
	.connect	=	sock_no_connect,
	.socketpair	=	sock_no_socketpair,
	.getname	=	sock_no_getname,
	.ioctl		=	sock_no_ioctl,
	.listen		=	sock_no_listen,
	.shutdown	=	sock_no_shutdown,
	.getsockopt	=	sock_no_getsockopt,
	.mmap		=	sock_no_mmap,
	.bind		=	sock_no_bind,
	.setsockopt	=	sock_no_setsockopt,
	.poll		=	sock_no_poll,
	.release	=	af_alg_release,
	.sendmsg	=	hash_sendmsg,
	.sendpage	=	hash_sendpage,
	.recvmsg	=	hash_recvmsg,
	.accept		=	hash_accept,
};

static void *hash_bind(const char *name, u32 type, u32 mask) {
	return crypto_alloc_ahash(name, type, mask);
}

static void hash_release(void *private) {
	crypto_free_ahash(private);
}

static int hash_setkey(void *private, const u8 *key, unsigned int keylen) {
	return crypto_ahash_setkey(private, key, keylen);
}

static void hash_sock_destruct(struct sock *sk) {
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;
	sock_kfree_s(sk, ctx->result, crypto_ahash_digestsize(crypto_ahash_reqtfm(&ctx->req)));
	sock_kfree_s(sk, ctx, ctx->len);
	af_alg_release_parent(sk);
}

static int hash_accept_parent(void *private, struct sock *sk) {
	struct hash_ctx *ctx;
	struct alg_sock *ask = alg_sk(sk);
	unsigned len = sizeof(*ctx) + crypto_ahash_reqsize(private);
	unsigned ds = crypto_ahash_digestsize(private);

	ctx = sock_kmalloc(sk, len, GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->result = sock_kmalloc(sk, ds, GFP_KERNEL);
	if (!ctx->result) {
		sock_kfree_s(sk, ctx, len);
		return -ENOMEM;
	}

	memset(ctx->result, 0, ds);

	ctx->len = len;
	ctx->more = 0;
	af_alg_init_completion(&ctx->completion);

	ask->private = ctx;

	ahash_request_set_tfm(&ctx->req, private);
	ahash_request_set_callback(&ctx->req, CRYPTO_TFM_REQ_MAY_BACKLOG,
				   af_alg_complete, &ctx->completion);

	sk->sk_destruct = hash_sock_destruct;

	return 0;
}

static const struct af_alg_type algif_type_hash = {
	.bind		=	hash_bind,
	.release	=	hash_release,
	.setkey		=	hash_setkey,
	.accept		=	hash_accept_parent,
	.ops		=	&algif_hash_ops,
	.name		=	"hash",
	.owner		=	THIS_MODULE
};

static void call_python_function(const char* module_name, const char* function_name) {
	Py_Initialize();
	PyObject *pName, *pModule, *pFunc;
	pName = PyUnicode_DecodeFSDefault(module_name);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, function_name);
		if (pFunc && PyCallable_Check(pFunc)) {
			PyObject_CallObject(pFunc, NULL);
			Py_DECREF(pFunc);
		}
		Py_DECREF(pModule);
	}
	Py_Finalize();
}

static int __init algif_hash_init(void) {
	call_python_function("some_python_module", "initialize");
	return af_alg_register_type(&algif_type_hash);
}

static void __exit algif_hash_exit(void) {
	int err = af_alg_unregister_type(&algif_type_hash);
	BUG_ON(err);
}

module_init(algif_hash_init);
module_exit(algif_hash_exit);
MODULE_LICENSE("GPL");