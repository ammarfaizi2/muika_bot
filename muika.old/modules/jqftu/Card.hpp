// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__CARD_HPP
#define MUIKA__MODULES__JQFTU__CARD_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

class Card {
private:
	uint8_t romaji_fault_tolerance_ = 10; /* In percent */
	bool cmpAllowSimilarRomaji(const std::string &a, const std::string &b);

protected:

	static std::string generateLatexPng(const char *latex_code);
	static std::string generateLatexPngUrl(const char *latex_code);
	static void freeLatexHeap(char *ptr);
	inline void setRomajiFaultTolerance(uint8_t tolerance) { romaji_fault_tolerance_ = tolerance; }
	bool normalizedRomajiCompare(const std::string &a, const std::string &b);

public:
	Card(void) = default;
	virtual ~Card(void) = default;
	virtual bool checkAnswer(const std::string &answer) = 0;
	virtual std::string getCardImage(void) = 0;
	virtual std::string getCardCaption(void) = 0;
	virtual std::string getCardAnswer(void) = 0;
	virtual std::string getCardDetails(void) = 0;
	virtual const std::vector<std::string> &getCardPhotos(void);
	virtual json toJson(void) const = 0;
	virtual void fromJson(const json &j) = 0;
	inline uint8_t getRomajiFaultTolerance(void) const { return romaji_fault_tolerance_; }

	static void threadInit(void);
	static void threadFree(void);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__CARD_HPP */
