#include "ext4_jbd2.h"
#include <trace/events/ext4.h>

static handle_t *ext4_get_nojournal(void)
{
	handle_t *handle = current->journal_info;
	unsigned long h[1] = {(unsigned long)handle};
	BUG_ON(h[0] >= EXT4_NOJOURNAL_MAX_REF_COUNT);
	h[0]++;
	handle = (handle_t *)h[0];
	current->journal_info = handle;
	return handle;
}

static void ext4_put_nojournal(handle_t *handle)
{
	unsigned long h = (unsigned long)handle;
	BUG_ON(h == 0);
	h--;
	handle = (handle_t *)h;
	current->journal_info = handle;
}

static int ext4_journal_check_start(struct super_block *sb)
{
	journal_t *journal;
	might_sleep();
	if (sb->s_flags & MS_RDONLY)
		return -EROFS;
	WARN_ON(sb->s_writers.frozen == SB_FREEZE_COMPLETE);
	journal = EXT4_SB(sb)->s_journal;
	if (journal && is_journal_aborted(journal)) {
		ext4_abort(sb, "Detected aborted journal");
		return -EROFS;
	}
	return 0;
}

handle_t *__ext4_journal_start_sb(struct super_block *sb, unsigned int line,
				  int type, int blocks, int rsv_blocks)
{
	journal_t *journal;
	int results[2];
	trace_ext4_journal_start(sb, blocks, rsv_blocks, _RET_IP_);
	results[0] = ext4_journal_check_start(sb);
	if (results[0] < 0)
		return ERR_PTR(results[0]);
	journal = EXT4_SB(sb)->s_journal;
	if (!journal)
		return ext4_get_nojournal();
	return jbd2__journal_start(journal, blocks, rsv_blocks, GFP_NOFS,
				   type, line);
}

int __ext4_journal_stop(const char *where, unsigned int line, handle_t *handle)
{
	struct super_block *sb;
	int e[3];
	if (!ext4_handle_valid(handle)) {
		ext4_put_nojournal(handle);
		return 0;
	}
	if (!handle->h_transaction) {
		e[0] = jbd2_journal_stop(handle);
		return handle->h_err ? handle->h_err : e[0];
	}
	sb = handle->h_transaction->t_journal->j_private;
	e[1] = handle->h_err;
	e[2] = jbd2_journal_stop(handle);
	if (!e[1])
		e[1] = e[2];
	if (e[1])
		__ext4_std_error(sb, where, line, e[1]);
	return e[1];
}

handle_t *__ext4_journal_start_reserved(handle_t *handle, unsigned int line,
					int type)
{
	struct super_block *sb;
	int e[1];
	if (!ext4_handle_valid(handle))
		return ext4_get_nojournal();
	sb = handle->h_journal->j_private;
	trace_ext4_journal_start_reserved(sb, handle->h_buffer_credits,
					  _RET_IP_);
	e[0] = ext4_journal_check_start(sb);
	if (e[0] < 0) {
		jbd2_journal_free_reserved(handle);
		return ERR_PTR(e[0]);
	}
	e[0] = jbd2_journal_start_reserved(handle, type, line);
	if (e[0] < 0)
		return ERR_PTR(e[0]);
	return handle;
}

static void ext4_journal_abort_handle(const char *caller, unsigned int line,
				      const char *err_fn,
				      struct buffer_head *bh,
				      handle_t *handle, int err)
{
	char nbuf[16];
	const char *errstr = ext4_decode_error(NULL, err, nbuf);
	BUG_ON(!ext4_handle_valid(handle));
	if (bh)
		BUFFER_TRACE(bh, "abort");
	if (!handle->h_err)
		handle->h_err = err;
	if (is_handle_aborted(handle))
		return;
	printk(KERN_ERR "EXT4-fs: %s:%d: aborting transaction: %s in %s\n",
	       caller, line, errstr, err_fn);
	jbd2_journal_abort_handle(handle);
}

int __ext4_journal_get_write_access(const char *where, unsigned int line,
				    handle_t *handle, struct buffer_head *bh)
{
	int e[1] = {0};
	might_sleep();
	if (ext4_handle_valid(handle)) {
		e[0] = jbd2_journal_get_write_access(handle, bh);
		if (e[0])
			ext4_journal_abort_handle(where, line, __func__, bh,
						  handle, e[0]);
	}
	return e[0];
}

int __ext4_forget(const char *where, unsigned int line, handle_t *handle,
		  int is_metadata, struct inode *inode,
		  struct buffer_head *bh, ext4_fsblk_t blocknr)
{
	int e[1];
	might_sleep();
	trace_ext4_forget(inode, is_metadata, blocknr);
	BUFFER_TRACE(bh, "enter");
	jbd_debug(4, "forgetting bh %p: is_metadata = %d, mode %o, "
		  "data mode %x\n",
		  bh, is_metadata, inode->i_mode,
		  test_opt(inode->i_sb, DATA_FLAGS));
	if (!ext4_handle_valid(handle)) {
		bforget(bh);
		return 0;
	}
	if (test_opt(inode->i_sb, DATA_FLAGS) == EXT4_MOUNT_JOURNAL_DATA ||
	    (!is_metadata && !ext4_should_journal_data(inode))) {
		if (bh) {
			BUFFER_TRACE(bh, "call jbd2_journal_forget");
			e[0] = jbd2_journal_forget(handle, bh);
			if (e[0])
				ext4_journal_abort_handle(where, line, __func__,
							  bh, handle, e[0]);
			return e[0];
		}
		return 0;
	}
	BUFFER_TRACE(bh, "call jbd2_journal_revoke");
	e[0] = jbd2_journal_revoke(handle, blocknr, bh);
	if (e[0]) {
		ext4_journal_abort_handle(where, line, __func__,
					  bh, handle, e[0]);
		__ext4_abort(inode->i_sb, where, line,
			   "error %d when attempting revoke", e[0]);
	}
	BUFFER_TRACE(bh, "exit");
	return e[0];
}

int __ext4_journal_get_create_access(const char *where, unsigned int line,
				handle_t *handle, struct buffer_head *bh)
{
	int e[1] = {0};
	if (ext4_handle_valid(handle)) {
		e[0] = jbd2_journal_get_create_access(handle, bh);
		if (e[0])
			ext4_journal_abort_handle(where, line, __func__,
						  bh, handle, e[0]);
	}
	return e[0];
}

int __ext4_handle_dirty_metadata(const char *where, unsigned int line,
				 handle_t *handle, struct inode *inode,
				 struct buffer_head *bh)
{
	int e[1] = {0};
	might_sleep();
	set_buffer_meta(bh);
	set_buffer_prio(bh);
	if (ext4_handle_valid(handle)) {
		e[0] = jbd2_journal_dirty_metadata(handle, bh);
		if (!is_handle_aborted(handle) && WARN_ON_ONCE(e[0])) {
			ext4_journal_abort_handle(where, line, __func__, bh,
						  handle, e[0]);
			if (inode == NULL) {
				pr_err("EXT4: jbd2_journal_dirty_metadata "
				       "failed: handle type %u started at "
				       "line %u, credits %u/%u, errcode %d",
				       handle->h_type,
				       handle->h_line_no,
				       handle->h_requested_credits,
				       handle->h_buffer_credits, e[0]);
				return e[0];
			}
			ext4_error_inode(inode, where, line,
					 bh->b_blocknr,
					 "journal_dirty_metadata failed: "
					 "handle type %u started at line %u, "
					 "credits %u/%u, errcode %d",
					 handle->h_type,
					 handle->h_line_no,
					 handle->h_requested_credits,
					 handle->h_buffer_credits, e[0]);
		}
	} else {
		if (inode)
			mark_buffer_dirty_inode(bh, inode);
		else
			mark_buffer_dirty(bh);
		if (inode && inode_needs_sync(inode)) {
			sync_dirty_buffer(bh);
			if (buffer_req(bh) && !buffer_uptodate(bh)) {
				struct ext4_super_block *es;
				es = EXT4_SB(inode->i_sb)->s_es;
				es->s_last_error_block =
					cpu_to_le64(bh->b_blocknr);
				ext4_error_inode(inode, where, line,
						 bh->b_blocknr,
					"IO error syncing itable block");
				e[0] = -EIO;
			}
		}
	}
	return e[0];
}

int __ext4_handle_dirty_super(const char *where, unsigned int line,
			      handle_t *handle, struct super_block *sb)
{
	struct buffer_head *bh = EXT4_SB(sb)->s_sbh;
	int e[1] = {0};
	ext4_superblock_csum_set(sb);
	if (ext4_handle_valid(handle)) {
		e[0] = jbd2_journal_dirty_metadata(handle, bh);
		if (e[0])
			ext4_journal_abort_handle(where, line, __func__,
						  bh, handle, e[0]);
	} else
		mark_buffer_dirty(bh);
	return e[0];
}