
// Nick Porcino, ©2013, Apache 2.0

#if defined(_WINDOWS)
# define _CRT_SECURE_NO_WARNINGS
#endif

#include "FFI.h"
#include "oscpkt/oscpkt.hh"

#include <iostream>
#include <map>
#include <mutex>
#include <thread>

#define OSCPKT_OSTREAM_OUTPUT
#include "oscpkt/oscpkt.hh"
#include "oscpkt/udp.hh"

#include "webby/webby.h"

#include "json/json.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

using std::cout;
using std::cerr;

using namespace oscpkt;

/*

/labrender/set,float,ack/path/to/entity (value) -> /ack/path/to/entity ()
/labrender/set,float/path/to/entity (value)

/labrender/control/path/to/entity (sexpr) -> /control/path/to/entity (ack)
/labrender/value/path/to/entity () -> /value/path/to/entity (value)
/labrender/state/path/to/entity () -> /state/path/to/entity (state)

*/

namespace LabRender {

    using namespace std;

    // eg: "{ \"jsonrpc\": \"2.0\", \"method\":\"subtract\", \"params\": [42, 23], \"id\": 1}";

    void parseJsonRpc(const std::string & rpc, std::string & method, std::vector<Command::Argument> & arguments,
                      int & intId, string & stringId)
    {
        Json::Value conf;

        stringstream ss;
        ss << rpc;
        ss >> conf;

        bool isJsonRpc = conf["jsonrpc"].asString() == "2.0";
        if (isJsonRpc) {
            if (conf["method"] != Json::nullValue && conf["id"] != Json::nullValue) {
                method = conf["method"].asString();
                Json::Value id = conf["id"];
                if (id.type() == Json::ValueType::intValue) {
                    intId = id.asInt();
                }
                else if (id.type() == Json::ValueType::stringValue) {
                    stringId = id.asString();
                }
                else {
                    //// raise an error
                }

                Json::Value params = conf["params"];
                if (params != Json::nullValue) {
                    if (params.type() == Json::ValueType::arrayValue) {
                        for (Json::Value::iterator it = params.begin(); it != params.end(); ++it) {
                            // positional parameters
                            switch ((*it).type()) {
                                case Json::ValueType::booleanValue:
                                    arguments.emplace_back(Command::Argument((*it).asBool()));
                                    break;

                                case Json::ValueType::intValue:
                                    arguments.emplace_back(Command::Argument((*it).asInt()));
                                    break;

                                case Json::ValueType::realValue:
                                    arguments.emplace_back(Command::Argument((*it).asFloat()));
                                    break;

                                case Json::ValueType::stringValue:
                                    arguments.emplace_back(Command::Argument((*it).asString()));
                                    break;

                                case Json::ValueType::uintValue:
                                    arguments.emplace_back(Command::Argument((*it).asInt()));
                                    break;


                                case Json::ValueType::arrayValue:
                                case Json::ValueType::nullValue:
                                case Json::ValueType::objectValue:
                                    break;
                            }
                        }
                    }
                    else if (params.type() == Json::ValueType::objectValue) {
                        for (Json::Value::iterator it = params.begin(); it != params.end(); ++it) {
                            // named parameters
                        }
                    }
                    else {
                        //// raise an error
                    }
                }
            }
        }
    }

    string createJsonRpcAck(int intId, const string & stringId, Ack & ack)
    {
        // create result object from ack
        string result = "\"todo\"";
        stringstream ss;
        if (intId >= 0) {
            ss << "{\"jsonrpc\":\"2.0\",\"result\":" << result << " \"id\":" << intId << "}";
        }
        else {
            ss << "{\"jsonrpc\":\"2.0\",\"result\":" << result << " \"id\":" << stringId << "}";
        }
        return ss.str();
    }


    class OSCServer::Detail {
    public:
        Detail(const std::string & name)
        : _name(name) {}

        string _name;
        shared_ptr<thread> _thread;
        bool _stop = false;

        mutex _functionsLock;
        map<string, shared_ptr<Command>> _functions;

        std::string name() const { return _name; }
    };

    void runOSCServer(int PORT_NUM, OSCServer::Detail * server)
    {
        UdpSocket sock;
        sock.bindTo(PORT_NUM);
        if (!sock.isOk())
        {
            cerr << "Error opening port " << PORT_NUM << ": " << sock.errorMessage() << "\n";
        }
        else
        {
            string serverName = "/" + server->name() + "/";

            cout << "OSC Server started on port " << PORT_NUM << std::endl;
            PacketReader pr;
            PacketWriter pw;
            while (sock.isOk() && !server->_stop)
            {
                if (sock.receiveNextPacket(30 /* timeout, in ms */))
                {
                    pr.init(sock.packetData(), sock.packetSize());
                    oscpkt::Message *msg;
                    while (pr.isOk() && (msg = pr.popMessage()) != 0)
                    {
                        oscpkt::Message::ArgReader arg = msg->partialMatch(serverName);
                        if (!arg) {
                            cout << "Unhandled message " << msg->addressPattern() << std::endl;
                            continue;
                        }

                        string addr = msg->addressPattern().substr(strlen("/labrender/"));
                        size_t pos = addr.find('/');
                        string path;
                        string command;

                        if (pos == string::npos)
                            command = addr;
                        else {
                            path = addr.substr(pos+1);
                            command = addr.substr(0, pos);
                        }

                        size_t modifiers = command.find(',');
                        string baseCommand = modifiers != string::npos? command.substr(0, modifiers) : command;

                        std::vector<Command::Argument> arguments;

                        bool isJsonRpc = baseCommand == "jsonRpc";
                        int jsonRpcId = -1;
                        string jsonRpcIdStr;

                        if (isJsonRpc) {
                            string rpc;
                            arg.popStr(rpc);
                            parseJsonRpc(rpc, baseCommand, arguments, jsonRpcId, jsonRpcIdStr);
                        }

                        shared_ptr<Command> cmdPtr;
                        {
                            unique_lock<mutex> lockFunctions(server->_functionsLock);
                            auto cmdIter = server->_functions.find(baseCommand);
                            if (cmdIter == server->_functions.end()) {
                                cout << "Unhandled message " << msg->addressPattern() << " because unknown command " << baseCommand << endl;
                                continue;
                            }

                            cmdPtr = cmdIter->second;
                        }
                        Command * cmd = cmdPtr.get();
                        bool mustAck = command.find(",ack") || cmd->autoacknowledge;

                        for (auto i = cmd->parameterSpecification.rbegin(); i != cmd->parameterSpecification.rend(); ++i) {
                            switch (i->second) {
                                case Command::SemanticType::bool_st: {
                                    bool b;
                                    arg.popBool(b);
                                    arguments.emplace_back(Command::Argument(b));
                                    break;
                                }
                                case Command::SemanticType::int4_st: {
                                    int32_t v0, v1, v2, v3;
                                    arg.popInt32(v3); arg.popInt32(v2); arg.popInt32(v1); arg.popInt32(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1, v2, v3));
                                    break;
                                }
                                case Command::SemanticType::int3_st: {
                                    int32_t v0, v1, v2;
                                    arg.popInt32(v2); arg.popInt32(v1); arg.popInt32(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1, v2));
                                    break;
                                }
                                case Command::SemanticType::int2_st: {
                                    int32_t v0, v1;
                                    arg.popInt32(v1); arg.popInt32(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1));
                                    break;
                                }
                                case Command::SemanticType::int_st: {
                                    int32_t v0;
                                    arg.popInt32(v0);
                                    arguments.emplace_back(Command::Argument(v0));
                                    break;
                                }
                                case Command::SemanticType::float4_st: {
                                    float v0, v1, v2, v3;
                                    arg.popFloat(v3); arg.popFloat(v2); arg.popFloat(v1); arg.popFloat(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1, v2, v3));
                                    break;
                                }
                                case Command::SemanticType::float3_st: {
                                    float v0, v1, v2;
                                    arg.popFloat(v2); arg.popFloat(v1); arg.popFloat(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1, v2));
                                    break;
                                }
                                case Command::SemanticType::float2_st: {
                                    float v0, v1;
                                    arg.popFloat(v1); arg.popFloat(v0);
                                    arguments.emplace_back(Command::Argument(v0, v1));
                                    break;
                                }
                                case Command::SemanticType::float_st: {
                                    float v0;
                                    arg.popFloat(v0);
                                    arguments.emplace_back(Command::Argument(v0));
                                    break;
                                }
                                case Command::SemanticType::string_st: {
                                    string s;
                                    arg.popStr(s);
                                    arguments.emplace_back(Command::Argument(s));
                                    break;
                                }
                                default:
                                    break;
                            }
                        }

                        Ack ack;
                        cmd->run(*cmd, path, arguments, ack);

                        if (isJsonRpc)
                        {
                            string jsonResponse = createJsonRpcAck(jsonRpcId, jsonRpcIdStr, ack);
                            Message repl;
                            repl.init(serverName + "ack/jsonRpc");
                            repl.pushStr(jsonResponse);
                            pw.init().addMessage(repl);
                            sock.sendPacketTo(pw.packetData(), pw.packetSize(), sock.packetOrigin());
                        }

                        if (mustAck || ack.payload.size() > 0)
                        {
                            Message repl;
                            repl.init(serverName + "ack/" + baseCommand);

                            for (auto & i : ack.payload)
                            {
                                switch (i.type) {
                                    case Command::SemanticType::bool_st:
                                        repl.pushBool(i.boolArg);
                                        break;
                                    case Command::SemanticType::int4_st:
                                        repl.pushInt32(i.intArg[0]);
                                        repl.pushInt32(i.intArg[1]);
                                        repl.pushInt32(i.intArg[2]);
                                        repl.pushInt32(i.intArg[3]);
                                        break;
                                    case Command::SemanticType::int3_st:
                                        repl.pushInt32(i.intArg[0]);
                                        repl.pushInt32(i.intArg[1]);
                                        repl.pushInt32(i.intArg[2]);
                                        break;
                                    case Command::SemanticType::int2_st:
                                        repl.pushInt32(i.intArg[0]);
                                        repl.pushInt32(i.intArg[1]);
                                        break;
                                    case Command::SemanticType::int_st:
                                        repl.pushInt32(i.intArg[0]);
                                        break;
                                    case Command::SemanticType::float4_st:
                                        repl.pushFloat(i.floatArg[0]);
                                        repl.pushFloat(i.floatArg[1]);
                                        repl.pushFloat(i.floatArg[2]);
                                        repl.pushFloat(i.floatArg[3]);
                                        break;
                                    case Command::SemanticType::float3_st:
                                        repl.pushFloat(i.floatArg[0]);
                                        repl.pushFloat(i.floatArg[1]);
                                        repl.pushFloat(i.floatArg[2]);
                                        break;
                                    case Command::SemanticType::float2_st:
                                        repl.pushFloat(i.floatArg[0]);
                                        repl.pushFloat(i.floatArg[1]);
                                        break;
                                    case Command::SemanticType::float_st:
                                        repl.pushFloat(i.floatArg[0]);
                                        break;
                                    case Command::SemanticType::string_st:
                                        repl.pushStr(i.stringArg);
                                        break;
                                    default:
                                        break;
                                }
                            }

                            pw.init().addMessage(repl);
                            sock.sendPacketTo(pw.packetData(), pw.packetSize(), sock.packetOrigin());
                        }
                    }
                }
            }
        }
    }

    OSCServer::OSCServer(const std::string & name)
    : _detail(std::make_shared<Detail>(name)) { }

    OSCServer::~OSCServer()
    {
        _detail->_stop = true;
        if (!!_detail->_thread)
            _detail->_thread->join();  // block until done
    }

    void OSCServer::start(const int port)
    {
        stop();
        _detail->_stop = false;
        _detail->_thread = std::make_shared<std::thread>([this, port]() {
            runOSCServer(port, this->_detail.get());
        });
    }

    void OSCServer::stop()
    {
        _detail->_stop = true;
        if (!!_detail->_thread)
            _detail->_thread->join();
        _detail->_thread.reset();
    }

    void OSCServer::registerCommand(shared_ptr<Command> & cmd)
    {
        unique_lock<mutex> lockFunctions(_detail->_functionsLock);
        _detail->_functions[cmd->name()] = cmd;
    }






    class WebSocketsServer::Detail {
    public:
        Detail(const std::string & name)
        : name(name) { }

        static std::mutex creationMutex;
        static WebSocketsServer::Detail * singleton;

        std::string name;
        map<string, shared_ptr<Command>> functions;
        shared_ptr<thread> serverThread;
        bool stop = false;

        int ws_connection_count = 0;
        enum { maxConnections = 8 };
        struct WebbyConnection *ws_connections[maxConnections];

        static void runServer(const int port, Detail * detail) {
            #if defined(_WIN32)
              {
                WORD wsa_version = MAKEWORD(2, 2);
                WSADATA wsa_data;
                if (0 != WSAStartup(wsa_version, &wsa_data))
                {
                    fprintf(stderr, "WSAStartup failed\n");
                    return;
                }
              }
            #endif

            WebbyServerConfig config;
            memset(&config, 0, sizeof config);
            config.bind_address = "127.0.0.1";
            config.listening_port = port;
            config.flags = WEBBY_SERVER_WEBSOCKETS | WEBBY_SERVER_LOG_DEBUG;
            config.connection_max = 4;
            config.request_buffer_size = 2048;
            config.io_buffer_size = 8192;
            config.dispatch = &test_dispatch;
            config.log = &test_log;
            config.ws_connect = &test_ws_connect;
            config.ws_connected = &test_ws_connected;
            config.ws_closed = &test_ws_closed;
            config.ws_frame = &test_ws_frame;

            size_t memory_size = WebbyServerMemoryNeeded(&config);
            void * memory = malloc(memory_size);
            WebbyServer * server = WebbyServerInit(&config, memory, memory_size);

            if (!server)
            {
                fprintf(stderr, "failed to init server\n");
                #if defined(_WIN32)
                  WSACleanup(); // re-entrant - WSAStartup can be called many times
                                // and must match the number of clean up calls
                #endif
                return;
            }

            cout << "WebSockets Server started on port " << port << std::endl;

            while (!detail->stop) {
                WebbyServerUpdate(server);

                // should actually be semaphored wait for incoming message

                for (int i = 0; i < detail->ws_connection_count; ++i) {
                    WebbyBeginSocketFrame(detail->ws_connections[i], WEBBY_WS_OP_TEXT_FRAME);
                    WebbyPrintf(detail->ws_connections[i], "Hello world over websockets!\n");
                    WebbyEndSocketFrame(detail->ws_connections[i]);
                    #if defined(_WIN32)
                        Sleep(30);
                    #else
                        usleep(30 * 1000);
                    #endif
                }
            }

            #if defined(_WIN32)
              WSACleanup(); // re-entrant - WSAStartup can be called many times
                            // and must match the number of clean up calls
            #endif
        }

        static int test_dispatch(struct WebbyConnection *connection)
        {
            if (0 == strcmp("/foo", connection->request.uri))
            {
                WebbyBeginResponse(connection, 200, 14, NULL, 0);
                WebbyWrite(connection, "Hello, world!\n", 14);
                WebbyEndResponse(connection);
                return 0;
            }
            else if (0 == strcmp("/bar", connection->request.uri))
            {
                WebbyBeginResponse(connection, 200, -1, NULL, 0);
                WebbyWrite(connection, "Hello, world!\n", 14);
                WebbyWrite(connection, "Hello, world?\n", 14);
                WebbyEndResponse(connection);
                return 0;
            }
            else if (0 == strcmp(connection->request.uri, "/quit"))
            {
                WebbyBeginResponse(connection, 200, -1, NULL, 0);
                WebbyPrintf(connection, "Goodbye, cruel world\n");
                WebbyEndResponse(connection);
                singleton->stop = true;
                return 0;
            }
            else
                return 1;
        }

        static void test_log(const char* text)
        {
            printf("[ws: debug] %s\n", text);
        }

        static int test_ws_connect(struct WebbyConnection *connection)
        {
            //printf("connection request %s\n", connection->request.uri);
            return 0; // succcess
            /* Allow websocket upgrades on /wstest */
            if (0 == strcmp(connection->request.uri, "/wstest") && singleton->ws_connection_count < maxConnections)
                return 0;
            else
                return 1;
        }


        static void test_ws_closed(struct WebbyConnection *connection)
        {
            int i;
            printf("WebSocket closed\n");

            for (i = 0; i < singleton->ws_connection_count; i++)
            {
                if (singleton->ws_connections[i] == connection)
                {
                    int remain = singleton->ws_connection_count - i;
                    memmove(singleton->ws_connections + i, singleton->ws_connections + i + 1, remain * sizeof(struct WebbyConnection *));
                    --singleton->ws_connection_count;
                    break;
                }
            }
        }

        static int test_ws_frame(struct WebbyConnection *connection, const struct WebbyWsFrame *frame)
        {
            int i = 0;

            printf("WebSocket frame incoming\n");
            printf("  Frame OpCode: %d\n", frame->opcode);
            printf("  Final frame?: %s\n", (frame->flags & WEBBY_WSF_FIN) ? "yes" : "no");
            printf("  Masked?     : %s\n", (frame->flags & WEBBY_WSF_MASKED) ? "yes" : "no");
            printf("  Data Length : %d\n", (int) frame->payload_length);

            while (i < frame->payload_length)
            {
                unsigned char buffer[16];
                int remain = frame->payload_length - i;
                size_t read_size = remain > (int) sizeof buffer ? sizeof buffer : (size_t) remain;
                size_t k;

                printf("%08x ", (int) i);

                if (0 != WebbyRead(connection, buffer, read_size))
                    break;

                for (k = 0; k < read_size; ++k)
                    printf("%02x ", buffer[k]);

                for (k = read_size; k < 16; ++k)
                    printf("   ");

                printf(" | ");

                for (k = 0; k < read_size; ++k)
                    printf("%c", isprint(buffer[k]) ? buffer[k] : '?');

                printf("\n");

                i += (int) read_size;
            }

            return 0;
        }


        static void test_ws_connected(struct WebbyConnection *connection)
        {
            printf("WebSocket connected\n");
            singleton->ws_connections[singleton->ws_connection_count++] = connection;
        }

    };

    WebSocketsServer::Detail * WebSocketsServer::Detail::singleton = nullptr;
    std::mutex WebSocketsServer::Detail::creationMutex;

    WebSocketsServer::WebSocketsServer(const std::string & name)
    {
        std::unique_lock<std::mutex> creationLock(WebSocketsServer::Detail::creationMutex);
        if (!WebSocketsServer::Detail::singleton) {
            _detail = std::make_shared<Detail>(name);
            WebSocketsServer::Detail::singleton = _detail.get();

        }
        else {
            WebSocketsServer::Detail::singleton = nullptr;
        }
    }

    WebSocketsServer::~WebSocketsServer()
    {
        if (_detail && !!_detail->serverThread) {
            _detail->serverThread->join(); // block until done
            WebSocketsServer::Detail::singleton = nullptr;
        }
    }

    void WebSocketsServer::start(const int port)
    {
        stop();
        if (_detail) {
            _detail->stop = false;
            _detail->serverThread = std::make_shared<std::thread>([this, port]() {
                WebSocketsServer::Detail::runServer(port, this->_detail.get());
            });
        }
    }

    void WebSocketsServer::stop()
    {
        if (_detail) {
            _detail->stop = true;
            if (!!_detail->serverThread)
                _detail->serverThread->join();
            _detail->serverThread.reset();
        }
    }

    void WebSocketsServer::registerCommand(std::shared_ptr<Command> & cmd)
    {
        if (_detail) {
            unique_lock<mutex> lockFunctions(WebSocketsServer::Detail::creationMutex);
            _detail->functions[cmd->name()] = cmd;
        }
    }

} // LabRender
