#include <crypto/hash.h>
#include <crypto/if_alg.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>

struct OX7B4DF339 {
	struct af_alg_sgl OX8ACF3E1A;
	u8 *OX3A29B9CB;
	struct af_alg_completion OX1E5F1D1F;
	unsigned int OXA8E0A2E7;
	bool OXC6F6A9AB;
	struct ahash_request OX4D5C8D40;
};

static int OX6A9A3F93(struct kiocb *OX6B8F8B72, struct socket *OX8D1C1DFA,
			struct msghdr *OXF1D3D44A, size_t OX5A1C5D9E)
{
	int OX61C4A2B4 = ALG_MAX_PAGES * PAGE_SIZE;
	struct sock *OXA1F1B0B7 = OX8D1C1DFA->sk;
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	struct OX7B4DF339 *OX4A8C1B5A = OX1D8D7E12->private;
	unsigned long OX1176E5B3;
	struct iovec *OXF2B9D72F;
	long OX7A1F7D1A = 0;
	int OXC9B7D064;

	if (OX61C4A2B4 > OXA1F1B0B7->sk_sndbuf)
		OX61C4A2B4 = OXA1F1B0B7->sk_sndbuf;

	lock_sock(OXA1F1B0B7);
	if (!OX4A8C1B5A->OXC6F6A9AB) {
		OXC9B7D064 = crypto_ahash_init(&OX4A8C1B5A->OX4D5C8D40);
		if (OXC9B7D064)
			goto OX6D975A3A;
	}

	OX4A8C1B5A->OXC6F6A9AB = 0;

	for (OXF2B9D72F = OXF1D3D44A->msg_iov, OX1176E5B3 = OXF1D3D44A->msg_iovlen; OX1176E5B3 > 0;
	     OX1176E5B3--, OXF2B9D72F++) {
		unsigned long OX4D9A3F5F = OXF2B9D72F->iov_len;
		char __user *OX2D1F1E9B = OXF2B9D72F->iov_base;

		while (OX4D9A3F5F) {
			int OX7E8B1D6A = min_t(unsigned long, OX4D9A3F5F, OX61C4A2B4);
			int OX4F1D9F91;

			OX4F1D9F91 = af_alg_make_sg(&OX4A8C1B5A->OX8ACF3E1A, OX2D1F1E9B, OX7E8B1D6A, 0);
			if (OX4F1D9F91 < 0) {
				OXC9B7D064 = OX7A1F7D1A ? 0 : OX4F1D9F91;
				goto OX6D975A3A;
			}

			ahash_request_set_crypt(&OX4A8C1B5A->OX4D5C8D40, OX4A8C1B5A->OX8ACF3E1A.sg, NULL,
						OX4F1D9F91);

			OXC9B7D064 = af_alg_wait_for_completion(
				crypto_ahash_update(&OX4A8C1B5A->OX4D5C8D40),
				&OX4A8C1B5A->OX1E5F1D1F);

			af_alg_free_sg(&OX4A8C1B5A->OX8ACF3E1A);

			if (OXC9B7D064)
				goto OX6D975A3A;

			OX4D9A3F5F -= OX4F1D9F91;
			OX2D1F1E9B += OX4F1D9F91;
			OX7A1F7D1A += OX4F1D9F91;
		}
	}

	OXC9B7D064 = 0;

	OX4A8C1B5A->OXC6F6A9AB = OXF1D3D44A->msg_flags & MSG_MORE;
	if (!OX4A8C1B5A->OXC6F6A9AB) {
		ahash_request_set_crypt(&OX4A8C1B5A->OX4D5C8D40, NULL, OX4A8C1B5A->OX3A29B9CB, 0);
		OXC9B7D064 = af_alg_wait_for_completion(crypto_ahash_final(&OX4A8C1B5A->OX4D5C8D40),
						 &OX4A8C1B5A->OX1E5F1D1F);
	}

OX6D975A3A:
	release_sock(OXA1F1B0B7);

	return OXC9B7D064 ?: OX7A1F7D1A;
}

static ssize_t OX5F8F1D1A(struct socket *OX8D1C1DFA, struct page *OX4D9A4F3F,
			     int OX5E8F1D6A, size_t OX7F8C1D9A, int OX4B8A1F9E)
{
	struct sock *OXA1F1B0B7 = OX8D1C1DFA->sk;
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	struct OX7B4DF339 *OX4A8C1B5A = OX1D8D7E12->private;
	int OXC9B7D064;

	lock_sock(OXA1F1B0B7);
	sg_init_table(OX4A8C1B5A->OX8ACF3E1A.sg, 1);
	sg_set_page(OX4A8C1B5A->OX8ACF3E1A.sg, OX4D9A4F3F, OX7F8C1D9A, OX5E8F1D6A);

	ahash_request_set_crypt(&OX4A8C1B5A->OX4D5C8D40, OX4A8C1B5A->OX8ACF3E1A.sg, OX4A8C1B5A->OX3A29B9CB, OX7F8C1D9A);

	if (!(OX4B8A1F9E & MSG_MORE)) {
		if (OX4A8C1B5A->OXC6F6A9AB)
			OXC9B7D064 = crypto_ahash_finup(&OX4A8C1B5A->OX4D5C8D40);
		else
			OXC9B7D064 = crypto_ahash_digest(&OX4A8C1B5A->OX4D5C8D40);
	} else {
		if (!OX4A8C1B5A->OXC6F6A9AB) {
			OXC9B7D064 = crypto_ahash_init(&OX4A8C1B5A->OX4D5C8D40);
			if (OXC9B7D064)
				goto OX4F8C1D7A;
		}

		OXC9B7D064 = crypto_ahash_update(&OX4A8C1B5A->OX4D5C8D40);
	}

	OXC9B7D064 = af_alg_wait_for_completion(OXC9B7D064, &OX4A8C1B5A->OX1E5F1D1F);
	if (OXC9B7D064)
		goto OX4F8C1D7A;

	OX4A8C1B5A->OXC6F6A9AB = OX4B8A1F9E & MSG_MORE;

OX4F8C1D7A:
	release_sock(OXA1F1B0B7);

	return OXC9B7D064 ?: OX7F8C1D9A;
}

static int OXE1C9D0A4(struct kiocb *OX6B8F8B72, struct socket *OX8D1C1DFA,
			struct msghdr *OXF1D3D44A, size_t OX5A1C5D9E, int OX4B8A1F9E)
{
	struct sock *OXA1F1B0B7 = OX8D1C1DFA->sk;
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	struct OX7B4DF339 *OX4A8C1B5A = OX1D8D7E12->private;
	unsigned OX7F9D1E5B = crypto_ahash_digestsize(crypto_ahash_reqtfm(&OX4A8C1B5A->OX4D5C8D40));
	int OXC9B7D064;

	if (OX5A1C5D9E > OX7F9D1E5B)
		OX5A1C5D9E = OX7F9D1E5B;
	else if (OX5A1C5D9E < OX7F9D1E5B)
		OXF1D3D44A->msg_flags |= MSG_TRUNC;

	lock_sock(OXA1F1B0B7);
	if (OX4A8C1B5A->OXC6F6A9AB) {
		OX4A8C1B5A->OXC6F6A9AB = 0;
		ahash_request_set_crypt(&OX4A8C1B5A->OX4D5C8D40, NULL, OX4A8C1B5A->OX3A29B9CB, 0);
		OXC9B7D064 = af_alg_wait_for_completion(crypto_ahash_final(&OX4A8C1B5A->OX4D5C8D40),
						 &OX4A8C1B5A->OX1E5F1D1F);
		if (OXC9B7D064)
			goto OX1F8D7A4C;
	}

	OXC9B7D064 = memcpy_toiovec(OXF1D3D44A->msg_iov, OX4A8C1B5A->OX3A29B9CB, OX5A1C5D9E);

OX1F8D7A4C:
	release_sock(OXA1F1B0B7);

	return OXC9B7D064 ?: OX5A1C5D9E;
}

static int OX4A1F8E0A(struct socket *OX8D1C1DFA, struct socket *OXF3D2A4B5, int OX4B8A1F9E)
{
	struct sock *OXA1F1B0B7 = OX8D1C1DFA->sk;
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	struct OX7B4DF339 *OX4A8C1B5A = OX1D8D7E12->private;
	struct ahash_request *OX3B9D7A4C = &OX4A8C1B5A->OX4D5C8D40;
	char OX7A1F4D3B[crypto_ahash_statesize(crypto_ahash_reqtfm(OX3B9D7A4C))];
	struct sock *OXF2B9C4A1;
	struct alg_sock *OX8D1F7E9C;
	struct OX7B4DF339 *OX1C9A5F6E;
	int OXC9B7D064;

	OXC9B7D064 = crypto_ahash_export(OX3B9D7A4C, OX7A1F4D3B);
	if (OXC9B7D064)
		return OXC9B7D064;

	OXC9B7D064 = af_alg_accept(OX1D8D7E12->parent, OXF3D2A4B5);
	if (OXC9B7D064)
		return OXC9B7D064;

	OXF2B9C4A1 = OXF3D2A4B5->sk;
	OX8D1F7E9C = alg_sk(OXF2B9C4A1);
	OX1C9A5F6E = OX8D1F7E9C->private;
	OX1C9A5F6E->OXC6F6A9AB = 1;

	OXC9B7D064 = crypto_ahash_import(&OX1C9A5F6E->OX4D5C8D40, OX7A1F4D3B);
	if (OXC9B7D064) {
		sock_orphan(OXF2B9C4A1);
		sock_put(OXF2B9C4A1);
	}

	return OXC9B7D064;
}

static struct proto_ops OXE5A9F1D1 = {
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
	.sendmsg	=	OX6A9A3F93,
	.sendpage	=	OX5F8F1D1A,
	.recvmsg	=	OXE1C9D0A4,
	.accept		=	OX4A1F8E0A,
};

static void *OX6A4D1E3B(const char *OX4B9D6E1A, u32 OX7C8F1D1A, u32 OX8C1D5E9A)
{
	return crypto_alloc_ahash(OX4B9D6E1A, OX7C8F1D1A, OX8C1D5E9A);
}

static void OX4A8C3F5A(void *OX2D1F7E9C)
{
	crypto_free_ahash(OX2D1F7E9C);
}

static int OXE3A9C1D7(void *OX2D1F7E9C, const u8 *OX3D1A5E9B, unsigned int OX5E9A1F7C)
{
	return crypto_ahash_setkey(OX2D1F7E9C, OX3D1A5E9B, OX5E9A1F7C);
}

static void OXE7A1F6D9(struct sock *OXA1F1B0B7)
{
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	struct OX7B4DF339 *OX4A8C1B5A = OX1D8D7E12->private;

	sock_kfree_s(OXA1F1B0B7, OX4A8C1B5A->OX3A29B9CB,
		     crypto_ahash_digestsize(crypto_ahash_reqtfm(&OX4A8C1B5A->OX4D5C8D40)));
	sock_kfree_s(OXA1F1B0B7, OX4A8C1B5A, OX4A8C1B5A->OXA8E0A2E7);
	af_alg_release_parent(OXA1F1B0B7);
}

static int OX5E8A1F6B(void *OX2D1F7E9C, struct sock *OXA1F1B0B7)
{
	struct OX7B4DF339 *OX4A8C1B5A;
	struct alg_sock *OX1D8D7E12 = alg_sk(OXA1F1B0B7);
	unsigned OX7D1A4E9B = sizeof(*OX4A8C1B5A) + crypto_ahash_reqsize(OX2D1F7E9C);
	unsigned OX5E9A1F6E = crypto_ahash_digestsize(OX2D1F7E9C);

	OX4A8C1B5A = sock_kmalloc(OXA1F1B0B7, OX7D1A4E9B, GFP_KERNEL);
	if (!OX4A8C1B5A)
		return -ENOMEM;

	OX4A8C1B5A->OX3A29B9CB = sock_kmalloc(OXA1F1B0B7, OX5E9A1F6E, GFP_KERNEL);
	if (!OX4A8C1B5A->OX3A29B9CB) {
		sock_kfree_s(OXA1F1B0B7, OX4A8C1B5A, OX7D1A4E9B);
		return -ENOMEM;
	}

	memset(OX4A8C1B5A->OX3A29B9CB, 0, OX5E9A1F6E);

	OX4A8C1B5A->OXA8E0A2E7 = OX7D1A4E9B;
	OX4A8C1B5A->OXC6F6A9AB = 0;
	af_alg_init_completion(&OX4A8C1B5A->OX1E5F1D1F);

	OX1D8D7E12->private = OX4A8C1B5A;

	ahash_request_set_tfm(&OX4A8C1B5A->OX4D5C8D40, OX2D1F7E9C);
	ahash_request_set_callback(&OX4A8C1B5A->OX4D5C8D40, CRYPTO_TFM_REQ_MAY_BACKLOG,
				   af_alg_complete, &OX4A8C1B5A->OX1E5F1D1F);

	OXA1F1B0B7->sk_destruct = OXE7A1F6D9;

	return 0;
}

static const struct af_alg_type OX7F8A1D9B = {
	.bind		=	OX6A4D1E3B,
	.release	=	OX4A8C3F5A,
	.setkey		=	OXE3A9C1D7,
	.accept		=	OX5E8A1F6B,
	.ops		=	&OXE5A9F1D1,
	.name		=	"hash",
	.owner		=	THIS_MODULE
};

static int __init OX7A9D4C1B(void)
{
	return af_alg_register_type(&OX7F8A1D9B);
}

static void __exit OX7C8F1D3A(void)
{
	int OXC9B7D064 = af_alg_unregister_type(&OX7F8A1D9B);
	BUG_ON(OXC9B7D064);
}

module_init(OX7A9D4C1B);
module_exit(OX7C8F1D3A);
MODULE_LICENSE("GPL");