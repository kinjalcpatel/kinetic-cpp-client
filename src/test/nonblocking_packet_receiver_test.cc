/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 * 
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without 
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public 
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

#include <arpa/inet.h>
#include <unistd.h>

#include "gmock/gmock.h"

#include "kinetic/kinetic.h"
#include "nonblocking_packet_service.h"
#include "mock_socket_wrapper_interface.h"
#include "mock_nonblocking_packet_service.h"
#include "matchers.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


namespace kinetic {

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using com::seagate::kinetic::client::proto::Command_MessageType_GET_RESPONSE;
using com::seagate::kinetic::client::proto::Command_Status_StatusCode_SUCCESS;
using com::seagate::kinetic::client::proto::Message_AuthType_HMACAUTH;
using com::seagate::kinetic::client::proto::Message_AuthType_UNSOLICITEDSTATUS;

using std::string;
using std::make_shared;

class NonblockingReceiverTest : public ::testing::Test {
    protected:
    // Create a pipe that we can use to feed data to the NonblockingReceiver
    void SetUp() {
        ASSERT_EQ(0, pipe(fds_));
        ASSERT_EQ(0, fcntl(fds_[0], F_SETFL, O_NONBLOCK));
        ASSERT_EQ(0, fcntl(fds_[1], F_SETFL, O_NONBLOCK));

        Message handshake;
        Command command;
        handshake.set_authtype(Message_AuthType_UNSOLICITEDSTATUS);
        command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
        WritePacket(handshake, command, "");
    }

    void TearDown() {
        ASSERT_EQ(0, close(fds_[0]));
        ASSERT_EQ(0, close(fds_[1]));
    }

    // Write a packet into the pipe
    void WritePacket(Message &message, const Command &command, const std::string &value) {
        message.set_commandbytes(command.SerializeAsString());
        if(!message.has_authtype()){
            message.set_authtype(Message_AuthType_HMACAUTH);
            message.mutable_hmacauth()->set_identity(3);
            message.mutable_hmacauth()->set_hmac(hmac_provider_.ComputeHmac(message, "key"));
        }

        std::string serialized_message;
        ASSERT_TRUE(message.SerializeToString(&serialized_message));
        ASSERT_EQ(1, write(fds_[1], "F", 1));
        uint32_t message_length = htonl(serialized_message.size());
        ASSERT_EQ(4, write(fds_[1], reinterpret_cast<char *>(&message_length), 4));
        uint32_t value_length = htonl(value.size());
        ASSERT_EQ(4, write(fds_[1], reinterpret_cast<char *>(&value_length), 4));
        ASSERT_EQ(static_cast<ssize_t>(serialized_message.size()), write(fds_[1],
            serialized_message.data(), serialized_message.size()));
        ASSERT_EQ(static_cast<ssize_t>(value.size()), write(fds_[1], value.data(), value.size()));
    }

    int fds_[2];
    HmacProvider hmac_provider_;

    void defaultReceiverSetup(Command &command,
            shared_ptr<MockSocketWrapperInterface> socket_wrapper, ConnectionOptions &options) {
        command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
        command.mutable_header()->set_acksequence(33);

        Message message;
        WritePacket(message, command, "value");
        EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
        EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
        options.user_id = 3;
        options.hmac_key = "key";
    }
};

TEST_F(NonblockingReceiverTest, SimpleMessageAndValue) {
    Command command;
    Message message;
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    command.mutable_header()->set_acksequence(33);
    WritePacket(message, command, "value");

    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));

    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ReceiveResponsesOutOfOrder) {
    Command command;
    Message message;
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    command.mutable_header()->set_acksequence(44);
    WritePacket(message, command, "value2");

    message.Clear();
    command.mutable_header()->set_acksequence(33);
    WritePacket(message, command, "value");

    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    EXPECT_CALL(*handler2, Handle_(_, "value2"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 44, 1));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, CallsErrorWhenNoAckSequence) {
    Command command;
    Message message;
    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    WritePacket(message, command, "value");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::PROTOCOL_ERROR_RESPONSE_NO_ACKSEQUENCE,
        "Response had no acksequence"), NULL));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, SetsConnectionId) {
    // The receiver should adjust its connection ID to whatever the server
    // decides it should be.
    Command command;
    Message message;
    command.mutable_header()->set_connectionid(42);
    WritePacket(message, command, "");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<NiceMock<MockHandler>>();
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    receiver.Receive();
    ASSERT_EQ(42, receiver.connection_id());
}

TEST_F(NonblockingReceiverTest, HandlesReadError) {
    const char header[] = { 'E' };  // invalid magic character
    ASSERT_EQ(static_cast<ssize_t>(sizeof(header)), write(fds_[1], header, sizeof(header)));
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(
            KineticStatusEq(StatusCode::CLIENT_IO_ERROR, "I/O read error"), NULL));
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, HandlesHmacError) {
    Message message;
    Command command;
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";

    message.set_commandbytes(command.SerializeAsString());
    message.set_authtype(Message_AuthType_HMACAUTH);
    message.mutable_hmacauth()->set_identity(options.user_id);
    message.mutable_hmacauth()->set_hmac(hmac_provider_.ComputeHmac(message, "wrong_hmac"));
    WritePacket(message, command,  "");
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<MockHandler>();
    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::CLIENT_RESPONSE_HMAC_VERIFICATION_ERROR,
        "Response HMAC mismatch"), NULL));
    ASSERT_TRUE(receiver.Enqueue(handler, 0, 0));
    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ErrorCausesAllEnqueuedRequestsToFail) {
    // If we encounter an error such as an invalid magic character or incorrect
    // HMAC, there's not much point in continuing the connection, so the
    // receiver should execute the failure callback on all enqueued requests.
    const char header[] = { 'E' };  // invalid magic character
    ASSERT_EQ(static_cast<ssize_t>(sizeof(header)), write(fds_[1], header, sizeof(header)));
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Error(KineticStatusEq(
            StatusCode::CLIENT_IO_ERROR, "I/O read error"), NULL));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(
            StatusCode::CLIENT_IO_ERROR, "I/O read error"), NULL));
    ASSERT_TRUE(receiver.Enqueue(handler1, 0, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 1, 1));
    ASSERT_EQ(kError, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, DestructorDeletesOutstandingRequests) {
    // When the receiver's destructor is called, it should execute the error
    // callback on any outstanding requests and also delete their handlers.
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fds_[0]));
    EXPECT_CALL(*socket_wrapper, getSSL()).WillRepeatedly(Return((SSL*) 0));
    ConnectionOptions options;
    options.user_id = 3;
    options.hmac_key = "key";
    NonblockingReceiver *receiver = new NonblockingReceiver(socket_wrapper,
        hmac_provider_, options);

    auto handler1 = make_shared<MockHandler>();
    auto handler2 = make_shared<MockHandler>();
    EXPECT_CALL(*handler1, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Receiver shutdown"), NULL));
    EXPECT_CALL(*handler2, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Receiver shutdown"), NULL));
    ASSERT_TRUE(receiver->Enqueue(handler1, 0, 0));
    ASSERT_TRUE(receiver->Enqueue(handler2, 1, 1));
    delete receiver;
}

TEST_F(NonblockingReceiverTest, RemoveInvalidHandlerKeyDoesntPerturbNormalOperation) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));

    ASSERT_FALSE(receiver.Remove(9732412));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}


TEST_F(NonblockingReceiverTest, RemoveValidHandlerKeyDeregistersHandler) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_TRUE(receiver.Enqueue(handler2, 34, 1));

    ASSERT_TRUE(receiver.Remove(1));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, EnqueueReturnsFalseWhenReUsingHandlerKey) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // existing handler should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, EnqueueDoesntSaveHandlerWhenErroneouslyReUsingHandlerKey) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));

    // the handler should not be recorded since the handler key is a dup
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // handler1 should still be called

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest,
        EnqueueWithDuplicateHandlerKeyDoesntPreventSubsequentMessageSeqReuse) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler1 = make_shared<StrictMock<MockHandler>>();
    auto handler2 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler1, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler1, 33, 0));

    // the handler should not be recorded since the handler key is a dup
    ASSERT_FALSE(receiver.Enqueue(handler2, 34, 0));

    // handler1 should still be called

    ASSERT_EQ(kIdle, receiver.Receive());

    // this handler uses the same message seq as the one whose handler key prevented enqueuing
    auto handler3 = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler3, Handle_(_, "value2"));
    ASSERT_TRUE(receiver.Enqueue(handler3, 34, 1));

    command.mutable_status()->set_code(Command_Status_StatusCode_SUCCESS);
    command.mutable_header()->set_acksequence(34);
    Message message;
    WritePacket(message, command, "value2");

    ASSERT_EQ(kIdle, receiver.Receive());
}

TEST_F(NonblockingReceiverTest, ExecutingHandlerRemovesHandlerKey) {
    Command command;
    auto socket_wrapper = make_shared<MockSocketWrapperInterface>();
    ConnectionOptions options;
    defaultReceiverSetup(command, socket_wrapper, options);
    NonblockingReceiver receiver(socket_wrapper, hmac_provider_, options);

    auto handler = make_shared<StrictMock<MockHandler>>();
    EXPECT_CALL(*handler, Handle_(_, "value"));
    ASSERT_TRUE(receiver.Enqueue(handler, 33, 0));

    ASSERT_EQ(kIdle, receiver.Receive());

    ASSERT_FALSE(receiver.Remove(0));
}

}  // namespace kinetic
