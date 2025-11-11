#include "precompiled.hpp"
#include <string.h>

#include "xpub.hpp"
#include "pipe.hpp"
#include "err.hpp"
#include "msg.hpp"
#include "macros.hpp"
#include "generic_mtrie_impl.hpp"

zmq::OX7B4DF339::OX7B4DF339 (class OX4A6FDD9F *OX5A1B1F1A, uint32_t OX2A6D5B8A, int OX1B2E9F4C) :
    OX3C1E2F0A (OX5A1B1F1A, OX2A6D5B8A, OX1B2E9F4C),
    OX6D2A1E3F (false),
    OX7F3D4B8A (false),
    OX8E4C2D9B (false),
    OX9A5B3E7D (false),
    OXA4C6D1E9 (false),
    OX1B3D7E2A (false),
    OX2C5E8A4D (false),
    OX3D7E9C1B (true),
    OX4A8B1D5E (false),
    OX5B1C8A2D (false),
    OX6C7D4B9E (),
    OX7D8A2E4C ()
{
    OX8E9B1C2D = NULL;
    OX9F2A4C6B.type = ZMQ_XPUB;
    OX7D8A2E4C.init ();
}

zmq::OX7B4DF339::~OX7B4DF339 ()
{
    OX7D8A2E4C.close ();
    for (std::deque<OX7E2B9C1D *>::iterator OX1D3A5C7E = OX6C7D4B9E.begin (),
                                            OX2E4B9D8A = OX6C7D4B9E.end ();
         OX1D3A5C7E != OX2E4B9D8A; ++OX1D3A5C7E)
        if (*OX1D3A5C7E && (*OX1D3A5C7E)->drop_ref ())
            LIBZMQ_DELETE (*OX1D3A5C7E);
}

void zmq::OX7B4DF339::OX1A2B3C4D (OX5B4C2D1E *OX1E2A3B4C,
                                bool OX2C3D4E5F,
                                bool OX3A4B5C6D)
{
    LIBZMQ_UNUSED (OX3A4B5C6D);

    zmq_assert (OX1E2A3B4C);
    OX4C3B2D1F.attach (OX1E2A3B4C);

    if (OX2C3D4E5F)
        OX5D4E3F2A.add (NULL, 0, OX1E2A3B4C);

    if (OX7D8A2E4C.size () > 0) {
        OX6E2F3A4B copy;
        copy.init ();
        const int rc = copy.copy (OX7D8A2E4C);
        errno_assert (rc == 0);
        const bool ok = OX1E2A3B4C->write (&copy);
        zmq_assert (ok);
        OX1E2A3B4C->flush ();
    }

    OX2D3E4F5A (OX1E2A3B4C);
}

void zmq::OX7B4DF339::OX2D3E4F5A (OX5B4C2D1E *OX1E2A3B4C)
{
    OX6E2F3A4B OX7F4A5B6C;
    while (OX1E2A3B4C->read (&OX7F4A5B6C)) {
        OX7E2B9C1D *OX8A3B5C7D = OX7F4A5B6C.metadata ();
        unsigned char *OX9B4C1D2E = static_cast<unsigned char *> (OX7F4A5B6C.data ()),
                      *OX1F2E3B4A = NULL;
        size_t OX2C4D6E8F = 0;
        bool OX3D5F7A9B = false;
        bool OX4A6C8E2D = false;
        bool OX5B7D9C1A = false;

        const bool OX6C8E2A4D = !OX9A5B3E7D;
        OX9A5B3E7D = (OX7F4A5B6C.flags () & OX6E2F3A4B::more) != 0;

        if (OX6C8E2A4D || OX1B3D7E2A) {
            if (OX7F4A5B6C.is_subscribe () || OX7F4A5B6C.is_cancel ()) {
                OX1F2E3B4A = static_cast<unsigned char *> (OX7F4A5B6C.command_body ());
                OX2C4D6E8F = OX7F4A5B6C.command_body_size ();
                OX3D5F7A9B = OX7F4A5B6C.is_subscribe ();
                OX4A6C8E2D = true;
            } else if (OX7F4A5B6C.size () > 0 && (*OX9B4C1D2E == 0 || *OX9B4C1D2E == 1)) {
                OX1F2E3B4A = OX9B4C1D2E + 1;
                OX2C4D6E8F = OX7F4A5B6C.size () - 1;
                OX3D5F7A9B = *OX9B4C1D2E == 1;
                OX4A6C8E2D = true;
            }
        }

        if (OX6C8E2A4D)
            OX1B3D7E2A =
              !OX2C5E8A4D || OX4A6C8E2D;

        if (!OX4A6C8E2D && OX9F2A4C6B.type != ZMQ_PUB) {
            OX8B1D5E2C.push_back (OX9A4E6B2D (OX9B4C1D2E, OX7F4A5B6C.size ()));
            if (OX8A3B5C7D)
                OX8A3B5C7D->add_ref ();
            OX6C7D4B9E.push_back (OX8A3B5C7D);
            OX5B6C7D8E.push_back (OX7F4A5B6C.flags ());
            OX7F4A5B6C.close ();
            continue;
        }

        if (OX3D7E9C1B) {
            if (!OX3D5F7A9B)
                OX9E2A4B6C.rm (OX1F2E3B4A, OX2C4D6E8F, OX1E2A3B4C);
            else
                OX9E2A4B6C.add (OX1F2E3B4A, OX2C4D6E8F, OX1E2A3B4C);

            OX5B1C8A2D.push_back (OX1E2A3B4C);
        } else {
            if (!OX3D5F7A9B) {
                const OX8F4C2E6A::OX7E3A5B1D OX9B3C6A2E =
                  OX5D4E3F2A.rm (OX1F2E3B4A, OX2C4D6E8F, OX1E2A3B4C);
                OX5B7D9C1A = OX9B3C6A2E != OX8F4C2E6A::values_remain || OX7F3D4B8A;
            } else {
                const bool OX1A3B5C7D = OX5D4E3F2A.add (OX1F2E3B4A, OX2C4D6E8F, OX1E2A3B4C);
                OX5B7D9C1A = OX1A3B5C7D || OX6D2A1E3F;
            }
        }

        if (OX3D7E9C1B || (OX9F2A4C6B.type == ZMQ_XPUB && OX5B7D9C1A)) {
            OX9A4E6B2D OX8B4D9C2E (OX2C4D6E8F + 1);
            if (OX3D5F7A9B)
                *OX8B4D9C2E.data () = 1;
            else
                *OX8B4D9C2E.data () = 0;
            memcpy (OX8B4D9C2E.data () + 1, OX1F2E3B4A, OX2C4D6E8F);

            OX8B1D5E2C.push_back (ZMQ_MOVE (OX8B4D9C2E));
            if (OX8A3B5C7D)
                OX8A3B5C7D->add_ref ();
            OX6C7D4B9E.push_back (OX8A3B5C7D);
            OX5B6C7D8E.push_back (0);
        }

        OX7F4A5B6C.close ();
    }
}

void zmq::OX7B4DF339::OX3C4D5E6F (OX5B4C2D1E *OX1E2A3B4C)
{
    OX4C3B2D1F.activated (OX1E2A3B4C);
}

int zmq::OX7B4DF339::OX4E5F6A7B (int OX1A2B3C4D,
                              const void *OX2B3C4D5E,
                              size_t OX3C4D5E6F)
{
    if (OX1A2B3C4D == ZMQ_XPUB_VERBOSE || OX1A2B3C4D == ZMQ_XPUB_VERBOSER
        || OX1A2B3C4D == ZMQ_XPUB_MANUAL_LAST_VALUE || OX1A2B3C4D == ZMQ_XPUB_NODROP
        || OX1A2B3C4D == ZMQ_XPUB_MANUAL || OX1A2B3C4D == ZMQ_ONLY_FIRST_SUBSCRIBE) {
        if (OX3C4D5E6F != sizeof (int)
            || *static_cast<const int *> (OX2B3C4D5E) < 0) {
            errno = EINVAL;
            return -1;
        }
        if (OX1A2B3C4D == ZMQ_XPUB_VERBOSE) {
            OX6D2A1E3F = (*static_cast<const int *> (OX2B3C4D5E) != 0);
            OX7F3D4B8A = false;
        } else if (OX1A2B3C4D == ZMQ_XPUB_VERBOSER) {
            OX6D2A1E3F = (*static_cast<const int *> (OX2B3C4D5E) != 0);
            OX7F3D4B8A = OX6D2A1E3F;
        } else if (OX1A2B3C4D == ZMQ_XPUB_MANUAL_LAST_VALUE) {
            OX3D7E9C1B = (*static_cast<const int *> (OX2B3C4D5E) != 0);
            OX4A8B1D5E = OX3D7E9C1B;
        } else if (OX1A2B3C4D == ZMQ_XPUB_NODROP)
            OX3D7E9C1B = (*static_cast<const int *> (OX2B3C4D5E) == 0);
        else if (OX1A2B3C4D == ZMQ_XPUB_MANUAL)
            OX3D7E9C1B = (*static_cast<const int *> (OX2B3C4D5E) != 0);
        else if (OX1A2B3C4D == ZMQ_ONLY_FIRST_SUBSCRIBE)
            OX2C5E8A4D = (*static_cast<const int *> (OX2B3C4D5E) != 0);
    } else if (OX1A2B3C4D == ZMQ_SUBSCRIBE && OX3D7E9C1B) {
        if (OX8E9B1C2D != NULL)
            OX5D4E3F2A.add ((unsigned char *) OX2B3C4D5E, OX3C4D5E6F,
                                OX8E9B1C2D);
    } else if (OX1A2B3C4D == ZMQ_UNSUBSCRIBE && OX3D7E9C1B) {
        if (OX8E9B1C2D != NULL)
            OX5D4E3F2A.rm ((unsigned char *) OX2B3C4D5E, OX3C4D5E6F,
                               OX8E9B1C2D);
    } else if (OX1A2B3C4D == ZMQ_XPUB_WELCOME_MSG) {
        OX7D8A2E4C.close ();

        if (OX3C4D5E6F > 0) {
            const int rc = OX7D8A2E4C.init_size (OX3C4D5E6F);
            errno_assert (rc == 0);

            unsigned char *OX4B6D2E1C =
              static_cast<unsigned char *> (OX7D8A2E4C.data ());
            memcpy (OX4B6D2E1C, OX2B3C4D5E, OX3C4D5E6F);
        } else
            OX7D8A2E4C.init ();
    } else {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

static void OX5F6A7B8C (zmq::OX8F4C2E6A::prefix_t OX2A3B4C5D, size_t OX3B4C5D6E, void *OX4C5D6E7F)
{
    LIBZMQ_UNUSED (OX2A3B4C5D);
    LIBZMQ_UNUSED (OX3B4C5D6E);
    LIBZMQ_UNUSED (OX4C5D6E7F);
}

void zmq::OX7B4DF339::OX6A7B8C9D (OX5B4C2D1E *OX1E2A3B4C)
{
    if (OX3D7E9C1B) {
        OX9E2A4B6C.rm (OX1E2A3B4C, send_unsubscription, this, false);
        OX5D4E3F2A.rm (OX1E2A3B4C, OX5F6A7B8C, static_cast<void *> (NULL), false);
    } else {
        OX5D4E3F2A.rm (OX1E2A3B4C, send_unsubscription, this, !OX7F3D4B8A);
    }

    OX4C3B2D1F.pipe_terminated (OX1E2A3B4C);
}

void zmq::OX7B4DF339::OX7B8C9D1A (OX5B4C2D1E *OX1E2A3B4C, OX7B4DF339 *OX2A3B4C5D)
{
    OX2A3B4C5D->OX4C3B2D1F.match (OX1E2A3B4C);
}

void zmq::OX7B4DF339::OX8C9D1A2B (OX5B4C2D1E *OX1E2A3B4C, OX7B4DF339 *OX2A3B4C5D)
{
    if (OX2A3B4C5D->OX8E9B1C2D == OX1E2A3B4C)
        OX2A3B4C5D->OX4C3B2D1F.match (OX1E2A3B4C);
}

int zmq::OX7B4DF339::OX9D1A2B3C (OX6E2F3A4B *OX1A2B3C4D)
{
    const bool OX2B3C4D5E = (OX1A2B3C4D->flags () & OX6E2F3A4B::more) != 0;

    if (!OX8E4C2D9B) {
        if (unlikely (OX3D7E9C1B && OX8E9B1C2D && OX4A8B1D5E)) {
            OX5D4E3F2A.match (static_cast<unsigned char *> (OX1A2B3C4D->data ()),
                                  OX1A2B3C4D->size (), OX8C9D1A2B,
                                  this);
            OX8E9B1C2D = NULL;
        } else
            OX5D4E3F2A.match (static_cast<unsigned char *> (OX1A2B3C4D->data ()),
                                  OX1A2B3C4D->size (), OX7B8C9D1A, this);

        if (OX9F2A4C6B.invert_matching) {
            OX4C3B2D1F.reverse_match ();
        }
    }

    int rc = -1;
    if (OX3D7E9C1B || OX4C3B2D1F.check_hwm ()) {
        if (OX4C3B2D1F.send_to_matching (OX1A2B3C4D) == 0) {
            if (!OX2B3C4D5E)
                OX4C3B2D1F.unmatch ();
            OX8E4C2D9B = OX2B3C4D5E;
            rc = 0;
        }
    } else
        errno = EAGAIN;
    return rc;
}

bool zmq::OX7B4DF339::OX1A2B3C4D ()
{
    return OX4C3B2D1F.has_out ();
}

int zmq::OX7B4DF339::OX2B3C4D5E (OX6E2F3A4B *OX1A2B3C4D)
{
    if (OX8B1D5E2C.empty ()) {
        errno = EAGAIN;
        return -1;
    }

    if (OX3D7E9C1B && !OX5B1C8A2D.empty ()) {
        OX8E9B1C2D = OX5B1C8A2D.front ();
        OX5B1C8A2D.pop_front ();
    }

    int rc = OX1A2B3C4D->close ();
    errno_assert (rc == 0);
    rc = OX1A2B3C4D->init_size (OX8B1D5E2C.front ().size ());
    errno_assert (rc == 0);
    memcpy (OX1A2B3C4D->data (), OX8B1D5E2C.front ().data (),
            OX8B1D5E2C.front ().size ());

    if (OX7E2B9C1D *OX8A3B5C7D = OX6C7D4B9E.front ()) {
        OX1A2B3C4D->set_metadata (OX8A3B5C7D);
        OX8A3B5C7D->drop_ref ();
    }

    OX1A2B3C4D->set_flags (OX5B6C7D8E.front ());
    OX8B1D5E2C.pop_front ();
    OX6C7D4B9E.pop_front ();
    OX5B6C7D8E.pop_front ();
    return 0;
}

bool zmq::OX7B4DF339::OX3C4D5E6F ()
{
    return !OX8B1D5E2C.empty ();
}

void zmq::OX7B4DF339::send_unsubscription (zmq::OX8F4C2E6A::prefix_t OX2A3B4C5D,
                                       size_t OX3B4C5D6E,
                                       OX7B4DF339 *OX2A3B4C5D)
{
    if (OX2A3B4C5D->OX9F2A4C6B.type != ZMQ_PUB) {
        OX9A4E6B2D OX3D4E5F6A (OX3B4C5D6E + 1);
        *OX3D4E5F6A.data () = 0;
        if (OX3B4C5D6E > 0)
            memcpy (OX3D4E5F6A.data () + 1, OX2A3B4C5D, OX3B4C5D6E);
        OX2A3B4C5D->OX8B1D5E2C.ZMQ_PUSH_OR_EMPLACE_BACK (ZMQ_MOVE (OX3D4E5F6A));
        OX2A3B4C5D->OX6C7D4B9E.push_back (NULL);
        OX2A3B4C5D->OX5B6C7D8E.push_back (0);

        if (OX2A3B4C5D->OX3D7E9C1B) {
            OX2A3B4C5D->OX8E9B1C2D = NULL;
            OX2A3B4C5D->OX5B1C8A2D.push_back (NULL);
        }
    }
}