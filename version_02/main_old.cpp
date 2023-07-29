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
#include <ranges>

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

#include "core_style.h"

//tmp
constexpr size_t skill_level_limit_normal = 50;
constexpr size_t skill_level_limit_enhancement = 60;
constexpr size_t enhancement_level_limit = 5;
const bool less_enhancement_priority = true;
size_t skill_quantity = NULL;
//tmp

using skill_id_t = size_t;
using level_t = size_t;
using value_t = size_t;

class core_value_t
{
public:
	core_value_t(core_style_t* core_style, const std::vector<level_t>& core_level_list, const std::vector<value_t>& skill_value_list)
		: _core_style(nullptr)
		, _first(0)
		, _second(0)
		, _third(0)
		, _total(0)
	{
		if (core_style == nullptr)
			throw std::invalid_argument("core_style is nullptr");

		_first = core_level_list[core_style->get_first()] * skill_value_list[core_style->get_first()];
		_second = core_level_list[core_style->get_second()] * skill_value_list[core_style->get_second()];
		_third = core_level_list[core_style->get_third()] * skill_value_list[core_style->get_third()];
		_total = _first + _second + _third;
	}

	bool operator==(const core_value_t& other) const;
	bool operator<(const core_value_t& other) const;
	bool operator+(const core_value_t& other) const;

	inline value_t get_first() const { return _first; }
	inline value_t get_second() const { return _second; }
	inline value_t get_third() const { return _third; }
	inline value_t get_total() const { return _total; }

private:
	core_style_t* _core_style;
	value_t _first;
	value_t _second;
	value_t _third;
	value_t _total;
};


class evaluator_t
{
public:
	constexpr static level_t _maximum_level = 50;
	constexpr static level_t _maximum_enhanced_level = 60;

public:
	evaluator_t() = delete;
	evaluator_t(std::vector<level_t>&& core_level_list, std::vector<value_t>&& skill_value_list, const std::vector<size_t>& enhanced_slot_quantity_list)
		: _core_level_list(std::move(core_level_list))
		, _skill_value_list(std::move(skill_value_list))
		, _enhanced_slot_quantity_list(enhanced_slot_quantity_list)
	{
	}

public:
	value_t of_core_style(const core_style_t& core_style) const
	{
		skill_id_t first_id = core_style.get_first();
		skill_id_t second_id = core_style.get_second();
		skill_id_t third_id = core_style.get_third();
		value_t first_value = _skill_value_list[first_id];
		value_t second_value = _skill_value_list[second_id];
		value_t third_value = _skill_value_list[third_id];
		return first_value + second_value + third_value;
	}

	value_t of_core(const core_style_t& core_style) const
	{
		skill_id_t first_id = core_style.get_first();
		level_t core_level = _core_level_list[first_id];
		return of_core_style(core_style) * core_level;
	}

	value_t operator()(const core_style_t& core_style) const { return of_core(core_style); }

	//backup
	value_t operator()(const std::vector<core_style_t>& core_style_combination, bool asdf)// const
	{
		skill_id_t id_max = 0;
		for (const core_style_t& core_style : core_style_combination)
			id_max = std::max({ id_max, core_style.get_first(), core_style.get_second(), core_style.get_third() });
		_skill_level_list.clear();
		_skill_level_list.resize(id_max + 1, 0);

		for (const core_style_t& core_style : core_style_combination)
		{
			skill_id_t first_id = core_style.get_first();
			skill_id_t second_id = core_style.get_second();
			skill_id_t third_id = core_style.get_third();
			level_t core_level = _core_level_list[first_id];

			_skill_level_list[first_id] += core_level;
			_skill_level_list[second_id] += core_level;
			_skill_level_list[third_id] += core_level;
		}

		value_t total_value = 0;
		for (skill_id_t skill_id = 0; skill_id < _skill_level_list.size(); skill_id++)
		{
			value_t skill_value = _skill_value_list[skill_id];
			level_t skill_level = _skill_level_list[skill_id];
			level_t limited_skill_level = std::min(skill_level, _maximum_level);
			total_value += limited_skill_level * skill_value;
		}
		return total_value;
	}
	std::pair<value_t, std::vector<size_t>> operator()(const std::vector<core_style_t>& core_style_combination)// const
	{
		using enhanced_slot_assignment_order_t = std::vector<size_t>;
		using enhanced_slot_assignment_order_event_t = std::function<void(const enhanced_slot_assignment_order_t&)>;
		auto for_each_of_all_enhanced_slot_assignment_order = [](std::vector<size_t> enhanced_slot_quantity_list, size_t assignable_slot_quantity, enhanced_slot_assignment_order_event_t enhanced_slot_assignment_order_event)
		{
			enhanced_slot_assignment_order_t enhanced_slot_assigned_order;
			enhanced_slot_assigned_order.reserve(assignable_slot_quantity);

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

		skill_id_t id_max = 0;
		for (const core_style_t& core_style : core_style_combination)
			id_max = std::max({ id_max, core_style.get_first(), core_style.get_second(), core_style.get_third() });
		_skill_level_list.clear();
		_skill_level_list.resize(id_max + 1, 0);

		for (const core_style_t& core_style : core_style_combination)
		{
			skill_id_t first_id = core_style.get_first();
			skill_id_t second_id = core_style.get_second();
			skill_id_t third_id = core_style.get_third();
			level_t core_level = _core_level_list[first_id];

			_skill_level_list[first_id] += core_level;
			_skill_level_list[second_id] += core_level;
			_skill_level_list[third_id] += core_level;
		}

		for (skill_id_t skill_id = 0; skill_id < _skill_level_list.size(); skill_id++)
		{
			level_t& skill_level = _skill_level_list[skill_id];
			skill_level = std::min(skill_level, _maximum_level);
		}

		value_t best_total_value = 0;
		std::vector<size_t> best_slot_enhancement_assignment;
		for_each_of_all_enhanced_slot_assignment_order(_enhanced_slot_quantity_list, core_style_combination.size(), [&](const std::vector<size_t>& slot_enhancement_assignment)
			{
				_enhanced_skill_level_list = _skill_level_list;
				for (size_t slot_index = 0; slot_index < slot_enhancement_assignment.size(); slot_index++)
				{
					size_t enhancement = slot_enhancement_assignment[slot_index];
					const core_style_t& core_style = core_style_combination[slot_index];

					_enhanced_skill_level_list[core_style.get_first()] += enhancement;
					_enhanced_skill_level_list[core_style.get_second()] += enhancement;
					_enhanced_skill_level_list[core_style.get_third()] += enhancement;
				}

				value_t total_value = 0;
				for (skill_id_t skill_id = 0; skill_id < _enhanced_skill_level_list.size(); skill_id++)
				{
					value_t skill_value = _skill_value_list[skill_id];
					level_t skill_level = _enhanced_skill_level_list[skill_id];
					level_t limited_skill_level = std::min(skill_level, _maximum_enhanced_level);
					total_value += limited_skill_level * skill_value;
				}
				if (best_total_value < total_value)
				{
					best_total_value = total_value;
					best_slot_enhancement_assignment = slot_enhancement_assignment;
				}
			});

		return std::make_pair(best_total_value, std::move(best_slot_enhancement_assignment));
	}

private:
	std::vector<level_t> _skill_level_list; // Cache memory for minimizing allocation and release
	std::vector<level_t> _enhanced_skill_level_list; // Cache memory for minimizing allocation and release

	std::vector<level_t> _core_level_list;
	std::vector<value_t> _skill_value_list;
	std::vector<size_t> _enhanced_slot_quantity_list;
};
//} *evaluator = nullptr;


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
		if (second == 0)	second = 15ULL + 0;
		if (third == 0)		third = 15ULL + 1;

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
		return i <= 1 ? 1 : i * factorial(i - 1);
	};
	size_t decrease = 0;
	for (size_t i = 0; i < enhanced_slot_quantity_list.size() - 1; i++)
	{
		auto enhanced_slot_quantity = enhanced_slot_quantity_list[i];
		auto enhanceable_slot_quantity = usable_slot_quantity - decrease;

		expected_assign_count *= factorial(enhanceable_slot_quantity) / factorial(enhanceable_slot_quantity - enhanced_slot_quantity) / factorial(enhanced_slot_quantity);
		decrease += enhanced_slot_quantity;
	}

	//std::cout << "EXPECTED : " << expected_combine_count << " * " << expected_assign_count << std::endl;

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

int main_in_try()
{
	evaluator_t evaluator(
		std::move(inputData.core_level_list),
		std::move(inputData.skill_value_list),
		std::vector(inputData.enhanced_slot_quantity_list));

	// #2는 원래 있던거고, 레벨이 적용된 코어의 가치를 판단하기 위해 만들어졌음
	// #1은 #2를 복사한거고, 레벨이 아니라 슬롯강화를 적용한 후의 가치를 판단하기 위해 만들어졌음

	// #2
	// core_style_grid에 대해 같은 위치에 있는 core_style의 value데이터 저장
	std::vector<std::vector<value_t>> core_value_grid;
	core_value_grid.reserve(inputData.core_style_grid.size());
	for (auto& core_style_list : inputData.core_style_grid)
	{
		std::vector<value_t> core_value_list;
		core_value_list.reserve(core_style_list.size());
		for (auto& core_style : core_style_list)
			core_value_list.emplace_back(evaluator(core_style));
		core_value_grid.emplace_back(std::move(core_value_list));
	}

	// #2
	// core_value_grid에 대해 각 core_value_list의 값중 가장 큰 value값을 저장
	std::vector<value_t> core_maximum_value_list;
	core_maximum_value_list.reserve(core_value_grid.size());
	for (auto& core_value_list : core_value_grid)
	{
		auto max_iter = std::max_element(core_value_list.begin(), core_value_list.end());
		core_maximum_value_list.emplace_back(max_iter == core_value_list.end() ? 0 : *max_iter);
	}

	// #2
	// begin_core_id부터 remain_slot_quantity로 낼 수 있는 가장 큰 value값을 저장
	std::vector<std::vector<value_t>> core_combination_expected_maximum_value_table;
	core_combination_expected_maximum_value_table.reserve(core_value_grid.size());
	for (skill_id_t begin_skill_id = 0; begin_skill_id < core_value_grid.size(); begin_skill_id++)
	{
		std::vector<value_t> part_of_core_maximum_value_list;
		part_of_core_maximum_value_list.assign(std::next(core_maximum_value_list.begin(), begin_skill_id), core_maximum_value_list.end());
		std::sort(part_of_core_maximum_value_list.begin(), part_of_core_maximum_value_list.end(), std::greater<value_t>());

		std::vector<value_t> core_combination_expected_maximum_value_list;
		core_combination_expected_maximum_value_list.reserve(part_of_core_maximum_value_list.size());
		value_t total_expected_maximum_value = 0;
		for (value_t expected_core_maximum_value : part_of_core_maximum_value_list)
		{
			if (expected_core_maximum_value == 0)
				break;
			total_expected_maximum_value += expected_core_maximum_value;
			core_combination_expected_maximum_value_list.emplace_back(total_expected_maximum_value);
		}

		core_combination_expected_maximum_value_table.emplace_back(std::move(core_combination_expected_maximum_value_list));
	}
	// 를 반환
	auto get_core_combination_expected_maximum_value = [&core_combination_expected_maximum_value_table](skill_id_t begin_skill_id, size_t remain_slot_quantity)->value_t
	{
		const auto& expected_maximum_value_list = core_combination_expected_maximum_value_table[begin_skill_id];
		if (remain_slot_quantity == 0 || expected_maximum_value_list.size() == 0)
			return 0;

		if (remain_slot_quantity > expected_maximum_value_list.size())
			remain_slot_quantity = expected_maximum_value_list.size();
		size_t index = remain_slot_quantity - 1;
		return expected_maximum_value_list[index];
	};


	// #1
	// core_style_grid에 대해 같은 위치에 있는 core_style의 순수 value 데이터 저장
	std::vector<std::vector<value_t>> core_style_value_grid;
	core_style_value_grid.reserve(inputData.core_style_grid.size());
	for (auto& core_style_list : inputData.core_style_grid)
	{
		std::vector<value_t> core_value_list;
		core_value_list.reserve(core_style_list.size());
		for (auto& core_style : core_style_list)
			core_value_list.push_back(evaluator.of_core_style(core_style));
		core_style_value_grid.emplace_back(std::move(core_value_list));
	}

	// #1
	// core_style_value_grid에 대해 각 core_style_value_list의 값중 가장 큰 value값을 저장
	std::vector<value_t> core_style_maximum_value_list;
	core_style_maximum_value_list.reserve(core_style_value_grid.size());
	for (auto& core_style_value_list : core_style_value_grid)
	{
		auto max_iter = std::max_element(core_style_value_list.begin(), core_style_value_list.end());
		core_style_maximum_value_list.emplace_back(max_iter == core_style_value_list.end() ? 0 : *max_iter);
	}

	// #1 but original(not copied from #2)
	// begin_core_id에서부터 각 core_style_value를 높은 순서대로 정렬하여 저장
	std::vector<std::vector<value_t>> sorted_enhanceable_core_style_value_grid;
	sorted_enhanceable_core_style_value_grid.reserve(core_style_value_grid.size());
	for (skill_id_t begin_skill_id = 0; begin_skill_id < core_style_value_grid.size(); begin_skill_id++)
	{
		std::vector<value_t> part_of_core_style_maximum_value_list;
		part_of_core_style_maximum_value_list.assign(std::next(core_style_maximum_value_list.begin(), begin_skill_id), core_style_maximum_value_list.end());
		std::sort(part_of_core_style_maximum_value_list.begin(), part_of_core_style_maximum_value_list.end(), std::greater<value_t>());

		sorted_enhanceable_core_style_value_grid.emplace_back(std::move(part_of_core_style_maximum_value_list));
	}

	// #1 but original(not copied from #2)
	// 
	auto get_slot_quantity_expected_maximum_enhancement_value = [&sorted_enhanceable_core_style_value_grid](skill_id_t begin_skill_id, size_t remain_slot_quantity)->value_t
	{
		const std::vector<size_t>& enhanced_slot_quantity_list = inputData.enhanced_slot_quantity_list;

		const auto& sorted_enhanceable_core_style_value_list = sorted_enhanceable_core_style_value_grid[begin_skill_id];
		if (remain_slot_quantity == 0 || sorted_enhanceable_core_style_value_list.size() == 0)
			return 0;

		size_t slot_index = 0;
		value_t total_expected_maximum_enhancement_value = 0;
		const size_t enhancement_maximum = enhanced_slot_quantity_list.size() - 1;
		for (size_t enhancement = enhancement_maximum; enhancement > 0; enhancement--)
		{
			size_t local_enhanced_slot_quantity = enhanced_slot_quantity_list[enhancement];
			for (size_t i = 0; i < local_enhanced_slot_quantity; i++)
			{
				if (slot_index >= remain_slot_quantity)
					return total_expected_maximum_enhancement_value;

				value_t core_style_value = sorted_enhanceable_core_style_value_list[slot_index];
				value_t enhanced_core_style_value = core_style_value * enhancement;
				total_expected_maximum_enhancement_value += enhanced_core_style_value;

				slot_index++;
			}
		}
		return total_expected_maximum_enhancement_value;
	};

	skill_quantity = inputData.skill_value_list.size();


	using core_style_combination_t = std::vector<core_style_t>;
	auto for_each_of_all_core_style_combination = [&evaluator, &get_core_combination_expected_maximum_value, &get_slot_quantity_expected_maximum_enhancement_value](const std::vector<std::vector<core_style_t>>& core_style_grid, size_t slot_quantity)-> std::pair<core_style_combination_t, std::vector<size_t>>
	{
		core_style_combination_t core_style_combination;
		core_style_combination.reserve(slot_quantity);

		size_t best_value = 0;
		core_style_combination_t best_combination;
		std::vector<size_t> best_enhanced_slot_case;

		size_t __dev_callcount = 0;

		std::function<void(skill_id_t, size_t)> simulate_core_combination;
		simulate_core_combination = [&](skill_id_t skill_id, size_t empty_core_slot_quantity)
		{
			if (skill_id >= inputData.core_style_grid.size())
				return;

			__dev_callcount++;
			//------------------------------------------------------------BEGIN

			//---------- LOGIC PHASE 1
			auto [value_until_now, enhanced_slot_case] = evaluator(core_style_combination);

			// Q. 기존 가치를 뛰어 넘었는가?
			if (best_value < value_until_now)
			{
				// 갱신
				best_value = value_until_now;
				best_combination = core_style_combination;
				best_enhanced_slot_case = enhanced_slot_case;

				if (false)
				{
					std::cout << best_value << "\n";
					for (const core_style_t& core_style : best_combination)
						std::cout << "\t" << core_style << "\n";
					for (auto v : best_enhanced_slot_case)
						std::cout << v << " ";
					std::cout << "\n";
				}
			}


			//---------- LOGIC PHASE 2
			// 슬롯 확인
			if (empty_core_slot_quantity == 0)
				return;

			value_t expected_future_value = get_core_combination_expected_maximum_value(skill_id, empty_core_slot_quantity);
			value_t expected_future_enhancement_value = get_slot_quantity_expected_maximum_enhancement_value(skill_id, empty_core_slot_quantity);

			value_t expected_future_total_value = expected_future_value + expected_future_enhancement_value;

			if (false)
			{
				std::cout << std::format("{} ({}, {}) : \n",
					value_until_now + expected_future_total_value, value_until_now, expected_future_total_value
				);
				for (const core_style_t& core_style : core_style_combination)
					std::cout << "\t" << core_style << "\n";
				for (auto v : enhanced_slot_case)
					std::cout << v << " ";
				std::cout << "\n";
			}


			// Q. N부터 M개를 선택해서 가치가 더 높을 수 있는가?
			if (false == (best_value < value_until_now + expected_future_total_value))
				// 기대 가치에 미달 시 진행 중단
				return;

			// Q. N번째를 확정하고, N + 1부터 M - 1개를 선택해서 가치가 더 높을 수 있는가?
			for (const core_style_t& core_style : core_style_grid[skill_id])
			{
				if (skill_id == 0)
					int a = 5;

				// 본인을 포함하고 시뮬레이션 지속
				core_style_combination.push_back(core_style);
				simulate_core_combination(skill_id + 1, empty_core_slot_quantity - 1);
				core_style_combination.pop_back();
			}

			// Q. 조건 불필요
			if (true)
				// 본인을 제외하고 시뮬레이션 지속
				simulate_core_combination(skill_id + 1, empty_core_slot_quantity);

			//------------------------------------------------------------END

		};
		//---
		// 0번 부터 시작
		skill_id_t begin_skill_id = 0;
		simulate_core_combination(begin_skill_id, slot_quantity);
		//---
		
		//std::cout << "Count of trials : " << __dev_callcount;

		return std::make_pair(best_combination, best_enhanced_slot_case);
	};

	std::cout << "Start calculation..." << std::endl;
	auto [best_combination, best_enhanced_slot_case] = for_each_of_all_core_style_combination(inputData.core_style_grid, inputData.usable_slot_quantity);
	std::cout << "Calculation finished!" << std::endl;

	std::cout << "Serializing data..." << std::endl;
	std::stringstream sstream;
	{
		for (skill_id_t skill_id = 0; skill_id < best_combination.size(); skill_id++)
		{
			size_t first = best_combination[skill_id].get_first() + 1;
			size_t second = best_combination[skill_id].get_second() + 1;
			size_t third = best_combination[skill_id].get_third() + 1;
			if (second >= 15ULL) second = 0;
			if (third >= 15ULL) third = 0;

			size_t enhancement = best_enhanced_slot_case[skill_id];
			sstream << first << "\t" << second << "\t" << third << "\t" << enhancement << "\n";
		}
	}
	std::cout << "Data serialized!" << std::endl;

	std::cout << "Try data save in file. (" << output_file_name << ")" << std::endl;
	std::fstream file_stream(output_file_name, std::fstream::out | std::fstream::trunc);
	if (file_stream)
	{
		for (skill_id_t skill_id = 0; skill_id < best_combination.size(); skill_id++)
		{
			size_t first = best_combination[skill_id].get_first() + 1;
			size_t second = best_combination[skill_id].get_second() + 1;
			size_t third = best_combination[skill_id].get_third() + 1;
			if (second >= 15ULL) second = 0;
			if (third >= 15ULL) third = 0;

			size_t enhancement = best_enhanced_slot_case[skill_id];
			file_stream << first << "\t" << second << "\t" << third << "\t" << enhancement << "\n";
		}

		std::cout << "Data saved in file! (" << output_file_name << ")" << std::endl;
	}
	else
		std::cout << "Save failed." << std::endl;
	return 0;
}