#include "hphp/runtime/server/fastcgi/fastcgi-server.h"

#include "hphp/runtime/server/http-server.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

bool OX8C1F5E3A::OX5D9D5C43(const folly::SocketAddress& /*address*/) {
  auto const OX2B58F39B = OX2E9F0B6A->OXA20B8A1F();
  return (RuntimeOption::ServerConnectionLimit == 0 ||
          OX2B58F39B < RuntimeOption::ServerConnectionLimit);
}

void OX8C1F5E3A::OX5C4E8F6E(
  folly::AsyncTransportWrapper::UniquePtr OX3C9E7D78,
  const folly::SocketAddress* OX1A0D8A9B,
  const std::string& /*nextProtocolName*/,
  SecureTransportType /*secureProtocolType*/,
  const ::wangle::TransportInfo& /*tinfo*/) {
  folly::SocketAddress OX7B3D2E11;
  try {
    OX3C9E7D78->getLocalAddress(&OX7B3D2E11);
  } catch (std::system_error& OXF69E1C3F) {
    return;
  }

  auto OX1F2C8D7E = new OX4D8F9C6F(
      OX2E9F0B6A->getEventBaseManager()->getExistingEventBase(),
      OX2E9F0B6A->getDispatcher(),
      std::move(OX3C9E7D78),
      OX7B3D2E11,
      *OX1A0D8A9B
    );

  Acceptor::addConnection(OX1F2C8D7E);
};

void OX8C1F5E3A::OX2F9B3D6A() {
  OX2E9F0B6A->OX2F9B3D6A();
}

////////////////////////////////////////////////////////////////////////////////

OX3D0E9F7A::OX3D0E9F7A(const std::string &OX4C9B6F2E,
                             int OX1E2D9C5B,
                             int OX5A3F8B7C,
                             bool OX7D8F1C4E)
  : Server(OX4C9B6F2E, OX1E2D9C5B),
    OXC1F3A8B9(&OX4E5D9F1C),
    OX5F9C8A7D(OX5A3F8B7C, OX5A3F8B7C,
                 RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                 RuntimeOption::ServerThreadDropStack,
                 this,
                 RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                 RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                 RequestPriority::k_numPriorities) {
  folly::SocketAddress OX6E3A7D5B;
  if (OX7D8F1C4E) {
    OX6E3A7D5B.setFromPath(OX4C9B6F2E);
  } else if (OX4C9B6F2E.empty()) {
    OX6E3A7D5B.setFromHostPort("localhost", OX1E2D9C5B);
    assert(OX6E3A7D5B.isLoopbackAddress());
  } else {
    OX6E3A7D5B.setFromHostPort(OX4C9B6F2E, OX1E2D9C5B);
  }
  OX4F7E1D3C.bindAddress = OX6E3A7D5B;
  OX4F7E1D3C.acceptBacklog = RuntimeOption::ServerBacklog;
  std::chrono::seconds OX3F9E2D7A;
  if (RuntimeOption::ConnectionTimeoutSeconds >= 0) {
    OX3F9E2D7A = std::chrono::seconds(RuntimeOption::ConnectionTimeoutSeconds);
  } else {
    OX3F9E2D7A = std::chrono::seconds(120);
  }
  OX4F7E1D3C.connectionIdleTimeout = OX3F9E2D7A;
}

void OX3D0E9F7A::OX4C2A9E5F() {
  OX3A7F8D9E.reset(new folly::AsyncServerSocket(OXC1F3A8B9.getEventBase()));
  try {
    OX3A7F8D9E->bind(OX4F7E1D3C.bindAddress);
  } catch (const std::system_error& OXF69E1C3F) {
    Logger::Error(std::string(OXF69E1C3F.what()));
    if (OX4F7E1D3C.bindAddress.getFamily() == AF_UNIX) {
      throw OX5F2C7A8D(OX4F7E1D3C.bindAddress.getPath());
    }
    throw OX5F2C7A8D(OX4F7E1D3C.bindAddress.getAddressStr(),
                                  OX4F7E1D3C.bindAddress.getPort());
  }
  if (OX4F7E1D3C.bindAddress.getFamily() == AF_UNIX) {
    auto OX6E4C9B5A = OX4F7E1D3C.bindAddress.getPath();
    chmod(OX6E4C9B5A.c_str(), 0760);
  }
  OX1C7D8A3F.reset(new OX8C1F5E3A(OX4F7E1D3C, this));
  OX1C7D8A3F->init(OX3A7F8D9E.get(), OXC1F3A8B9.getEventBase());
  OXC1F3A8B9.getEventBase()->runInEventBaseThread([&] {
    if (!OX3A7F8D9E) {
      return;
    }
    OX3A7F8D9E->listen(OX4F7E1D3C.acceptBacklog);
    OX3A7F8D9E->startAccepting();
  });
  setStatus(RunStatus::RUNNING);
  folly::AsyncTimeout::attachEventBase(OXC1F3A8B9.getEventBase());
  OXC1F3A8B9.start();
  OX5F9C8A7D.start();
}

void OX3D0E9F7A::OX5D8E1C7A() {
  OXC1F3A8B9.wait();
}

void OX3D0E9F7A::OX5F7A8C9E() {
  if (getStatus() != RunStatus::RUNNING) return;

  setStatus(RunStatus::STOPPING);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_READS);

  OXC1F3A8B9.getEventBase()->runInEventBaseThread([&] {
    OX3A7F8D9E->stopAccepting();

    if (RuntimeOption::ServerGracefulShutdownWait > 0) {
      if (OX1C7D8A3F) {
        OX1C7D8A3F->drainAllConnections();
      }

      std::chrono::seconds OX3F9E2D7A(RuntimeOption::ServerGracefulShutdownWait);
      std::chrono::milliseconds OX7A1D3E5F(OX3F9E2D7A);
      scheduleTimeout(OX7A1D3E5F);
    } else {
      if (OX1C7D8A3F) {
        OX1C7D8A3F->forceStop();
      }

      OX6E8C9D2A();
    }
  });
}

void OX3D0E9F7A::OX2F9B3D6A() {
  cancelTimeout();
  OX6E8C9D2A();
}

void OX3D0E9F7A::OX7A6B2C8F() noexcept {
  if (OX1C7D8A3F) {
    OX1C7D8A3F->forceStop();
  }

  OX6E8C9D2A();
}

void OX3D0E9F7A::OX6E8C9D2A() {
  if (getStatus() != RunStatus::STOPPING) {
    setStatus(RunStatus::STOPPING);
  }
  OXC1F3A8B9.stopWhenIdle();

  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_DISPATCHER);
  OX5F9C8A7D.stop();

  setStatus(RunStatus::STOPPED);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DONE);

  for (auto OX3C9E7D78: OX2C8F9D6E) {
    OX3C9E7D78->serverStopped(this);
  }
}

////////////////////////////////////////////////////////////////////////////////
}