#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <exception>
#include <functional>

#include <queue>
#include "./double_priority_queue.h"

#include <array>
#include <algorithm>
#include <memory>
#include <numeric>

template <typename _ElementType>
using vector_2d = std::vector<std::vector<_ElementType>>;

const std::filesystem::path input_file_name("./input.txt");
const std::filesystem::path output_file_name("./output.txt");


void desynchronize_io()
{
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(static_cast<std::ostream*>(nullptr));
	std::cerr.tie(static_cast<std::ostream*>(nullptr));
}

using skill_id_t = size_t;

class core_style_t
{
public:
	core_style_t() = delete;
	core_style_t(skill_id_t _first, skill_id_t _second, skill_id_t _third)
		: first(_first), second(_second), third(_third)
	{
		if (_first == _second ||
			_first == _third ||
			_second == _third)
		{
			std::string what("core_style element is duplicated { ");
			what += std::to_string(_first) + ", ";
			what += std::to_string(_second) + ", ";
			what += std::to_string(_third) + " }";
			throw std::exception(what.c_str());
		}
	}
	core_style_t(const core_style_t& _other)
		: core_style_t(_other.first, _other.second, _other.third)
	{}

public:
	bool is_strictly_equal(const core_style_t& other) const { return this == &other; }
	bool is_normally_equal(const core_style_t& other) const
	{
		return this->first == other.first &&
			this->second == other.second &&
			this->third == other.third;
	}
	bool is_virtually_equal(const core_style_t& other) const
	{
		return this->first == other.first &&
			((this->second == other.second && this->third == other.third) ||
			(this->second == other.third && this->third == other.second));
	}

	bool operator==(const core_style_t& other) const { return is_normally_equal(other); }
	bool operator!=(const core_style_t& other) const { return !(this->operator==(other)); }

	bool operator<(const core_style_t& _other) const
	{
		return (this->first == _other.first) ?
			((this->second == _other.second) ?
				this->third < _other.third :
				this->second < _other.second) :
			this->first < _other.first;
	}
	bool operator>(const core_style_t& _other) const
	{
		return (this->first == _other.first) ?
			((this->second == _other.second) ?
				this->third > _other.third :
		this->second > _other.second) :
			this->first > _other.first;
	}

public:
	skill_id_t first;
	skill_id_t second;
	skill_id_t third;
};

std::ostream& operator<<(std::ostream& ostream, const core_style_t& core_style)
{
	return ostream << "C{ " << core_style.first << ", " << core_style.second << ", " << core_style.third << " }";
};

#include <random>

struct Input_t
{
	size_t usable_slot_quantity;
	std::vector<size_t> enhanced_slot_quantity_list;
	std::vector<size_t> core_level_list;
	std::vector<size_t> skill_value_list;
	std::vector<std::vector<core_style_t>> core_style_grid;
} inputData;

int main_in_try();
int main()
{
	desynchronize_io();

	// read file
	std::fstream fileStream(input_file_name);
	std::vector<std::vector<size_t>> excel_grid;
	excel_grid.reserve(200);
	while (!fileStream.eof())
	{
		std::vector<size_t> excel_list;
		excel_list.reserve(3);

		char rawStream[65536];
		fileStream.getline(rawStream, 65535);
		
		std::stringstream sstream(rawStream);
		while (!sstream.eof())
		{
			std::string value;
			sstream >> value;
			if (value.size())
				excel_list.emplace_back(std::stoull(value));
		}

		if (!excel_list.empty())
			excel_grid.emplace_back(std::move(excel_list));
	}

	if (excel_grid.size() < 3)
	{
		std::cout << "no core\n";
		return 0;
	}

	// excel to input data
	auto enhanced_slot_quantity_list = std::move(excel_grid[0]);
	size_t usable_slot_quantity = std::accumulate(enhanced_slot_quantity_list.begin(), enhanced_slot_quantity_list.end(), 0);
	auto core_level_list = std::move(excel_grid[1]);
	auto skill_value_list = std::move(excel_grid[2]);
	core_level_list.resize(17, 0);
	skill_value_list.resize(17, 0);

	std::vector<std::vector<size_t>> raw_core_style_list;
	raw_core_style_list.insert(raw_core_style_list.end(), std::next(excel_grid.begin(), 3), excel_grid.end());
	
	// repackage to core_style_grid
	std::vector<std::vector<core_style_t>> core_style_grid(17);
	for (auto& raw_core_style : raw_core_style_list)
	{
		size_t& first = raw_core_style[0];
		size_t& second = raw_core_style[1];
		size_t& third = raw_core_style[2];
		if (first == 0)		continue;
		if (second == 0)	second = 15ULL + 1;
		if (third == 0)		third = 15ULL + 2;

		for (auto& id : raw_core_style)
			id -= 1;
		core_style_grid[first].emplace_back(first, second, third);
	}

	size_t expected_combine_count = 1;
	for (auto& core_style_list : core_style_grid)
	{
		auto is_core_style_virtually_less = [](const core_style_t& l, const core_style_t& r)
		{
			return l.first == r.first ?
				(std::min(l.second, l.third) == std::min(r.second, r.third) ?
					std::max(l.second, l.third) < std::max(r.second, r.third) :
					std::min(l.second, l.third) < std::min(r.second, r.third)) :
				l.first < r.first;
		};
		auto is_core_style_virtually_equal = [](auto& l, auto& r)->bool
		{
			return l.is_virtually_equal(r);
		};

		std::sort(core_style_list.begin(), core_style_list.end(), is_core_style_virtually_less);
		core_style_list.erase(std::unique(core_style_list.begin(), core_style_list.end(), is_core_style_virtually_equal), core_style_list.end());

		expected_combine_count *= core_style_list.size() + 1;
	}

	size_t expected_assign_count = 1;
	std::function<size_t(size_t)> factorial;
	factorial = [&factorial](size_t i)->size_t
	{
		return i <= 1 ? 1 : i * factorial(i-1);
	};
	size_t decrease = 0;
	for (size_t i = 0; i < enhanced_slot_quantity_list.size() - 1; i++)
	{
		auto enhanced_slot_quantity = enhanced_slot_quantity_list[i];
		auto enhanceable_slot_quantity = usable_slot_quantity - decrease;

		expected_assign_count *= factorial(enhanceable_slot_quantity) / factorial(enhanceable_slot_quantity - enhanced_slot_quantity) / factorial(enhanced_slot_quantity);
		decrease += enhanced_slot_quantity;
	}

	std::cout << "EXPECTED : " << expected_combine_count << " * "<< expected_assign_count << std::endl;

	try
	{
		inputData.usable_slot_quantity = usable_slot_quantity;
		inputData.enhanced_slot_quantity_list = std::move(enhanced_slot_quantity_list);
		inputData.core_level_list = std::move(core_level_list);
		inputData.skill_value_list = std::move(skill_value_list);
		inputData.core_style_grid = std::move(core_style_grid);

		main_in_try();
	}
	catch (const std::exception& exception)
	{
		std::cout << "EXCEPTION : " << exception.what() << std::endl;
	}
}

constexpr size_t skill_level_limit_normal = 50;
constexpr size_t skill_level_limit_enhancement = 60;
constexpr size_t enhancement_level_limit = 5;
const bool less_enhancement_priority = true;

size_t skill_quantity = NULL;

std::vector<size_t>* core_level_list__p;
int main_in_try()
{
	core_level_list__p = &inputData.core_level_list;
	skill_quantity = inputData.skill_value_list.size();

	//--------------------------------------------------


	using enhanced_slot_assignment_order_t = std::vector<size_t>;
	using enhanced_slot_assignment_order_event_t = std::function<void(const enhanced_slot_assignment_order_t&)>;
	auto for_each_of_all_enhanced_slot_assignment_order = [](std::vector<size_t> enhanced_slot_quantity_list, size_t assignable_slot_quantity, enhanced_slot_assignment_order_event_t enhanced_slot_assignment_order_event)
	{
		enhanced_slot_assignment_order_t enhanced_slot_assigned_order;
		enhanced_slot_assigned_order.reserve(assignable_slot_quantity);
		
		// DEBUG
		bool HACK = false;
		if (HACK)
		{
			enhanced_slot_assigned_order.resize(assignable_slot_quantity, 0);
			enhanced_slot_assignment_order_event(enhanced_slot_assigned_order);
			return;
		}

		std::function<void()> assign_enhanced_slot;
		assign_enhanced_slot = [&]()
		{
			size_t slot_index = enhanced_slot_assigned_order.size();
			if (slot_index == assignable_slot_quantity)
			{
				enhanced_slot_assignment_order_event(enhanced_slot_assigned_order);
				return;
			}

			for (size_t enhancement_level = 0; enhancement_level <= enhancement_level_limit; enhancement_level++)
			{
				size_t& enhanced_slot_quantity = enhanced_slot_quantity_list[enhancement_level];
				if (enhanced_slot_quantity == 0)
					continue;

				enhanced_slot_quantity -= 1;
				enhanced_slot_assigned_order.push_back(enhancement_level);
				{
					assign_enhanced_slot();
				}
				enhanced_slot_assigned_order.pop_back();
				enhanced_slot_quantity += 1;
			}
		};
		assign_enhanced_slot();
	};

	using core_style_combination_t = std::vector<core_style_t>;
	using core_style_combination_event_t = std::function<void(const core_style_combination_t&)>;
	auto for_each_of_all_core_style_combination = [](const std::vector<std::vector<core_style_t>>& core_style_grid, size_t slot_quantity, core_style_combination_event_t core_style_combine_event)
	{
		core_style_combination_t core_style_combination;
		core_style_combination.reserve(slot_quantity);

		std::function<void(skill_id_t)> simulate_core_combination;
		simulate_core_combination = [&](skill_id_t main_skill_id_begin)
		{
			// Prevent unintended actions
			{
				size_t remain_slot = slot_quantity - core_style_combination.size();
				if (remain_slot <= 0)
					throw std::exception("equip logic error");
			}

			for (skill_id_t main_skill_id = main_skill_id_begin; main_skill_id < skill_quantity; main_skill_id++)
			{
				const auto& core_style_list = core_style_grid[main_skill_id];
				size_t for_develop_count = 0;
				for (const core_style_t& core_style : core_style_list)
				{
					// DEBUG
					if (main_skill_id_begin == main_skill_id &&
						core_style_combination.size() <= 2)
					{
						for_develop_count++;

						std::cout << "START : ";
						for (size_t i = 0; i < core_style_combination.size(); i++)
							std::cout << "\t";
						std::cout << main_skill_id
							<< " : [ "
							<< for_develop_count
							<< " / "
							<<core_style_grid[main_skill_id].size()
							<< " ]"
							<< "\n";
					}

					core_style_combination.push_back(core_style);	// equip
					{
						core_style_combine_event(core_style_combination);

						size_t remain_slot = slot_quantity - core_style_combination.size();
						if (remain_slot > 0)
							simulate_core_combination(main_skill_id + 1);
					}
					core_style_combination.pop_back(); // unequip
				}
			}
		};

		constexpr skill_id_t id_begin = 0;
		size_t remain_slot = slot_quantity - core_style_combination.size();
		if (remain_slot > 0)
			simulate_core_combination(id_begin);
		else
			throw std::exception("no core slot");
	};

	class core_matrix_t
	{
	public:
		core_matrix_t() = delete;
		core_matrix_t(core_style_combination_t core_style_combination, enhanced_slot_assignment_order_t enhanced_slot_assignment_order)
			: _core_style_combination(core_style_combination)
			, _enhanced_slot_assignment_order(enhanced_slot_assignment_order)

			, _used_slot_quantity(0)
			, _used_enhancement_level(0)
			, _total_skill_level(0)
			, _skill_level_list(skill_quantity, 0)
		{
			if (core_style_combination.size() != enhanced_slot_assignment_order.size())
				throw std::exception("core_matrix slot size not equal");

			_used_slot_quantity = core_style_combination.size();
			_used_enhancement_level = 0;
			std::vector<size_t> skill_unlimited_level_list(skill_quantity, 0);
			std::vector<size_t> skill_unlimited_enhancement_list(skill_quantity, 0);

			for (size_t slot_index = 0; slot_index < _used_slot_quantity; slot_index++)
			{
				core_style_t core_style = core_style_combination[slot_index];
				size_t enhanced_level = enhanced_slot_assignment_order[slot_index];

				_used_enhancement_level += enhanced_level;

				std::array<skill_id_t, 3> skill_id_array = { core_style.first, core_style.second, core_style.third };
				for (skill_id_t skill_id : skill_id_array)
				{
					skill_unlimited_level_list[skill_id] += (*core_level_list__p)[skill_id];
					skill_unlimited_enhancement_list[skill_id] += enhanced_level;
				}
			}

			for (skill_id_t skill_id = 0; skill_id < skill_quantity; skill_id++)
			{
				size_t skill_unlimited_level = skill_unlimited_level_list[skill_id];
				size_t skill_unlimited_enhanced_level = skill_unlimited_enhancement_list[skill_id];
				size_t skill_limited_normal_level = std::min(skill_unlimited_level, skill_level_limit_normal);
				size_t skill_limited_enhanced_level = std::min(skill_limited_normal_level + skill_unlimited_enhanced_level, skill_level_limit_enhancement);

				_total_skill_level += skill_limited_enhanced_level;
				_skill_level_list[skill_id] = skill_limited_enhanced_level;
			}
		}

	public:
		inline const core_style_combination_t& get_core_style_combination() const { return _core_style_combination; }
		inline const enhanced_slot_assignment_order_t& get_enhanced_slot_assignment_order() const { return _enhanced_slot_assignment_order; }

		inline size_t get_skill_level(skill_id_t skill_id) const { return _skill_level_list[skill_id]; }
		inline size_t get_total_skill_level() const { return _total_skill_level; }
		inline size_t get_used_slot_quantity() const { return _used_slot_quantity; }
		inline size_t get_used_enhancement_level() const { return _used_enhancement_level; }

	private:
		core_style_combination_t _core_style_combination;
		enhanced_slot_assignment_order_t _enhanced_slot_assignment_order;

		size_t _used_slot_quantity;
		size_t _used_enhancement_level;
		size_t _total_skill_level;
		std::vector<size_t> _skill_level_list;
	};

	class core_matrix_value_t
	{
	public:
		core_matrix_value_t()
			: _core_matrix(nullptr)
			, _skills_value(0)
		{}
		core_matrix_value_t(std::shared_ptr<const core_matrix_t> const core_matrix, const std::vector<size_t>& skill_weight_list)
			: _core_matrix(core_matrix)
			, _skills_value(0)
		{
			for (skill_id_t skill_id = 0; skill_id < skill_quantity; skill_id++)
			{
				size_t skill_level = _core_matrix->get_skill_level(skill_id);
				size_t skill_weight = skill_weight_list[skill_id];
				size_t skill_value = skill_level * skill_weight;
				_skills_value += skill_value;
			}
		}

		core_matrix_value_t& operator=(const core_matrix_value_t& other)
		{
			_core_matrix = other._core_matrix;
			_skills_value = other._skills_value;
			return *this;
		}

	public:
		bool operator<(const core_matrix_value_t& other) const
		{
			if (_core_matrix == nullptr &&
				other._core_matrix == nullptr)
				return false;
			if (_core_matrix == nullptr)
				return true;
			if (other._core_matrix == nullptr)
				return false;

			if (_skills_value == other._skills_value)
			{
				size_t used_enhancement_level = _core_matrix->get_used_enhancement_level();
				size_t other_used_enhancement_level = other._core_matrix->get_used_enhancement_level();
				size_t used_slot_quantity = _core_matrix->get_used_slot_quantity();
				size_t other_used_slot_quantity = other._core_matrix->get_used_slot_quantity();

				if (less_enhancement_priority)
					return (other_used_enhancement_level == used_enhancement_level) ?
					(other_used_slot_quantity < used_slot_quantity) :
					(other_used_enhancement_level < used_enhancement_level);
				else
					return (other_used_slot_quantity == used_slot_quantity) ?
					(other_used_enhancement_level < used_enhancement_level) :
					(other_used_slot_quantity < used_slot_quantity);
			}
			else
				return _skills_value < other._skills_value;
		}
		bool operator>(const core_matrix_value_t& other) const
		{
			if (_core_matrix == nullptr &&
				other._core_matrix == nullptr)
				return false;
			if (_core_matrix == nullptr)
				return false;
			if (other._core_matrix == nullptr)
				return true;

			if (_skills_value == other._skills_value)
			{
				size_t used_enhancement_level = _core_matrix->get_used_enhancement_level();
				size_t other_used_enhancement_level = other._core_matrix->get_used_enhancement_level();
				size_t used_slot_quantity = _core_matrix->get_used_slot_quantity();
				size_t other_used_slot_quantity = other._core_matrix->get_used_slot_quantity();

				if (less_enhancement_priority)
					return (other_used_enhancement_level == used_enhancement_level) ?
					(other_used_slot_quantity > used_slot_quantity) :
					(other_used_enhancement_level > used_enhancement_level);
				else
					return (other_used_slot_quantity == used_slot_quantity) ?
					(other_used_enhancement_level > used_enhancement_level) :
					(other_used_slot_quantity > used_slot_quantity);
			}
			else
				return _skills_value > other._skills_value;
		}

		inline const std::shared_ptr<const core_matrix_t> get_core_matrix() const { return _core_matrix; }
		inline size_t get_skills_value() const { return _skills_value; }

	private:
		std::shared_ptr<const core_matrix_t> _core_matrix;
		size_t _skills_value;
	};

	double_priority_queue<core_matrix_value_t, std::greater<core_matrix_value_t>> dpq;
	constexpr size_t matrix_pqueue_size_limit = 100;

	size_t slot_quantity = inputData.usable_slot_quantity;
	for_each_of_all_core_style_combination(inputData.core_style_grid, slot_quantity, [&](const core_style_combination_t& core_style_combination)
		{
			size_t combination_used_slot_quantity = core_style_combination.size();
			bool is_only_collect_best_enhanced_matrix_in_combination = true;

			auto collect_core_matrix_value = [&dpq](const core_matrix_value_t& core_matrix_value)
			{
				if (dpq.empty() || dpq.bottom() < core_matrix_value)
				{
					dpq.push(core_matrix_value);
					if (dpq.size() > matrix_pqueue_size_limit)
						dpq.pop_bottom();
				}
			};

			core_matrix_value_t best_enhanced_matrix_in_combination;
			for_each_of_all_enhanced_slot_assignment_order(inputData.enhanced_slot_quantity_list, combination_used_slot_quantity, [&](const enhanced_slot_assignment_order_t& enhanced_slot_assignment_order)
				{
					std::shared_ptr<core_matrix_t> core_matrix = std::make_shared<core_matrix_t>(core_style_combination, enhanced_slot_assignment_order);

					core_matrix_value_t core_matrix_value(core_matrix, inputData.skill_value_list);
					best_enhanced_matrix_in_combination = std::max(best_enhanced_matrix_in_combination, core_matrix_value);

					if (is_only_collect_best_enhanced_matrix_in_combination == false)
						collect_core_matrix_value(core_matrix_value);
				});

			if (is_only_collect_best_enhanced_matrix_in_combination == true)
				collect_core_matrix_value(best_enhanced_matrix_in_combination);
		});

	auto cmv = dpq.top();
	std::fstream file_stream(output_file_name, std::fstream::out | std::fstream::trunc);
	if (file_stream)
	{
		auto cm = cmv.get_core_matrix();
		auto csc = cm->get_core_style_combination();
		auto enh = cm->get_enhanced_slot_assignment_order();

		//std::array<size_t, 6> used_enhanced_slot_quantity_list;
		//used_enhanced_slot_quantity_list.fill(0);
		//
		//for (size_t enhancement: enh)
		//	if (enhancement > 0)
		//		used_enhanced_slot_quantity_list[enhancement]++;
		//used_enhanced_slot_quantity_list[0] = cm->get_used_slot_quantity() - std::accumulate(std::next(used_enhanced_slot_quantity_list.begin(), 1), used_enhanced_slot_quantity_list.end(), 0);

		for (skill_id_t skill_id = 0; skill_id < cm->get_used_slot_quantity(); skill_id++)
		{
			size_t first = csc[skill_id].first + 1;
			size_t second = csc[skill_id].second + 1;
			size_t third = csc[skill_id].third + 1;
			size_t enhancement = enh[skill_id];

			file_stream << first << "\t" << second << "\t" << third << "\t" << enhancement << "\n";
		}
	}


	while (!dpq.empty())
	{
		auto cmv = dpq.top();
		dpq.pop_top();

		auto cm = cmv.get_core_matrix();
		auto csc = cm->get_core_style_combination();
		auto enh = cm->get_enhanced_slot_assignment_order();

		std::cout << cmv.get_skills_value() << ", ";
		std::cout << cm->get_used_slot_quantity() << ", ";
		std::cout << cm->get_used_enhancement_level() << " : ";
		for (size_t skill_id = 0; skill_id < skill_quantity; skill_id++)
		{
			auto lv = cm->get_skill_level(skill_id);
			std::cout << lv << " ";
		}
		std::cout << "{ ";
		for (skill_id_t skill_id = 0; skill_id < cm->get_used_slot_quantity(); skill_id++)
		{
			std::cout << "{ ";
			std::cout << csc[skill_id].first;
			std::cout << ", ";
			std::cout << csc[skill_id].second;
			std::cout << ", ";
			std::cout << csc[skill_id].third;
			std::cout << " (";
			std::cout << "+" << enh[skill_id];
			std::cout << ") }";
		}
		std::cout << " }";

		std::cout << "\n";
	}
	return 0;
}