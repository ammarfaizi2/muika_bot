#include <muika/Muika.hpp>
#include <iostream>

class SimpleReactor: public muika::Reactor {
public:
	SimpleReactor(void) {}
	virtual ~SimpleReactor(void) {}

	virtual void sendMsgText(const std::string &chat_id,
				 const std::string &text,
				 const std::string &reply_to_msg_id = "") override
	{
		std::cout << "sendMsgText: " << chat_id << ", " << text << ", " << reply_to_msg_id << std::endl;
	}

	virtual void sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const std::string &caption = "",
				     const std::string &reply_to_msg_id = "") override
	{
		std::cout << "sendMsgPhotoUrl: " << chat_id << ", " << photo_url << ", " << caption << ", " << reply_to_msg_id << std::endl;
	}

	virtual void sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const std::string &caption = "",
				      const std::string &reply_to_msg_id = "") override
	{
		std::cout << "sendMsgPhotoFile: " << chat_id << ", " << photo_file_path << ", " << caption << ", " << reply_to_msg_id << std::endl;
	}
};

int main(void)
{
	std::shared_ptr<muika::Reactor> reactor = std::make_shared<SimpleReactor>();
	muika::MuikaConfig cfg;
	cfg.storage_path = "./data";
	muika::Muika muika(cfg, reactor);
	muika.loadModule("hello");
	muika::MsgPtr msg = muika.createMsgText("chat_id", "msg_id", "user_id", "name", "uname", "Hello, world!");
	muika.passMsg(msg);
	return 0;
}
