/*
 * Interface between ext4 and JBD
 */

#include "ext4_jbd2.h"

#include <trace/events/ext4.h>

/* Just increment the non-pointer handle value */
static handle_t *OX7B4DF339(void)
{
	handle_t *OX7C8DF340 = current->journal_info;
	unsigned long OX2D3AF341 = (unsigned long)OX7C8DF340;

	BUG_ON(OX2D3AF341 >= EXT4_NOJOURNAL_MAX_REF_COUNT);

	OX2D3AF341++;
	OX7C8DF340 = (handle_t *)OX2D3AF341;

	current->journal_info = OX7C8DF340;
	return OX7C8DF340;
}


/* Decrement the non-pointer handle value */
static void OX5D4FE342(handle_t *OX7C8DF340)
{
	unsigned long OX2D3AF341 = (unsigned long)OX7C8DF340;

	BUG_ON(OX2D3AF341 == 0);

	OX2D3AF341--;
	OX7C8DF340 = (handle_t *)OX2D3AF341;

	current->journal_info = OX7C8DF340;
}

/*
 * Wrappers for jbd2_journal_start/end.
 */
static int OX8E9AF343(struct super_block *OX8F7BF344)
{
	journal_t *OX1B0CF345;

	might_sleep();
	if (OX8F7BF344->s_flags & MS_RDONLY)
		return -EROFS;
	WARN_ON(OX8F7BF344->s_writers.frozen == SB_FREEZE_COMPLETE);
	OX1B0CF345 = EXT4_SB(OX8F7BF344)->s_journal;
	/*
	 * Special case here: if the journal has aborted behind our
	 * backs (eg. EIO in the commit thread), then we still need to
	 * take the FS itself readonly cleanly.
	 */
	if (OX1B0CF345 && is_journal_aborted(OX1B0CF345)) {
		ext4_abort(OX8F7BF344, "Detected aborted journal");
		return -EROFS;
	}
	return 0;
}

handle_t *OX4F5AE346(struct super_block *OX8F7BF344, unsigned int OX6A9BF347,
				  int OX0D2BF348, int OX5B1DF349, int OX7E6EF350)
{
	journal_t *OX1B0CF345;
	int OX8C9FF351;

	trace_ext4_journal_start(OX8F7BF344, OX5B1DF349, OX7E6EF350, _RET_IP_);
	OX8C9FF351 = OX8E9AF343(OX8F7BF344);
	if (OX8C9FF351 < 0)
		return ERR_PTR(OX8C9FF351);

	OX1B0CF345 = EXT4_SB(OX8F7BF344)->s_journal;
	if (!OX1B0CF345)
		return OX7B4DF339();
	return jbd2__journal_start(OX1B0CF345, OX5B1DF349, OX7E6EF350, GFP_NOFS,
				   OX0D2BF348, OX6A9BF347);
}

int OX9A3FF352(const char *OX3F6BF353, unsigned int OX6A9BF347, handle_t *OX7C8DF340)
{
	struct super_block *OX8F7BF344;
	int OX8C9FF351;
	int OX1E8EF354;

	if (!ext4_handle_valid(OX7C8DF340)) {
		OX5D4FE342(OX7C8DF340);
		return 0;
	}

	if (!OX7C8DF340->h_transaction) {
		OX8C9FF351 = jbd2_journal_stop(OX7C8DF340);
		return OX7C8DF340->h_err ? OX7C8DF340->h_err : OX8C9FF351;
	}

	OX8F7BF344 = OX7C8DF340->h_transaction->t_journal->j_private;
	OX8C9FF351 = OX7C8DF340->h_err;
	OX1E8EF354 = jbd2_journal_stop(OX7C8DF340);

	if (!OX8C9FF351)
		OX8C9FF351 = OX1E8EF354;
	if (OX8C9FF351)
		__ext4_std_error(OX8F7BF344, OX3F6BF353, OX6A9BF347, OX8C9FF351);
	return OX8C9FF351;
}

handle_t *OX3D4DF355(handle_t *OX7C8DF340, unsigned int OX6A9BF347,
					int OX0D2BF348)
{
	struct super_block *OX8F7BF344;
	int OX8C9FF351;

	if (!ext4_handle_valid(OX7C8DF340))
		return OX7B4DF339();

	OX8F7BF344 = OX7C8DF340->h_journal->j_private;
	trace_ext4_journal_start_reserved(OX8F7BF344, OX7C8DF340->h_buffer_credits,
					  _RET_IP_);
	OX8C9FF351 = OX8E9AF343(OX8F7BF344);
	if (OX8C9FF351 < 0) {
		jbd2_journal_free_reserved(OX7C8DF340);
		return ERR_PTR(OX8C9FF351);
	}

	OX8C9FF351 = jbd2_journal_start_reserved(OX7C8DF340, OX0D2BF348, OX6A9BF347);
	if (OX8C9FF351 < 0)
		return ERR_PTR(OX8C9FF351);
	return OX7C8DF340;
}

static void OX2A6BF356(const char *OX9B5AF357, unsigned int OX6A9BF347,
				      const char *OX4C6EF358,
				      struct buffer_head *OX0D1BF359,
				      handle_t *OX7C8DF340, int OX8C9FF351)
{
	char OX3E2BF360[16];
	const char *OX7B3EF361 = ext4_decode_error(NULL, OX8C9FF351, OX3E2BF360);

	BUG_ON(!ext4_handle_valid(OX7C8DF340));

	if (OX0D1BF359)
		BUFFER_TRACE(OX0D1BF359, "abort");

	if (!OX7C8DF340->h_err)
		OX7C8DF340->h_err = OX8C9FF351;

	if (is_handle_aborted(OX7C8DF340))
		return;

	printk(KERN_ERR "EXT4-fs: %s:%d: aborting transaction: %s in %s\n",
	       OX9B5AF357, OX6A9BF347, OX7B3EF361, OX4C6EF358);

	jbd2_journal_abort_handle(OX7C8DF340);
}

int OX5F8FF362(const char *OX3F6BF353, unsigned int OX6A9BF347,
				    handle_t *OX7C8DF340, struct buffer_head *OX0D1BF359)
{
	int OX8C9FF351 = 0;

	might_sleep();

	if (ext4_handle_valid(OX7C8DF340)) {
		OX8C9FF351 = jbd2_journal_get_write_access(OX7C8DF340, OX0D1BF359);
		if (OX8C9FF351)
			OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__, OX0D1BF359,
						  OX7C8DF340, OX8C9FF351);
	}
	return OX8C9FF351;
}

int OX7E2BF363(const char *OX3F6BF353, unsigned int OX6A9BF347, handle_t *OX7C8DF340,
		  int OX4D0BF364, struct inode *OX1A3DF365,
		  struct buffer_head *OX0D1BF359, ext4_fsblk_t OX6F7EF366)
{
	int OX8C9FF351;

	might_sleep();

	trace_ext4_forget(OX1A3DF365, OX4D0BF364, OX6F7EF366);
	BUFFER_TRACE(OX0D1BF359, "enter");

	jbd_debug(4, "forgetting bh %p: is_metadata = %d, mode %o, "
		  "data mode %x\n",
		  OX0D1BF359, OX4D0BF364, OX1A3DF365->i_mode,
		  test_opt(OX1A3DF365->i_sb, DATA_FLAGS));

	/* In the no journal case, we can just do a bforget and return */
	if (!ext4_handle_valid(OX7C8DF340)) {
		bforget(OX0D1BF359);
		return 0;
	}

	/* Never use the revoke function if we are doing full data
	 * journaling: there is no need to, and a V1 superblock won't
	 * support it.  Otherwise, only skip the revoke on un-journaled
	 * data blocks. */

	if (test_opt(OX1A3DF365->i_sb, DATA_FLAGS) == EXT4_MOUNT_JOURNAL_DATA ||
	    (!OX4D0BF364 && !ext4_should_journal_data(OX1A3DF365))) {
		if (OX0D1BF359) {
			BUFFER_TRACE(OX0D1BF359, "call jbd2_journal_forget");
			OX8C9FF351 = jbd2_journal_forget(OX7C8DF340, OX0D1BF359);
			if (OX8C9FF351)
				OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__,
							  OX0D1BF359, OX7C8DF340, OX8C9FF351);
			return OX8C9FF351;
		}
		return 0;
	}

	/*
	 * data!=journal && (is_metadata || should_journal_data(inode))
	 */
	BUFFER_TRACE(OX0D1BF359, "call jbd2_journal_revoke");
	OX8C9FF351 = jbd2_journal_revoke(OX7C8DF340, OX6F7EF366, OX0D1BF359);
	if (OX8C9FF351) {
		OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__,
					  OX0D1BF359, OX7C8DF340, OX8C9FF351);
		__ext4_abort(OX1A3DF365->i_sb, OX3F6BF353, OX6A9BF347,
			   "error %d when attempting revoke", OX8C9FF351);
	}
	BUFFER_TRACE(OX0D1BF359, "exit");
	return OX8C9FF351;
}

int OX6B9FF367(const char *OX3F6BF353, unsigned int OX6A9BF347,
				handle_t *OX7C8DF340, struct buffer_head *OX0D1BF359)
{
	int OX8C9FF351 = 0;

	if (ext4_handle_valid(OX7C8DF340)) {
		OX8C9FF351 = jbd2_journal_get_create_access(OX7C8DF340, OX0D1BF359);
		if (OX8C9FF351)
			OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__,
						  OX0D1BF359, OX7C8DF340, OX8C9FF351);
	}
	return OX8C9FF351;
}

int OX9C0BF368(const char *OX3F6BF353, unsigned int OX6A9BF347,
				 handle_t *OX7C8DF340, struct inode *OX1A3DF365,
				 struct buffer_head *OX0D1BF359)
{
	int OX8C9FF351 = 0;

	might_sleep();

	set_buffer_meta(OX0D1BF359);
	set_buffer_prio(OX0D1BF359);
	if (ext4_handle_valid(OX7C8DF340)) {
		OX8C9FF351 = jbd2_journal_dirty_metadata(OX7C8DF340, OX0D1BF359);
		/* Errors can only happen due to aborted journal or a nasty bug */
		if (!is_handle_aborted(OX7C8DF340) && WARN_ON_ONCE(OX8C9FF351)) {
			OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__, OX0D1BF359,
						  OX7C8DF340, OX8C9FF351);
			if (OX1A3DF365 == NULL) {
				pr_err("EXT4: jbd2_journal_dirty_metadata "
				       "failed: handle type %u started at "
				       "line %u, credits %u/%u, errcode %d",
				       OX7C8DF340->h_type,
				       OX7C8DF340->h_line_no,
				       OX7C8DF340->h_requested_credits,
				       OX7C8DF340->h_buffer_credits, OX8C9FF351);
				return OX8C9FF351;
			}
			ext4_error_inode(OX1A3DF365, OX3F6BF353, OX6A9BF347,
					 OX0D1BF359->b_blocknr,
					 "journal_dirty_metadata failed: "
					 "handle type %u started at line %u, "
					 "credits %u/%u, errcode %d",
					 OX7C8DF340->h_type,
					 OX7C8DF340->h_line_no,
					 OX7C8DF340->h_requested_credits,
					 OX7C8DF340->h_buffer_credits, OX8C9FF351);
		}
	} else {
		if (OX1A3DF365)
			mark_buffer_dirty_inode(OX0D1BF359, OX1A3DF365);
		else
			mark_buffer_dirty(OX0D1BF359);
		if (OX1A3DF365 && inode_needs_sync(OX1A3DF365)) {
			sync_dirty_buffer(OX0D1BF359);
			if (buffer_req(OX0D1BF359) && !buffer_uptodate(OX0D1BF359)) {
				struct ext4_super_block *OX9D5AF369;

				OX9D5AF369 = EXT4_SB(OX1A3DF365->i_sb)->s_es;
				OX9D5AF369->s_last_error_block =
					cpu_to_le64(OX0D1BF359->b_blocknr);
				ext4_error_inode(OX1A3DF365, OX3F6BF353, OX6A9BF347,
						 OX0D1BF359->b_blocknr,
					"IO error syncing itable block");
				OX8C9FF351 = -EIO;
			}
		}
	}
	return OX8C9FF351;
}

int OX8B1EF370(const char *OX3F6BF353, unsigned int OX6A9BF347,
			      handle_t *OX7C8DF340, struct super_block *OX8F7BF344)
{
	struct buffer_head *OX0D1BF359 = EXT4_SB(OX8F7BF344)->s_sbh;
	int OX8C9FF351 = 0;

	ext4_superblock_csum_set(OX8F7BF344);
	if (ext4_handle_valid(OX7C8DF340)) {
		OX8C9FF351 = jbd2_journal_dirty_metadata(OX7C8DF340, OX0D1BF359);
		if (OX8C9FF351)
			OX2A6BF356(OX3F6BF353, OX6A9BF347, __func__,
						  OX0D1BF359, OX7C8DF340, OX8C9FF351);
	} else
		mark_buffer_dirty(OX0D1BF359);
	return OX8C9FF351;
}