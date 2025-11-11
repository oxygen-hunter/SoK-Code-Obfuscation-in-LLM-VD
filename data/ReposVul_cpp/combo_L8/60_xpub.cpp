#include <Python.h>
#include <string.h>
#include <deque>
#include <errno.h>
#include "xpub.hpp"
#include "pipe.hpp"
#include "err.hpp"
#include "msg.hpp"
#include "macros.hpp"
#include "generic_mtrie_impl.hpp"

extern "C" {
    #include <dlfcn.h>
    typedef void(*zmq_assert_func)(bool);
}

class ctx_t;
namespace zmq {
    class xpub_t : public socket_base_t {
    public:
        xpub_t(ctx_t *parent_, uint32_t tid_, int sid_);
        ~xpub_t();
        void xattach_pipe(pipe_t *pipe_, bool subscribe_to_all_, bool locally_initiated_);
        void xread_activated(pipe_t *pipe_);
        void xwrite_activated(pipe_t *pipe_);
        int xsetsockopt(int option_, const void *optval_, size_t optvallen_);
        void xpipe_terminated(pipe_t *pipe_);
        int xsend(msg_t *msg_);
        bool xhas_out();
        int xrecv(msg_t *msg_);
        bool xhas_in();
        static void send_unsubscription(zmq::mtrie_t::prefix_t data_, size_t size_, xpub_t *self_);
        static void mark_as_matching(pipe_t *pipe_, xpub_t *self_);
        static void mark_last_pipe_as_matching(pipe_t *pipe_, xpub_t *self_);
    private:
        zmq_assert_func zmq_assert;
        void *zmq_assert_handle;
    };
}

zmq::xpub_t::xpub_t(ctx_t *parent_, uint32_t tid_, int sid_) :
    socket_base_t(parent_, tid_, sid_),
    _verbose_subs(false),
    _verbose_unsubs(false),
    _more_send(false),
    _more_recv(false),
    _process_subscribe(false),
    _only_first_subscribe(false),
    _lossy(true),
    _manual(false),
    _send_last_pipe(false),
    _pending_pipes(),
    _welcome_msg() 
{
    zmq_assert_handle = dlopen("libzmq_assert.so", RTLD_LAZY);
    zmq_assert = (zmq_assert_func)dlsym(zmq_assert_handle, "zmq_assert");

    _last_pipe = NULL;
    options.type = ZMQ_XPUB;
    _welcome_msg.init();
}

zmq::xpub_t::~xpub_t() {
    _welcome_msg.close();
    for (std::deque<metadata_t*>::iterator it = _pending_metadata.begin(), 
                                            end = _pending_metadata.end();
         it != end; ++it)
        if (*it && (*it)->drop_ref())
            LIBZMQ_DELETE(*it);

    dlclose(zmq_assert_handle);
}

void zmq::xpub_t::xattach_pipe(pipe_t *pipe_, bool subscribe_to_all_, bool locally_initiated_) {
    LIBZMQ_UNUSED(locally_initiated_);

    zmq_assert(pipe_);
    _dist.attach(pipe_);

    if (subscribe_to_all_)
        _subscriptions.add(NULL, 0, pipe_);

    if (_welcome_msg.size() > 0) {
        msg_t copy;
        copy.init();
        const int rc = copy.copy(_welcome_msg);
        errno_assert(rc == 0);
        const bool ok = pipe_->write(&copy);
        zmq_assert(ok);
        pipe_->flush();
    }

    xread_activated(pipe_);
}

void zmq::xpub_t::xread_activated(pipe_t *pipe_) {
    msg_t msg;
    while (pipe_->read(&msg)) {
        metadata_t *metadata = msg.metadata();
        unsigned char *msg_data = static_cast<unsigned char*>(msg.data()), *data = NULL;
        size_t size = 0;
        bool subscribe = false;
        bool is_subscribe_or_cancel = false;
        bool notify = false;

        const bool first_part = !_more_recv;
        _more_recv = (msg.flags() & msg_t::more) != 0;

        if (first_part || _process_subscribe) {
            if (msg.is_subscribe() || msg.is_cancel()) {
                data = static_cast<unsigned char*>(msg.command_body());
                size = msg.command_body_size();
                subscribe = msg.is_subscribe();
                is_subscribe_or_cancel = true;
            } else if (msg.size() > 0 && (*msg_data == 0 || *msg_data == 1)) {
                data = msg_data + 1;
                size = msg.size() - 1;
                subscribe = *msg_data == 1;
                is_subscribe_or_cancel = true;
            }
        }

        if (first_part)
            _process_subscribe = !_only_first_subscribe || is_subscribe_or_cancel;

        if (!is_subscribe_or_cancel && options.type != ZMQ_PUB) {
            _pending_data.push_back(blob_t(msg_data, msg.size()));
            if (metadata)
                metadata->add_ref();
            _pending_metadata.push_back(metadata);
            _pending_flags.push_back(msg.flags());
            msg.close();
            continue;
        }

        if (_manual) {
            if (!subscribe)
                _manual_subscriptions.rm(data, size, pipe_);
            else
                _manual_subscriptions.add(data, size, pipe_);

            _pending_pipes.push_back(pipe_);
        } else {
            if (!subscribe) {
                const mtrie_t::rm_result rm_result = _subscriptions.rm(data, size, pipe_);
                notify = rm_result != mtrie_t::values_remain || _verbose_unsubs;
            } else {
                const bool first_added = _subscriptions.add(data, size, pipe_);
                notify = first_added || _verbose_subs;
            }
        }

        if (_manual || (options.type == ZMQ_XPUB && notify)) {
            blob_t notification(size + 1);
            if (subscribe)
                *notification.data() = 1;
            else
                *notification.data() = 0;
            memcpy(notification.data() + 1, data, size);

            _pending_data.push_back(ZMQ_MOVE(notification));
            if (metadata)
                metadata->add_ref();
            _pending_metadata.push_back(metadata);
            _pending_flags.push_back(0);
        }

        msg.close();
    }
}

void zmq::xpub_t::xwrite_activated(pipe_t *pipe_) {
    _dist.activated(pipe_);
}

int zmq::xpub_t::xsetsockopt(int option_, const void *optval_, size_t optvallen_) {
    if (option_ == ZMQ_XPUB_VERBOSE || option_ == ZMQ_XPUB_VERBOSER
        || option_ == ZMQ_XPUB_MANUAL_LAST_VALUE || option_ == ZMQ_XPUB_NODROP
        || option_ == ZMQ_XPUB_MANUAL || option_ == ZMQ_ONLY_FIRST_SUBSCRIBE) {
        if (optvallen_ != sizeof(int)
            || *static_cast<const int*>(optval_) < 0) {
            errno = EINVAL;
            return -1;
        }
        if (option_ == ZMQ_XPUB_VERBOSE) {
            _verbose_subs = (*static_cast<const int*>(optval_) != 0);
            _verbose_unsubs = false;
        } else if (option_ == ZMQ_XPUB_VERBOSER) {
            _verbose_subs = (*static_cast<const int*>(optval_) != 0);
            _verbose_unsubs = _verbose_subs;
        } else if (option_ == ZMQ_XPUB_MANUAL_LAST_VALUE) {
            _manual = (*static_cast<const int*>(optval_) != 0);
            _send_last_pipe = _manual;
        } else if (option_ == ZMQ_XPUB_NODROP)
            _lossy = (*static_cast<const int*>(optval_) == 0);
        else if (option_ == ZMQ_XPUB_MANUAL)
            _manual = (*static_cast<const int*>(optval_) != 0);
        else if (option_ == ZMQ_ONLY_FIRST_SUBSCRIBE)
            _only_first_subscribe = (*static_cast<const int*>(optval_) != 0);
    } else if (option_ == ZMQ_SUBSCRIBE && _manual) {
        if (_last_pipe != NULL)
            _subscriptions.add((unsigned char*)optval_, optvallen_,
                                _last_pipe);
    } else if (option_ == ZMQ_UNSUBSCRIBE && _manual) {
        if (_last_pipe != NULL)
            _subscriptions.rm((unsigned char*)optval_, optvallen_,
                               _last_pipe);
    } else if (option_ == ZMQ_XPUB_WELCOME_MSG) {
        _welcome_msg.close();

        if (optvallen_ > 0) {
            const int rc = _welcome_msg.init_size(optvallen_);
            errno_assert(rc == 0);

            unsigned char *data =
              static_cast<unsigned char*>(_welcome_msg.data());
            memcpy(data, optval_, optvallen_);
        } else
            _welcome_msg.init();
    } else {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

void zmq::xpub_t::xpipe_terminated(pipe_t *pipe_) {
    if (_manual) {
        _manual_subscriptions.rm(pipe_, send_unsubscription, this, false);
        _subscriptions.rm(pipe_, stub, static_cast<void*>(NULL), false);
    } else {
        _subscriptions.rm(pipe_, send_unsubscription, this, !_verbose_unsubs);
    }

    _dist.pipe_terminated(pipe_);
}

void zmq::xpub_t::mark_as_matching(pipe_t *pipe_, xpub_t *self_) {
    self_->_dist.match(pipe_);
}

void zmq::xpub_t::mark_last_pipe_as_matching(pipe_t *pipe_, xpub_t *self_) {
    if (self_->_last_pipe == pipe_)
        self_->_dist.match(pipe_);
}

int zmq::xpub_t::xsend(msg_t *msg_) {
    const bool msg_more = (msg_->flags() & msg_t::more) != 0;

    if (!_more_send) {
        if (unlikely(_manual && _last_pipe && _send_last_pipe)) {
            _subscriptions.match(static_cast<unsigned char*>(msg_->data()),
                                  msg_->size(), mark_last_pipe_as_matching,
                                  this);
            _last_pipe = NULL;
        } else
            _subscriptions.match(static_cast<unsigned char*>(msg_->data()),
                                  msg_->size(), mark_as_matching, this);
        if (options.invert_matching) {
            _dist.reverse_match();
        }
    }

    int rc = -1;
    if (_lossy || _dist.check_hwm()) {
        if (_dist.send_to_matching(msg_) == 0) {
            if (!msg_more)
                _dist.unmatch();
            _more_send = msg_more;
            rc = 0;
        }
    } else
        errno = EAGAIN;
    return rc;
}

bool zmq::xpub_t::xhas_out() {
    return _dist.has_out();
}

int zmq::xpub_t::xrecv(msg_t *msg_) {
    if (_pending_data.empty()) {
        errno = EAGAIN;
        return -1;
    }

    if (_manual && !_pending_pipes.empty()) {
        _last_pipe = _pending_pipes.front();
        _pending_pipes.pop_front();
    }

    int rc = msg_->close();
    errno_assert(rc == 0);
    rc = msg_->init_size(_pending_data.front().size());
    errno_assert(rc == 0);
    memcpy(msg_->data(), _pending_data.front().data(),
            _pending_data.front().size());

    if (metadata_t *metadata = _pending_metadata.front()) {
        msg_->set_metadata(metadata);
        metadata->drop_ref();
    }

    msg_->set_flags(_pending_flags.front());
    _pending_data.pop_front();
    _pending_metadata.pop_front();
    _pending_flags.pop_front();
    return 0;
}

bool zmq::xpub_t::xhas_in() {
    return !_pending_data.empty();
}

void zmq::xpub_t::send_unsubscription(zmq::mtrie_t::prefix_t data_,
                                       size_t size_,
                                       xpub_t *self_) {
    if (self_->options.type != ZMQ_PUB) {
        blob_t unsub(size_ + 1);
        *unsub.data() = 0;
        if (size_ > 0)
            memcpy(unsub.data() + 1, data_, size_);
        self_->_pending_data.ZMQ_PUSH_OR_EMPLACE_BACK(ZMQ_MOVE(unsub));
        self_->_pending_metadata.push_back(NULL);
        self_->_pending_flags.push_back(0);

        if (self_->_manual) {
            self_->_last_pipe = NULL;
            self_->_pending_pipes.push_back(NULL);
        }
    }
}

static void stub(zmq::mtrie_t::prefix_t data_, size_t size_, void *arg_) {
    LIBZMQ_UNUSED(data_);
    LIBZMQ_UNUSED(size_);
    LIBZMQ_UNUSED(arg_);
}