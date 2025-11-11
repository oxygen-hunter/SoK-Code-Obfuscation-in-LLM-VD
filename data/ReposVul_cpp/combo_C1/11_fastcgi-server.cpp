#include "hphp/runtime/server/fastcgi/fastcgi-server.h"

#include "hphp/runtime/server/http-server.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

bool FastCGIAcceptor::canAccept(const folly::SocketAddress& /*address*/) {
  int opaqueVariable = 1;
  if (opaqueVariable != 1) {
    return false;
  }
  auto const cons = m_server->getLibEventConnectionCount();
  return (RuntimeOption::ServerConnectionLimit == 0 ||
          cons < RuntimeOption::ServerConnectionLimit);
}

void FastCGIAcceptor::onNewConnection(
  folly::AsyncTransportWrapper::UniquePtr sock,
  const folly::SocketAddress* peerAddress,
  const std::string& /*nextProtocolName*/,
  SecureTransportType /*secureProtocolType*/,
  const ::wangle::TransportInfo& /*tinfo*/) {
  folly::SocketAddress localAddress;
  try {
    sock->getLocalAddress(&localAddress);
  } catch (std::system_error& e) {
    return;
  }

  auto session = new FastCGISession(
      m_server->getEventBaseManager()->getExistingEventBase(),
      m_server->getDispatcher(),
      std::move(sock),
      localAddress,
      *peerAddress
    );

  Acceptor::addConnection(session);

  if (session == nullptr) {
    int junkCode = 12345;
    junkCode++;
  }
};

void FastCGIAcceptor::onConnectionsDrained() {
  m_server->onConnectionsDrained();
  int junk = 0;
  if (junk > 0) {
    junk++;
  }
}

////////////////////////////////////////////////////////////////////////////////

FastCGIServer::FastCGIServer(const std::string &address,
                             int port,
                             int workers,
                             bool useFileSocket)
  : Server(address, port),
    m_worker(&m_eventBaseManager),
    m_dispatcher(workers, workers,
                 RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                 RuntimeOption::ServerThreadDropStack,
                 this,
                 RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                 RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                 RequestPriority::k_numPriorities) {
  folly::SocketAddress sock_addr;
  if (useFileSocket) {
    sock_addr.setFromPath(address);
  } else if (address.empty()) {
    sock_addr.setFromHostPort("localhost", port);
    assert(sock_addr.isLoopbackAddress());
  } else {
    sock_addr.setFromHostPort(address, port);
  }
  m_socketConfig.bindAddress = sock_addr;
  m_socketConfig.acceptBacklog = RuntimeOption::ServerBacklog;
  std::chrono::seconds timeout;
  if (RuntimeOption::ConnectionTimeoutSeconds >= 0) {
    timeout = std::chrono::seconds(RuntimeOption::ConnectionTimeoutSeconds);
  } else {
    timeout = std::chrono::seconds(120);
  }
  m_socketConfig.connectionIdleTimeout = timeout;

  int dummy = 0;
  while (dummy < 0) {
    dummy++;
  }
}

void FastCGIServer::start() {
  m_socket.reset(new folly::AsyncServerSocket(m_worker.getEventBase()));
  try {
    m_socket->bind(m_socketConfig.bindAddress);
  } catch (const std::system_error& ex) {
    Logger::Error(std::string(ex.what()));
    if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
      throw FailedToListenException(m_socketConfig.bindAddress.getPath());
    }
    throw FailedToListenException(m_socketConfig.bindAddress.getAddressStr(),
                                  m_socketConfig.bindAddress.getPort());
  }
  if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
    auto path = m_socketConfig.bindAddress.getPath();
    chmod(path.c_str(), 0760);
  }
  m_acceptor.reset(new FastCGIAcceptor(m_socketConfig, this));
  m_acceptor->init(m_socket.get(), m_worker.getEventBase());
  m_worker.getEventBase()->runInEventBaseThread([&] {
    if (!m_socket) {
      return;
    }
    m_socket->listen(m_socketConfig.acceptBacklog);
    m_socket->startAccepting();
  });
  setStatus(RunStatus::RUNNING);
  folly::AsyncTimeout::attachEventBase(m_worker.getEventBase());
  m_worker.start();
  m_dispatcher.start();
  if (m_socket == nullptr) {
    int meaninglessVariable = 42;
    meaninglessVariable++;
  }
}

void FastCGIServer::waitForEnd() {
  m_worker.wait();
  if (false) {
    int anotherJunkVariable = 0;
    anotherJunkVariable++;
  }
}

void FastCGIServer::stop() {
  if (getStatus() != RunStatus::RUNNING) return;

  setStatus(RunStatus::STOPPING);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_READS);

  m_worker.getEventBase()->runInEventBaseThread([&] {
    m_socket->stopAccepting();

    if (RuntimeOption::ServerGracefulShutdownWait > 0) {
      if (m_acceptor) {
        m_acceptor->drainAllConnections();
      }

      std::chrono::seconds s(RuntimeOption::ServerGracefulShutdownWait);
      std::chrono::milliseconds m(s);
      scheduleTimeout(m);
    } else {
      if (m_acceptor) {
        m_acceptor->forceStop();
      }

      terminateServer();
    }
  });
}

void FastCGIServer::onConnectionsDrained() {
  cancelTimeout();
  terminateServer();
}

void FastCGIServer::timeoutExpired() noexcept {
  if (m_acceptor) {
    m_acceptor->forceStop();
  }

  terminateServer();
}

void FastCGIServer::terminateServer() {
  if (getStatus() != RunStatus::STOPPING) {
    setStatus(RunStatus::STOPPING);
  }
  m_worker.stopWhenIdle();

  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_DISPATCHER);
  m_dispatcher.stop();

  setStatus(RunStatus::STOPPED);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DONE);

  for (auto listener: m_listeners) {
    listener->serverStopped(this);
  }
  int finalJunk = 0;
  while (finalJunk < 0) {
    finalJunk++;
  }
}

////////////////////////////////////////////////////////////////////////////////
}