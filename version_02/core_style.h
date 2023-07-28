#pragma once
#include <ostream>
#include <string>

using skill_id_t = size_t;
class core_style_t
{
public:
	core_style_t() = delete;
	core_style_t(skill_id_t _first, skill_id_t _second, skill_id_t _third)
		: first(_first), second(_second), third(_third)
	{
		if (is_element_duplicated())
			throw std::exception(operator std::string().c_str());
	}
	core_style_t(const core_style_t& _other)
		: core_style_t(_other.first, _other.second, _other.third)
	{}

public:
	operator std::string() const { return std::format("Core Style [ {}, {}, {} ]", first, second, third); }
	friend std::ostream& operator<<(std::ostream& ostream, const core_style_t& core_style) { return ostream << core_style.operator std::string(); }

	bool is_element_duplicated() const { return first == second || first == third || second == third; }

	bool is_strictly_equal(const core_style_t& other) const;
	bool is_normally_equal(const core_style_t& other) const;
	bool is_virtually_equal(const core_style_t& other) const;

	bool is_normally_less(const core_style_t& other) const;
	bool is_virtually_less(const core_style_t& other) const;
	
	bool operator==(const core_style_t& other) const { return is_normally_equal(other); }
	bool operator!=(const core_style_t& other) const { return !(this->operator==(other)); }
	bool operator<(const core_style_t& other) const { return is_normally_less(other); }

	inline skill_id_t get_first() const { return first; }
	inline skill_id_t get_second() const { return second; }
	inline skill_id_t get_third() const { return third; }

public:
	skill_id_t first;
	skill_id_t second;
	skill_id_t third;
};


bool core_style_t::is_strictly_equal(const core_style_t& other) const { return this == &other; }
bool core_style_t::is_normally_equal(const core_style_t& other) const
{
	return this->first == other.first &&
		this->second == other.second &&
		this->third == other.third;
}
bool core_style_t::is_virtually_equal(const core_style_t& other) const
{
	return this->first == other.first &&
		((this->second == other.second && this->third == other.third) ||
			(this->second == other.third && this->third == other.second));
}

bool core_style_t::is_normally_less(const core_style_t& other) const
{
	return this->first == other.first ?
		((this->second == other.second) ?
			this->third < other.third :
			this->second < other.second) :
		this->first < other.first;
}
bool core_style_t::is_virtually_less(const core_style_t& other) const
{
	return this->first == other.first ?
		(std::min(this->second, this->third) == std::min(other.second, other.third) ?
			std::max(this->second, this->third) < std::max(other.second, other.third) :
			std::min(this->second, this->third) < std::min(other.second, other.third)) :
		this->first < other.first;
}
