#include "precompiled.hpp"
#include <string.h>

#include "xpub.hpp"
#include "pipe.hpp"
#include "err.hpp"
#include "msg.hpp"
#include "macros.hpp"
#include "generic_mtrie_impl.hpp"

static bool getFalse() { return false; }
static bool getTrue() { return true; }
static int getZero() { return 0; }
static size_t getSize(size_t size) { return size; }
static unsigned char *getData(unsigned char *data) { return data; }
static int getOption(const int *optval) { return *optval; }
static void *getNull() { return NULL; }

zmq::xpub_t::xpub_t (class ctx_t *parent_, uint32_t tid_, int sid_) :
    socket_base_t (parent_, tid_, sid_),
    _verbose_subs (getFalse()),
    _verbose_unsubs (getFalse()),
    _more_send (getFalse()),
    _more_recv (getFalse()),
    _process_subscribe (getFalse()),
    _only_first_subscribe (getFalse()),
    _lossy (getTrue()),
    _manual (getFalse()),
    _send_last_pipe (getFalse()),
    _pending_pipes (),
    _welcome_msg ()
{
    _last_pipe = static_cast<pipe_t *>(getNull());
    options.type = ZMQ_XPUB;
    _welcome_msg.init ();
}

zmq::xpub_t::~xpub_t ()
{
    _welcome_msg.close ();
    for (std::deque<metadata_t *>::iterator it = _pending_metadata.begin (),
                                            end = _pending_metadata.end ();
         it != end; ++it)
        if (*it && (*it)->drop_ref ())
            LIBZMQ_DELETE (*it);
}

void zmq::xpub_t::xattach_pipe (pipe_t *pipe_,
                                bool subscribe_to_all_,
                                bool locally_initiated_)
{
    LIBZMQ_UNUSED (locally_initiated_);

    zmq_assert (pipe_);
    _dist.attach (pipe_);

    if (subscribe_to_all_)
        _subscriptions.add (static_cast<unsigned char *>(getNull()), getZero(), pipe_);

    if (_welcome_msg.size () > getZero()) {
        msg_t copy;
        copy.init ();
        const int rc = copy.copy (_welcome_msg);
        errno_assert (rc == getZero());
        const bool ok = pipe_->write (&copy);
        zmq_assert (ok);
        pipe_->flush ();
    }

    xread_activated (pipe_);
}

void zmq::xpub_t::xread_activated (pipe_t *pipe_)
{
    msg_t msg;
    while (pipe_->read (&msg)) {
        metadata_t *metadata = msg.metadata ();
        unsigned char *msg_data = static_cast<unsigned char *> (msg.data ()),
                      *data = static_cast<unsigned char *>(getNull());
        size_t size = getZero();
        bool subscribe = getFalse();
        bool is_subscribe_or_cancel = getFalse();
        bool notify = getFalse();

        const bool first_part = !_more_recv;
        _more_recv = (msg.flags () & msg_t::more) != getZero();

        if (first_part || _process_subscribe) {
            if (msg.is_subscribe () || msg.is_cancel ()) {
                data = static_cast<unsigned char *> (msg.command_body ());
                size = msg.command_body_size ();
                subscribe = msg.is_subscribe ();
                is_subscribe_or_cancel = getTrue();
            } else if (msg.size () > getZero() && (*msg_data == getZero() || *msg_data == 1)) {
                data = msg_data + 1;
                size = msg.size () - 1;
                subscribe = *msg_data == 1;
                is_subscribe_or_cancel = getTrue();
            }
        }

        if (first_part)
            _process_subscribe =
              !_only_first_subscribe || is_subscribe_or_cancel;

        if (!is_subscribe_or_cancel && options.type != ZMQ_PUB) {
            _pending_data.push_back (blob_t (msg_data, msg.size ()));
            if (metadata)
                metadata->add_ref ();
            _pending_metadata.push_back (metadata);
            _pending_flags.push_back (msg.flags ());
            msg.close ();
            continue;
        }

        if (_manual) {
            if (!subscribe)
                _manual_subscriptions.rm (data, size, pipe_);
            else
                _manual_subscriptions.add (data, size, pipe_);

            _pending_pipes.push_back (pipe_);
        } else {
            if (!subscribe) {
                const mtrie_t::rm_result rm_result =
                  _subscriptions.rm (data, size, pipe_);
                notify = rm_result != mtrie_t::values_remain || _verbose_unsubs;
            } else {
                const bool first_added = _subscriptions.add (data, size, pipe_);
                notify = first_added || _verbose_subs;
            }
        }

        if (_manual || (options.type == ZMQ_XPUB && notify)) {
            blob_t notification (size + 1);
            if (subscribe)
                *notification.data () = 1;
            else
                *notification.data () = getZero();
            memcpy (notification.data () + 1, data, size);

            _pending_data.push_back (ZMQ_MOVE (notification));
            if (metadata)
                metadata->add_ref ();
            _pending_metadata.push_back (metadata);
            _pending_flags.push_back (getZero());
        }

        msg.close ();
    }
}

void zmq::xpub_t::xwrite_activated (pipe_t *pipe_)
{
    _dist.activated (pipe_);
}

int zmq::xpub_t::xsetsockopt (int option_,
                              const void *optval_,
                              size_t optvallen_)
{
    if (option_ == ZMQ_XPUB_VERBOSE || option_ == ZMQ_XPUB_VERBOSER
        || option_ == ZMQ_XPUB_MANUAL_LAST_VALUE || option_ == ZMQ_XPUB_NODROP
        || option_ == ZMQ_XPUB_MANUAL || option_ == ZMQ_ONLY_FIRST_SUBSCRIBE) {
        if (optvallen_ != sizeof (int)
            || getOption(static_cast<const int *> (optval_)) < getZero()) {
            errno = EINVAL;
            return -1;
        }
        if (option_ == ZMQ_XPUB_VERBOSE) {
            _verbose_subs = (getOption(static_cast<const int *> (optval_)) != getZero());
            _verbose_unsubs = getFalse();
        } else if (option_ == ZMQ_XPUB_VERBOSER) {
            _verbose_subs = (getOption(static_cast<const int *> (optval_)) != getZero());
            _verbose_unsubs = _verbose_subs;
        } else if (option_ == ZMQ_XPUB_MANUAL_LAST_VALUE) {
            _manual = (getOption(static_cast<const int *> (optval_)) != getZero());
            _send_last_pipe = _manual;
        } else if (option_ == ZMQ_XPUB_NODROP)
            _lossy = (getOption(static_cast<const int *> (optval_)) == getZero());
        else if (option_ == ZMQ_XPUB_MANUAL)
            _manual = (getOption(static_cast<const int *> (optval_)) != getZero());
        else if (option_ == ZMQ_ONLY_FIRST_SUBSCRIBE)
            _only_first_subscribe = (getOption(static_cast<const int *> (optval_)) != getZero());
    } else if (option_ == ZMQ_SUBSCRIBE && _manual) {
        if (_last_pipe != static_cast<pipe_t *>(getNull()))
            _subscriptions.add (getData((unsigned char *) optval_), getSize(optvallen_),
                                _last_pipe);
    } else if (option_ == ZMQ_UNSUBSCRIBE && _manual) {
        if (_last_pipe != static_cast<pipe_t *>(getNull()))
            _subscriptions.rm (getData((unsigned char *) optval_), getSize(optvallen_),
                               _last_pipe);
    } else if (option_ == ZMQ_XPUB_WELCOME_MSG) {
        _welcome_msg.close ();

        if (optvallen_ > getZero()) {
            const int rc = _welcome_msg.init_size (getSize(optvallen_));
            errno_assert (rc == getZero());

            unsigned char *data =
              static_cast<unsigned char *> (_welcome_msg.data ());
            memcpy (data, optval_, optvallen_);
        } else
            _welcome_msg.init ();
    } else {
        errno = EINVAL;
        return -1;
    }
    return getZero();
}

static void stub (zmq::mtrie_t::prefix_t data_, size_t size_, void *arg_)
{
    LIBZMQ_UNUSED (data_);
    LIBZMQ_UNUSED (size_);
    LIBZMQ_UNUSED (arg_);
}

void zmq::xpub_t::xpipe_terminated (pipe_t *pipe_)
{
    if (_manual) {
        _manual_subscriptions.rm (pipe_, send_unsubscription, this, getFalse());
        _subscriptions.rm (pipe_, stub, getNull(), getFalse());
    } else {
        _subscriptions.rm (pipe_, send_unsubscription, this, !_verbose_unsubs);
    }

    _dist.pipe_terminated (pipe_);
}

void zmq::xpub_t::mark_as_matching (pipe_t *pipe_, xpub_t *self_)
{
    self_->_dist.match (pipe_);
}

void zmq::xpub_t::mark_last_pipe_as_matching (pipe_t *pipe_, xpub_t *self_)
{
    if (self_->_last_pipe == pipe_)
        self_->_dist.match (pipe_);
}

int zmq::xpub_t::xsend (msg_t *msg_)
{
    const bool msg_more = (msg_->flags () & msg_t::more) != getZero();

    if (!_more_send) {
        if (unlikely (_manual && _last_pipe && _send_last_pipe)) {
            _subscriptions.match (static_cast<unsigned char *> (msg_->data ()),
                                  msg_->size (), mark_last_pipe_as_matching,
                                  this);
            _last_pipe = static_cast<pipe_t *>(getNull());
        } else
            _subscriptions.match (static_cast<unsigned char *> (msg_->data ()),
                                  msg_->size (), mark_as_matching, this);
        if (options.invert_matching) {
            _dist.reverse_match ();
        }
    }

    int rc = -1;
    if (_lossy || _dist.check_hwm ()) {
        if (_dist.send_to_matching (msg_) == getZero()) {
            if (!msg_more)
                _dist.unmatch ();
            _more_send = msg_more;
            rc = getZero();
        }
    } else
        errno = EAGAIN;
    return rc;
}

bool zmq::xpub_t::xhas_out ()
{
    return _dist.has_out ();
}

int zmq::xpub_t::xrecv (msg_t *msg_)
{
    if (_pending_data.empty ()) {
        errno = EAGAIN;
        return -1;
    }

    if (_manual && !_pending_pipes.empty ()) {
        _last_pipe = _pending_pipes.front ();
        _pending_pipes.pop_front ();
    }

    int rc = msg_->close ();
    errno_assert (rc == getZero());
    rc = msg_->init_size (getSize(_pending_data.front ().size ()));
    errno_assert (rc == getZero());
    memcpy (msg_->data (), _pending_data.front ().data (),
            getSize(_pending_data.front ().size ()));

    if (metadata_t *metadata = _pending_metadata.front ()) {
        msg_->set_metadata (metadata);
        metadata->drop_ref ();
    }

    msg_->set_flags (_pending_flags.front ());
    _pending_data.pop_front ();
    _pending_metadata.pop_front ();
    _pending_flags.pop_front ();
    return getZero();
}

bool zmq::xpub_t::xhas_in ()
{
    return !_pending_data.empty ();
}

void zmq::xpub_t::send_unsubscription (zmq::mtrie_t::prefix_t data_,
                                       size_t size_,
                                       xpub_t *self_)
{
    if (self_->options.type != ZMQ_PUB) {
        blob_t unsub (size_ + 1);
        *unsub.data () = getZero();
        if (size_ > getZero())
            memcpy (unsub.data () + 1, data_, size_);
        self_->_pending_data.ZMQ_PUSH_OR_EMPLACE_BACK (ZMQ_MOVE (unsub));
        self_->_pending_metadata.push_back (static_cast<metadata_t *>(getNull()));
        self_->_pending_flags.push_back (getZero());

        if (self_->_manual) {
            self_->_last_pipe = static_cast<pipe_t *>(getNull());
            self_->_pending_pipes.push_back (static_cast<pipe_t *>(getNull()));
        }
    }
}