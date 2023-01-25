# spdr

spdr is a small and efficient networking library. 

## Getting Started

### Building

The best way to get spdr is through [vcpkg](https://vcpkg.io/en/index.html). 
(Soon, this will be done right after the release of 0.2.0.)

You can build spdr through Visual Studio 2022 or CMake, but you need a working
vpckg to pull dependencies.

### A simple example.

In the example folder, you will find the chat example. This wil show you the 
way to implement a simple network protocol. 

The first thing to do is you need to define some basics:

```cpp
constexpr auto CHAT_PROTOCOL_ID = 0xC5A7;
```

Every network protocol should have a unique id. This id helps to ensure when
a connection is established that each network node (client or sever) are 
actually talking the same "language". If you change the anything to your
protocol, you should also change this magic id. 

The next thing is we need to define some messages:

```cpp
enum class ChatMessageId : spdr::MessageId
{
    JOIN,
    SERVER,
    CHAT
};

using JoinMessage   = spdr::Message<ChatMessageId::JOIN, std::string>;
using ServerMessage = spdr::Message<ChatMessageId::SERVER, std::string, std::string>;
using ChatMessage = spdr::Message<ChatMessageId::CHAT, std::string>;
```

We first define an enum that contains all message ids. These are unique ids
per message type and are of type `MessageId`. 

Futher we define the actuall messages types. This done by alisaing a `Message`
with the message id and all payload data types unsing the `using` clause.

To implement the chat server we need to create a network `Node` with the given 
protocol id.

```cpp
auto server = spdr::Node{CHAT_PROTOCOL_ID};
```

Since we need to keep track of the connected users, we will create a small 
structure that mapps the peer id (id of the other node) to a user name:

```cpp
auto users = std::map<spdr::PeerID, std::string>{};
```

For a start, we expect the client to first send a join message:

```cpp
server.on_message<JoinMessage>([&] (auto peer, auto name)
{
    users[peer] = name;
    server.broadcast<ServerMessage>("SERVER", std::format("{} joined", name));
}
```

With some error handling omitted, we save the username with the peer and
tell all other clients that a new user joined the chat. 

Once a client is joined, we expect to recive chat messages. 

```cpp
server.on_message<ChatMessage>([&] (auto peer, auto text)
{
    std::string name = std::get<std::string>(users[peer]);
    server.broadcast<ServerMessage>(name, text);
});
```

Assuming that the client properly joined, we first get the user name of the 
client and then send the text to all clients (including the client that sent
the message).

Of course we need to make sure we register clients that disconnect:

```cpp
server.on_disconnect([&] (unsigned int peer)
{
    auto i = users.find(peer);
    if (i != users.end())
    {
        server.broadcast<ServerMessage>("SERVER", std::format("{} disconnected", name));
        users.erase(i);
    }
});
```

Now that we setup the server, we need to tell it to actually do something:

```cpp
server.listen(2001);
server.run();
```

This will let the server to listen on port 2001 and start processing messages.

If you want ever stop the server you will need to call:

```cpp
server.stop
```

The client is implemented in a similar fassion: 

```cpp
auto client = spdr::Node{CHAT_PROTOCOL_ID};
```

The client only needs to handle server messages:

```cpp
node.on_message<ServerMessage>([this] (auto peer, auto user, auto text)
{
    if (user != username)
    {
        std::cout << std::format("{:<10}: {}\n", user, text);
    }
});
```

In the chat example, input is taken directly from `cin`. In this vain
we need to figure out when the server happens to disconnect, so we can
stop processing input:

```cpp
auto running = std::atomic<bool>(true);
node.on_disconnect([this] (auto)
{
    running = false;
});
```

The boolean is atomic, because we are using the feature that the node
will process messages on a seperate thread. You probably should 
read about threadding below, so you don't break your fancy applications.

In the chat example we get the server's IP address and user name from the
command line arguments.

```cpp
auto address  = std::string{argv[1]};
auto username = std::string{argv[2]};
```

Now that we have all the bits together, we need to connect the client to 
the server and send a join message:

```cpp
auto id = node.connect({address, CHAT_PORT});
node.send<JoinMessage>(id, username);
```

Finally we will read input as it is typed and send it to the server:

```cpp
while (running)
{
    std::string line;
    std::getline(std::cin, line);

    if (!line.empty())
    {
        node.send<ChatMessage>(server, line);
    }
}
```

## Threading

Each `Node` has it's own thread to process network messages. By default this 
thread is used to also process message handlers. There is the option
to move the message handlers onto a different thread and reduce potential 
synchonisation overhead. 

### Using Your Own Thread

spdr uses [c9y](https://github.com/rioki/c9y)'s sync infrastructure to
safly deletage work between threads. If you want to move processing into a 
differen thread, such as for example the thread processing business logic,
you need to specify this thread id on construction of the `Node` and 
periodically call `c9y::sync_point()`. 

For example:

```cpp
auto client = spdr::Node{MY_PROTOCOL_ID, std::this_thread::get_id()};
// some setup on client
client.connect({...});

while (running) 
{
    do_logic();
    do_rendering();
    c9y::sync_point();
}
```

This allows you to not have any additional thread synchonisaton in your 
code, since the message handlers will run in your main thread.

### Run

A similar thing happens when using `Node::run`. The run function will 
also call `sync_point` and use the calling thread to split work over 
two threads and make sure any logic does not slow down the network code.

Although in practice this never happens, but there is a race condition between 
calling `listen` or `connect` and `run`. The first message handler could 
end up on the wrong thread when network input comes in before `run` properly
starts. 

If you intend to use `run` you should consider setting the handling thread 
on the construction of the node:

```cpp
auto client = spdr::Node{MY_PROTOCOL_ID, std::this_thread::get_id()};
// some setup on client
client.connect({...});
client.run();
```

## License

The spdr library is distributed under the MIT license. See [License.txt](License.txt)
for details.
