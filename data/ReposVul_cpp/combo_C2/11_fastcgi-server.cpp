#include "hphp/runtime/server/fastcgi/fastcgi-server.h"
#include "hphp/runtime/server/http-server.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

bool FastCGIAcceptor::canAccept(const folly::SocketAddress& /*address*/) {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        auto const cons = m_server->getLibEventConnectionCount();
        dispatch = 1;
        continue;
      }
      case 1:
        return (RuntimeOption::ServerConnectionLimit == 0 ||
                m_server->getLibEventConnectionCount() < RuntimeOption::ServerConnectionLimit);
    }
  }
}

void FastCGIAcceptor::onNewConnection(
  folly::AsyncTransportWrapper::UniquePtr sock,
  const folly::SocketAddress* peerAddress,
  const std::string& /*nextProtocolName*/,
  SecureTransportType /*secureProtocolType*/,
  const ::wangle::TransportInfo& /*tinfo*/) {
  int dispatch = 0;
  folly::SocketAddress localAddress;
  while (true) {
    switch (dispatch) {
      case 0:
        try {
          sock->getLocalAddress(&localAddress);
          dispatch = 2;
        } catch (std::system_error& e) {
          return;
        }
        continue;
      case 2: {
        auto session = new FastCGISession(
          m_server->getEventBaseManager()->getExistingEventBase(),
          m_server->getDispatcher(),
          std::move(sock),
          localAddress,
          *peerAddress
        );
        dispatch = 3;
        continue;
      }
      case 3:
        Acceptor::addConnection(session);
        return;
    }
  }
};

void FastCGIAcceptor::onConnectionsDrained() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        m_server->onConnectionsDrained();
        return;
    }
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
  int dispatch = 0;
  folly::SocketAddress sock_addr;
  while (true) {
    switch (dispatch) {
      case 0:
        if (useFileSocket) {
          sock_addr.setFromPath(address);
          dispatch = 3;
        } else if (address.empty()) {
          sock_addr.setFromHostPort("localhost", port);
          assert(sock_addr.isLoopbackAddress());
          dispatch = 3;
        } else {
          sock_addr.setFromHostPort(address, port);
          dispatch = 3;
        }
        continue;
      case 3:
        m_socketConfig.bindAddress = sock_addr;
        m_socketConfig.acceptBacklog = RuntimeOption::ServerBacklog;
        dispatch = 4;
        continue;
      case 4:
        if (RuntimeOption::ConnectionTimeoutSeconds >= 0) {
          std::chrono::seconds timeout = std::chrono::seconds(RuntimeOption::ConnectionTimeoutSeconds);
          m_socketConfig.connectionIdleTimeout = timeout;
        } else {
          std::chrono::seconds timeout = std::chrono::seconds(120);
          m_socketConfig.connectionIdleTimeout = timeout;
        }
        return;
    }
  }
}

void FastCGIServer::start() {
  int dispatch = 0;
  m_socket.reset(new folly::AsyncServerSocket(m_worker.getEventBase()));
  while (true) {
    switch (dispatch) {
      case 0:
        try {
          m_socket->bind(m_socketConfig.bindAddress);
          dispatch = 2;
        } catch (const std::system_error& ex) {
          Logger::Error(std::string(ex.what()));
          if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
            throw FailedToListenException(m_socketConfig.bindAddress.getPath());
          }
          throw FailedToListenException(m_socketConfig.bindAddress.getAddressStr(),
                                        m_socketConfig.bindAddress.getPort());
        }
        continue;
      case 2:
        if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
          auto path = m_socketConfig.bindAddress.getPath();
          chmod(path.c_str(), 0760);
        }
        m_acceptor.reset(new FastCGIAcceptor(m_socketConfig, this));
        m_acceptor->init(m_socket.get(), m_worker.getEventBase());
        dispatch = 3;
        continue;
      case 3:
        m_worker.getEventBase()->runInEventBaseThread([&] {
          if (!m_socket) {
            return;
          }
          m_socket->listen(m_socketConfig.acceptBacklog);
          m_socket->startAccepting();
        });
        dispatch = 4;
        continue;
      case 4:
        setStatus(RunStatus::RUNNING);
        folly::AsyncTimeout::attachEventBase(m_worker.getEventBase());
        m_worker.start();
        m_dispatcher.start();
        return;
    }
  }
}

void FastCGIServer::waitForEnd() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        m_worker.wait();
        return;
    }
  }
}

void FastCGIServer::stop() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        if (getStatus() != RunStatus::RUNNING) return;
        setStatus(RunStatus::STOPPING);
        HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_READS);
        dispatch = 1;
        continue;
      case 1:
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
        return;
    }
  }
}

void FastCGIServer::onConnectionsDrained() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        cancelTimeout();
        terminateServer();
        return;
    }
  }
}

void FastCGIServer::timeoutExpired() noexcept {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        if (m_acceptor) {
          m_acceptor->forceStop();
        }
        terminateServer();
        return;
    }
  }
}

void FastCGIServer::terminateServer() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0:
        if (getStatus() != RunStatus::STOPPING) {
          setStatus(RunStatus::STOPPING);
        }
        m_worker.stopWhenIdle();
        HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_DISPATCHER);
        m_dispatcher.stop();
        setStatus(RunStatus::STOPPED);
        HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DONE);
        dispatch = 1;
        continue;
      case 1:
        for (auto listener: m_listeners) {
          listener->serverStopped(this);
        }
        return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}