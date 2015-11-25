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

namespace kinetic {

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using com::seagate::kinetic::proto::Command_MessageType_GET_RESPONSE;
using com::seagate::kinetic::proto::Command_Status_StatusCode_SUCCESS;

using std::string;
using std::make_shared;

// NonblockingPacketServiceTest

TEST(NonblockingPacketServiceTest, HandlesSenderFailure) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    auto sender = new MockNonblockingSender();
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();
    EXPECT_CALL(*sender, Send()).WillOnce(Return(kError));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));
}

TEST(NonblockingPacketServiceTest, SubmitAfterFailureInvokesErrorOnCallback) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    auto sender = new MockNonblockingSender();
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();
    EXPECT_CALL(*sender, Send()).WillOnce(Return(kError));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));

    auto handler = new MockHandler();

    EXPECT_CALL(*handler, Error(KineticStatusEq(StatusCode::CLIENT_SHUTDOWN,
        "Client already shut down"), NULL));

    service.Submit(unique_ptr<Message>(), unique_ptr<Command>(), make_shared<string>("zomg"),
        unique_ptr<HandlerInterface>(handler));
}

TEST(NonblockingPacketServiceTest, CanRemoveHandlerAfterError) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    auto sender = new MockNonblockingSender();
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    EXPECT_CALL(*sender, Remove(42)).WillOnce(Return(false));
    EXPECT_CALL(*sender, Send()).WillOnce(Return(kError));
    EXPECT_CALL(*receiver, Remove(42)).WillOnce(Return(false));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));

    // should not crash
    ASSERT_FALSE(service.Remove(42));
}

TEST(NonblockingPacketServiceTest, RemoveCallsSenderButReceiverWhenSenderRemoves) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    HandlerKey key = 0L;

    EXPECT_CALL(*sender, Remove(key)).WillOnce(Return(true));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    ASSERT_TRUE(service.Remove(key));
}

TEST(NonblockingPacketServiceTest, RemoveCallsBothSenderAndReceiverWhenSenderDoesntRemove) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    HandlerKey key = 0L;
    EXPECT_CALL(*sender, Remove(key)).WillOnce(Return(false));
    EXPECT_CALL(*receiver, Remove(key)).WillOnce(Return(true));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    ASSERT_TRUE(service.Remove(key));
}

TEST(NonblockingPacketServiceTest, RemoveReturnsFalseWhenNeitherSenderNorRecieverCanRemove) {
    // If the sender fails, the service should return an error and subsequent
    // calls should fail immediately without even attempting I/O.
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();


    HandlerKey key = 0L;
    EXPECT_CALL(*sender, Remove(key)).WillOnce(Return(false));
    EXPECT_CALL(*receiver, Remove(key)).WillOnce(Return(false));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    ASSERT_FALSE(service.Remove(key));
}

TEST(NonblockingPacketServiceTest, HandlesReceiverFailure) {
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    EXPECT_CALL(*sender, Send()).WillOnce(Return(kIdle));
    EXPECT_CALL(*receiver, Receive()).WillOnce(Return(kError));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));
    ASSERT_FALSE(service.Run(&read_fds, &write_fds, &nfds));
}

TEST(NonblockingPacketServiceTest, SetsWriteFileDescriptorWhenBusySending) {
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    const int fd = 42;
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fd));
    EXPECT_CALL(*sender, Send()).WillOnce(Return(kIoWait));
    EXPECT_CALL(*receiver, Receive()).WillOnce(Return(kIdle));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_TRUE(service.Run(&read_fds, &write_fds, &nfds));
    ASSERT_FALSE(FD_ISSET(fd, &read_fds));
    ASSERT_TRUE(FD_ISSET(fd, &write_fds));
    ASSERT_EQ(fd + 1, nfds);
}

TEST(NonblockingPacketServiceTest, SetsReadFileDescriptorWhenBusyReceiving) {
    MockNonblockingSender *sender = new StrictMock<MockNonblockingSender>;
    auto receiver = make_shared<StrictMock<MockNonblockingReceiver>>();
    auto socket_wrapper = make_shared<StrictMock<MockSocketWrapperInterface>>();

    const int fd = 42;
    EXPECT_CALL(*socket_wrapper, fd()).WillRepeatedly(Return(fd));
    EXPECT_CALL(*sender, Send()).WillOnce(Return(kIdle));
    EXPECT_CALL(*receiver, Receive()).WillOnce(Return(kIoWait));

    NonblockingPacketService service(socket_wrapper, unique_ptr<NonblockingSenderInterface>(sender),
        receiver);

    fd_set read_fds, write_fds;
    int nfds;
    ASSERT_TRUE(service.Run(&read_fds, &write_fds, &nfds));
    ASSERT_TRUE(FD_ISSET(fd, &read_fds));
    ASSERT_FALSE(FD_ISSET(fd, &write_fds));
    ASSERT_EQ(fd + 1, nfds);
}

}  // namespace kinetic
