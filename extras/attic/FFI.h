
// Nick Porcino, ©2013, Apache 2.0

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace LabRender {

    /*

     Servers are created with names, for example

     OSCServer status("status");

     The server will listen for messages intended for it on a given port.

     status.start(1337);

     Messages for it are expected to have the server name at the start of
     the address.

     /status/address

     By default, servers don't know about any treatment of addresses.
     If a command is registered, the command is expected to follow the server
     name.

     /status/command

     The server will acknowledge a message if the command contains a hint to
     do so.

     /status/command,ack

     If a user writes a command that generates ack data, it will automatically
     be sent back.

     class PingCommand : public LabRender::Command {
     public:
         PingCommand()
         {
             run =
                 [](const LabRender::Command & cmd, const string & path, const std::vector<Argument> &, LabRender::Ack & ack) {
                 ack.payload.push_back(Argument("pong"));
                 cout << "Sending pong" << endl;
             };
         }
         virtual ~PingCommand() {}

         virtual std::string name() const override { return "ping"; }
     };

    status.registerCommand(std::make_shared<PingCommand>());

     Commands have a pretty simple data interface. They can take a few POD types,
     described in the SemanticType enumeration. When sending a command, specify
     the attached data type. The address of the thing to apply the command to
     is expected to follow the command.

     /status/setValue,float/path/to/value (3.0)

     */

    class Ack;

    class Command {
    public:
        enum class SemanticType : unsigned int {
            bool_st = 0,
            int_st, int2_st, int3_st, int4_st,
            float_st, float2_st, float3_st, float4_st,
            string_st,
            unknown_st
        };

        class Argument {
        public:
            SemanticType type = SemanticType::unknown_st;
            union {
                bool boolArg;
                int32_t intArg[4];
                float floatArg[4];
            };
            std::string stringArg;

            Argument(bool b) : type(SemanticType::bool_st) { boolArg = b; }
            Argument(int32_t i0) : type(SemanticType::int_st) { intArg[0] = i0; }
            Argument(int32_t i0, int32_t i1) : type(SemanticType::int2_st) { intArg[0] = i0; intArg[1] = i1; }
            Argument(int32_t i0, int32_t i1, int32_t i2) : type(SemanticType::int3_st) { intArg[0] = i0; intArg[1] = i1; intArg[2] = i2; }
            Argument(int32_t i0, int32_t i1, int32_t i2, int32_t i3) : type(SemanticType::int4_st) { intArg[0] = i0; intArg[1] = i1; intArg[2] = i2; intArg[3] = i3; }
            Argument(float f0) : type(SemanticType::float_st) { floatArg[0] = f0; }
            Argument(float f0, float f1) : type(SemanticType::float2_st) { floatArg[0] = f0; floatArg[1] = f1; }
            Argument(float f0, float f1, float f2) : type(SemanticType::float3_st) { floatArg[0] = f0; floatArg[1] = f1; floatArg[2] = f2; }
            Argument(float f0, float f1, float f2, float f3) : type(SemanticType::float4_st) { floatArg[0] = f0; floatArg[1] = f1; floatArg[2] = f2; floatArg[3] = f3; }
            Argument(const std::string & s) : type(SemanticType::string_st) { stringArg = s; }
            Argument(char const*const s) : type(SemanticType::string_st) { stringArg = s; }

            Argument() { }
            Argument(const Argument & rhs) { *this = rhs; }
            Argument & operator= (const Argument & rhs) {
                type = rhs.type;
                intArg[0] = rhs.intArg[0];
                intArg[1] = rhs.intArg[1];
                intArg[2] = rhs.intArg[2];
                intArg[3] = rhs.intArg[3];
                stringArg = rhs.stringArg;
                return *this;
            }
        };

        Command() { }
        virtual ~Command() {}

        Command(const Command & rhs) { *this = rhs; }
        Command & operator= (const Command & rhs) {
            run = rhs.run;
            return *this;
        }

        virtual std::string name() const = 0;

        bool autoacknowledge = false;

        std::vector<std::pair<std::string, SemanticType>> parameterSpecification;

        // if the command has an ack payload, it is expected
        // to rewrite the argumentType if necessary and also
        // the associated arguments.
        //
        // The run paramters are
        //      the command itself,
        //      the path associated with the command,
        //      the input parameters,
        //      the acknowledge
        //
        // run is a function object so that commands can be constructed easily with
        // lambdas and no need to subclass.
        //
        std::function<void(const Command &,
                           const std::string &,
                           const std::vector<Argument> &,
                           Ack &)
                      > run;
    };

    class Ack {
    public:
        virtual ~Ack() {}
        std::string path;
        std::vector<Command::Argument> payload;
    };

    class OSCServer {
    public:
        class Detail;
        std::shared_ptr<Detail> _detail;

        OSCServer(const std::string & name);
        ~OSCServer();

        void start(const int port);
        void stop();

        void registerCommand(std::shared_ptr<Command> & cmd);
    };

    class WebSocketsServer {
    public:
        class Detail;
        std::shared_ptr<Detail> _detail;

        WebSocketsServer(const std::string & name);
        ~WebSocketsServer();

        void start(const int port);
        void stop();

        void registerCommand(std::shared_ptr<Command> & cmd);
    };

} // LabRender
