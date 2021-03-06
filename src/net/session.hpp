#pragma once

#include "base/time.hpp"
#include "net/connection.hpp"

#include <memory>

namespace net
{

class Session
{
  public:
    //==================
    class Handler
    {
      public:
        //===================
        virtual void onReceive(const base::Bytes& bytes) = 0;
        // virtual void onSend() = 0;
        virtual void onClose() = 0;
        //===================
        virtual ~Handler() = default;
        //===================
    };
    //==================
    explicit Session(std::unique_ptr<Connection> connection);
    ~Session();
    //==================
    bool isActive() const;
    bool isClosed() const;
    //==================
    void setHandler(std::unique_ptr<Handler> handler);
    //==================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //==================
    void start();
    void close();
    //==================
    const Endpoint& getEndpoint() const noexcept;
    const base::Time& getLastSeen() const noexcept;
    //==================
  private:
    //==================
    std::shared_ptr<Connection> _connection;
    std::unique_ptr<Handler> _handler;
    //==================
    base::Time _last_seen;
    //==================
    void receive();
    //==================
};


} // namespace net