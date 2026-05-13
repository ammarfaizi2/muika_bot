// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__TESTS__MOCK_REACTOR_HPP
#define MUIKA__TESTS__MOCK_REACTOR_HPP

#include <muika/OutboundMessage.hpp>
#include <muika/Reactor.hpp>

#include <mutex>
#include <string>
#include <vector>

namespace muika {
namespace tests {

class MockReactor: public muika::Reactor {
public:
	enum class Kind {
		Text,
		PhotoUrl,
		PhotoFile,
	};

	struct SentMessage {
		Kind kind;
		std::string chat_id;
		std::string photo;
		muika::OutboundMessage om;
	};

	MockReactor(void) = default;
	~MockReactor(void) override = default;

	std::string sendMsgText(const std::string &chat_id,
				const muika::OutboundMessage &om) override
	{
		std::lock_guard<std::mutex> lk(mutex_);
		std::string id = nextId();
		sent_.push_back({Kind::Text, chat_id, "", om});
		return id;
	}

	std::string sendMsgPhotoUrl(const std::string &chat_id,
				    const std::string &photo_url,
				    const muika::OutboundMessage &om) override
	{
		std::lock_guard<std::mutex> lk(mutex_);
		std::string id = nextId();
		sent_.push_back({Kind::PhotoUrl, chat_id, photo_url, om});
		return id;
	}

	std::string sendMsgPhotoFile(const std::string &chat_id,
				     const std::string &photo_file_path,
				     const muika::OutboundMessage &om) override
	{
		std::lock_guard<std::mutex> lk(mutex_);
		std::string id = nextId();
		sent_.push_back({Kind::PhotoFile, chat_id, photo_file_path, om});
		return id;
	}

	std::vector<SentMessage> drain(void)
	{
		std::lock_guard<std::mutex> lk(mutex_);
		auto out = std::move(sent_);
		sent_.clear();
		return out;
	}

	std::size_t size(void)
	{
		std::lock_guard<std::mutex> lk(mutex_);
		return sent_.size();
	}

private:
	std::mutex mutex_;
	std::vector<SentMessage> sent_;
	uint64_t next_id_ = 1000;

	std::string nextId(void)
	{
		return std::to_string(next_id_++);
	}
};

} /* namespace tests */
} /* namespace muika */

#endif /* MUIKA__TESTS__MOCK_REACTOR_HPP */
