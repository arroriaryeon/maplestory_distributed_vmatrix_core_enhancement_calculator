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
#include <array>
#include <tuple>

const std::filesystem::path input_file_name("./input.txt");
const std::filesystem::path output_file_name("./output.txt");

void desynchronize_io()
{
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(static_cast<std::ostream*>(nullptr));
	std::cerr.tie(static_cast<std::ostream*>(nullptr));
}

#include "core_style.h"

using skill_id_t = size_t;
using level_t = size_t;
using value_t = size_t;

constexpr level_t max_skill_level_of_core = 50;
constexpr level_t max_skill_level_of_slot = 60;

value_t evaluate_core_style(const core_style_t& core_style, const std::vector<value_t>& skill_value_list)
{
	return
		skill_value_list[core_style.get_first()] +
		skill_value_list[core_style.get_second()] +
		skill_value_list[core_style.get_third()];
}

value_t evaluate_core_style_with_level(const core_style_t& core_style, const std::vector<value_t>& skill_value_list, const std::vector<level_t>& core_level_list)
{
	return
		core_level_list[core_style.get_first()] * skill_value_list[core_style.get_first()] +
		core_level_list[core_style.get_first()] * skill_value_list[core_style.get_second()] +
		core_level_list[core_style.get_first()] * skill_value_list[core_style.get_third()];
}

value_t evaluate_core_combination(
	const std::vector<core_style_t>& core_style_combination,
	const std::vector<value_t>& skill_value_list,
	const std::vector<level_t>& core_level_list)
{
	skill_id_t highest_skill_id = 0;
	for (const core_style_t& core_style : core_style_combination)
		highest_skill_id = std::max({ highest_skill_id, core_style.get_first(), core_style.get_second(), core_style.get_third() });
	
	std::vector<level_t> skill_level_list(highest_skill_id + 1, 0);
	for (const core_style_t& core_style : core_style_combination)
	{
		skill_id_t first_id = core_style.get_first();
		skill_id_t second_id = core_style.get_second();
		skill_id_t third_id = core_style.get_third();
		level_t core_level = core_level_list[first_id];
		skill_level_list[first_id] += core_level;
		skill_level_list[second_id] += core_level;
		skill_level_list[third_id] += core_level;
	}

	value_t total_value = 0;
	for (skill_id_t skill_id = 0; skill_id < skill_level_list.size(); skill_id++)
	{
		value_t skill_value = skill_value_list[skill_id];
		level_t skill_level = skill_level_list[skill_id];
		level_t limited_skill_level = std::min(skill_level, max_skill_level_of_core);
		total_value += limited_skill_level * skill_value;
	}
	return total_value;
}



std::vector<std::pair<core_style_t, size_t>> get_best_vmatrix_assignment(
	const std::vector<std::vector<core_style_t>>& core_style_grid,
	const std::vector<level_t>& core_level_list,
	const std::vector<value_t>& skill_value_list,
	const std::vector<size_t>& slot_quantity_list);

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
		auto is_core_style_virtually_less = [](const core_style_t& l, const core_style_t& r) { return l.is_virtually_less(r); };
		auto is_core_style_virtually_equal = [](const core_style_t& l, const core_style_t& r)->bool { return l.is_virtually_equal(r); };

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

		//inputData.usable_slot_quantity = usable_slot_quantity;
		//inputData.enhanced_slot_quantity_list = std::move(enhanced_slot_quantity_list);
		//inputData.core_level_list = std::move(core_level_list);
		//inputData.skill_value_list = std::move(skill_value_list);
		//inputData.core_style_grid = std::move(core_style_grid);
		//main_in_try();

	// 없는게 디버깅하기 더 편한듯
	try
	{
		auto result = get_best_vmatrix_assignment(core_style_grid, core_level_list, skill_value_list, enhanced_slot_quantity_list);
		int a = 5321312;
	}
	catch (const std::exception& exception)
	{
		std::cout << "EXCEPTION : " << exception.what() << std::endl;
	}
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////


std::vector<std::pair<core_style_t, size_t>> get_best_vmatrix_assignment(
	const std::vector<std::vector<core_style_t>>& core_style_grid,
	const std::vector<level_t>& core_level_list,
	const std::vector<value_t>& skill_value_list,
	const std::vector<size_t>& slot_quantity_list)
{
	const size_t total_slot_quantity(std::accumulate(slot_quantity_list.begin(), slot_quantity_list.end(), 0));
	const size_t total_enhanced_slot_quantity(total_slot_quantity - slot_quantity_list.front());

	// core_combination의 최대 가치 및 최적의 강화 슬롯 반환
	auto evalute_core_combination_with_optimal_enhancement = [&skill_value_list, &core_level_list, slot_quantity_list](const std::vector<core_style_t>& core_combination)->std::pair<value_t, std::vector<size_t>>
	{
		size_t skill_quantity = 0;
		{
			skill_id_t max_skill_index = 0;
			for (const core_style_t& core_style : core_combination)
				max_skill_index = std::max({ max_skill_index, core_style.get_first(), core_style.get_second(), core_style.get_third() });
			skill_quantity = max_skill_index + 1;
		}

		// core의 가치에 따라 정렬 한 뒤, 기존 core_combination의 순서대로 되돌리기 위해 특수 객체를 사용할 필요가 있었음
		struct core_info_t
		{
			core_info_t(const core_style_t* core_style_p, size_t origin_index, value_t unit_value, value_t level_value) : core_style_p(core_style_p), origin_index(origin_index), unit_value(unit_value), level_value(level_value) {}
			const core_style_t& get_core_style() const { return *core_style_p; }

			const core_style_t* core_style_p;
			size_t origin_index;
			value_t unit_value;
			value_t level_value;
		};
		// core_style의 기준 가치가 높은 순서대로 정렬한 특수 객체 리스트 생성
		static std::vector<core_info_t> ordered_core_info_list;
		ordered_core_info_list.clear();
		ordered_core_info_list.reserve(core_combination.size());
		for (size_t i = 0; i < core_combination.size(); i++)
		{
			const core_style_t& core_style = core_combination[i];
			value_t core_unit_value = evaluate_core_style(core_style, skill_value_list);
			value_t core_level_value = evaluate_core_style_with_level(core_style, skill_value_list, core_level_list);
			ordered_core_info_list.emplace_back(&core_style, i, core_unit_value, core_level_value);
		}
		std::sort(ordered_core_info_list.begin(), ordered_core_info_list.end(), [](const core_info_t& l, const core_info_t& r)->bool
			{
				return l.unit_value < r.unit_value;
			});

		// 이하 로직에서 core_combination매개변수를 의도치 않게 사용하지 않도록 주의가 필요함

		auto get_expected_maximum_value_of_unused_slot = [](const std::vector<size_t>& remaining_slot_enhancement_list, size_t slot_index)->value_t
		{
			size_t core_style_index = slot_index;
			value_t total_value = 0;
			for (size_t enhancement_level = remaining_slot_enhancement_list.size() - 1; enhancement_level > 0; enhancement_level--)
			{
				size_t usable_slot_quantity = remaining_slot_enhancement_list[enhancement_level];
				for (size_t i = 0; i < usable_slot_quantity; i++)
				{
					if (slot_index == remaining_slot_enhancement_list.size() ||
						core_style_index == ordered_core_info_list.size())
						goto end_loop;

					value_t core_level_value = ordered_core_info_list[core_style_index].level_value;
					value_t core_enhancement_value = ordered_core_info_list[core_style_index].unit_value * enhancement_level;
					total_value += core_level_value + core_enhancement_value;

					core_style_index++;
				}
			}
		end_loop:
			return total_value;
		};

		auto get_enhanced_core_combination_value = [&skill_quantity, &core_level_list, &skill_value_list](const std::vector<level_t>& slot_enhancement_list)->value_t
		{
			static std::vector<level_t> skill_level_list;
			static std::vector<level_t> skill_enhancement_list;
			skill_level_list.clear();
			skill_level_list.resize(skill_quantity, 0);
			skill_enhancement_list.clear();
			skill_enhancement_list.resize(skill_quantity, 0);

			for (size_t core_index = 0; core_index < slot_enhancement_list.size(); core_index++)
			{
				const core_style_t& core_style = ordered_core_info_list[core_index].get_core_style();
				const level_t& core_level = core_level_list[core_style.get_first()];
				const level_t& enhancement_level = slot_enhancement_list[core_index];

				skill_level_list[core_style.get_first()] += core_level;
				skill_level_list[core_style.get_second()] += core_level;
				skill_level_list[core_style.get_third()] += core_level;
				skill_enhancement_list[core_style.get_first()] += enhancement_level;
				skill_enhancement_list[core_style.get_second()] += enhancement_level;
				skill_enhancement_list[core_style.get_third()] += enhancement_level;
			}

			value_t total_value = 0;
			for (size_t core_index = 0; core_index < skill_quantity; core_index++)
			{
				level_t skill_level = skill_level_list[core_index];
				level_t skill_enhancement = skill_enhancement_list[core_index];
				level_t result_level = std::min(max_skill_level_of_slot, std::min(max_skill_level_of_core, skill_level) + skill_enhancement);

				value_t skill_unit_value = skill_value_list[core_index];
				value_t result_skill_value = skill_unit_value * result_level;
				total_value += result_skill_value;
			}
			return total_value;
		};

		///// LOGIC
		level_t best_value = 0;
		level_t best_used_total_level = 0;
		std::vector<level_t> best_slot_enhancement_list;
		{
			level_t used_total_level = 0;
			std::vector<size_t> usable_slot_quantity_list = slot_quantity_list;

			const size_t slot_limit = ordered_core_info_list.size();
			std::vector<level_t> slot_enhancement_list;
			slot_enhancement_list.reserve(slot_limit);

			std::function<void()> simulate_slot_enhancement;
			simulate_slot_enhancement = [&]()
			{
				size_t slot_index = slot_enhancement_list.size();

				value_t until_now_value = get_enhanced_core_combination_value(slot_enhancement_list);

				// Q. 이미 모든 슬롯에 강화 수치가 부여되었는가?
				if (slot_index == slot_limit)
				{
					// Q. 기존 가치를 뛰어 넘었는가?
					if (best_value < until_now_value ||
						(best_value == until_now_value && best_used_total_level > used_total_level))
					{
						best_value = until_now_value;
						best_used_total_level = used_total_level;
						best_slot_enhancement_list = slot_enhancement_list;
					}
					return;
				}

				// Q. 더이상 슬롯 강화를 진행해도 가치가 높을 수 없는가?
				{
					value_t expected_feature_value = get_expected_maximum_value_of_unused_slot(usable_slot_quantity_list, slot_index);
					if (until_now_value + expected_feature_value < best_value)
						return;
				}

				// 현재 슬롯에 강화 수치 부여 및 진행
				for (size_t enhancement_level = 0; enhancement_level < usable_slot_quantity_list.size(); enhancement_level++)
				{
					size_t& usable_slot_quantity = usable_slot_quantity_list[enhancement_level];
					if (usable_slot_quantity == 0)
						continue;

					usable_slot_quantity++;
					used_total_level += enhancement_level;
					slot_enhancement_list.push_back(enhancement_level);
					{
						// 다음 슬롯에서 반복
						simulate_slot_enhancement();
					}
					slot_enhancement_list.pop_back();
					used_total_level -= enhancement_level;
					usable_slot_quantity--;
				}
			};
			simulate_slot_enhancement();
		}
		///// LOGIC

		// 정렬되기 전 순서로 수정해 줘야 함
		std::vector<level_t> result_slot_enhancement_list;
		result_slot_enhancement_list.resize(best_slot_enhancement_list.size());
		for (size_t index = 0; index < ordered_core_info_list.size(); index++)
		{
			size_t origin_index = ordered_core_info_list[index].origin_index;
			const auto& src = best_slot_enhancement_list[index];
			auto& dest = result_slot_enhancement_list[origin_index];
			dest = src;
		}
		return std::make_pair(best_value, result_slot_enhancement_list);
	};
	


	//////////////////////////////////////////////////
	// 가치 예상 관련

	std::vector<value_t> maximum_value_of_each_core_type;
	maximum_value_of_each_core_type.reserve(core_style_grid.size());
	for (const std::vector<core_style_t>& core_style_list : core_style_grid)
	{
		value_t maximum_value = 0;
		for (const core_style_t& core_style : core_style_list)
		{
			value_t current_value = evaluate_core_style(core_style, skill_value_list);
			maximum_value = std::max(maximum_value, current_value);
		}
		maximum_value_of_each_core_type.push_back(maximum_value);
	}

	std::vector<std::vector<value_t>> expected_maximum_core_combination_value_table; // Format : table[start_skill_id][remaining_slot_quantity - 1]
	expected_maximum_core_combination_value_table.reserve(maximum_value_of_each_core_type.size());
	{
		// core_unit_value to core_level_value
		std::vector<value_t> max_level_value_of_each_core_type = maximum_value_of_each_core_type;
		for (skill_id_t skill_id = 0; skill_id < max_level_value_of_each_core_type.size(); skill_id++)
			max_level_value_of_each_core_type[skill_id] *= core_level_list[skill_id];

		std::vector<value_t> part_of_maximum_value_list;
		part_of_maximum_value_list.reserve(max_level_value_of_each_core_type.size());
		for (auto iter = max_level_value_of_each_core_type.begin(); iter != max_level_value_of_each_core_type.end(); iter++)
		{
			part_of_maximum_value_list.assign(iter, max_level_value_of_each_core_type.end());
			std::sort(part_of_maximum_value_list.begin(), part_of_maximum_value_list.end(), std::greater<value_t>());

			std::vector<value_t> remaining_slot_expected_value_list;
			remaining_slot_expected_value_list.reserve(part_of_maximum_value_list.size());

			value_t accumulated_value = 0;
			for (value_t maximum_value_of_core_type : part_of_maximum_value_list)
			{
				accumulated_value += maximum_value_of_core_type;
				remaining_slot_expected_value_list.push_back(accumulated_value);
			}
			expected_maximum_core_combination_value_table.emplace_back(std::move(remaining_slot_expected_value_list));
		}
	}

	auto get_maximum_expected_value_of_remaining_slot_quantity = [&expected_maximum_core_combination_value_table](size_t remaining_slot_quantity, skill_id_t starting_skill_id)->value_t {
		const auto& maximum_value_list = expected_maximum_core_combination_value_table[starting_skill_id];
		if (remaining_slot_quantity == 0 || maximum_value_list.empty())
			return 0;

		const size_t index = std::min(remaining_slot_quantity, maximum_value_list.size()) - 1;
		return maximum_value_list[index];
	};

	std::vector<size_t> ordered_maximum_value_of_each_core_type(maximum_value_of_each_core_type.begin(), maximum_value_of_each_core_type.end());
	std::sort(ordered_maximum_value_of_each_core_type.begin(), ordered_maximum_value_of_each_core_type.end(), std::greater<size_t>());

	auto get_maximum_expected_value_of_remaining_enhancement_slot_quantity = [&ordered_maximum_value_of_each_core_type](const std::vector<size_t>& enhancement_slot_quantity_list, skill_id_t starting_skill_id)->value_t
	{
		if (enhancement_slot_quantity_list.size() == 0 ||
			ordered_maximum_value_of_each_core_type.size() == 0)
			return 0;

		value_t total_value = 0;
		auto iter_a = ordered_maximum_value_of_each_core_type.begin();
		auto iter_a_end = ordered_maximum_value_of_each_core_type.end();

		for (size_t enhancement_level = enhancement_slot_quantity_list.size() - 1; enhancement_level > 0; enhancement_level--)
		{
			size_t this_enhancement_level_slot_quantity = enhancement_slot_quantity_list[enhancement_level];
			for (size_t i = 0; i < this_enhancement_level_slot_quantity; i++)
			{
				value_t core_style_value = (*iter_a);
				value_t enhanced_core_value = core_style_value * enhancement_level;

				total_value += enhanced_core_value;
				iter_a++;

				if (iter_a == iter_a_end)
					goto escape_loop;
			}
		}
	escape_loop:
		return total_value;
	};



	std::vector<core_style_t> core_style_combination;
	core_style_combination.reserve(total_slot_quantity);

	size_t best_value = 0;
	std::vector<core_style_t> best_combination;
	std::vector<size_t> best_enhanced_slot_case;

	std::function<void(skill_id_t, size_t)> simulate_core_combination;
	simulate_core_combination = [&](skill_id_t skill_id, size_t empty_core_slot_quantity)
	{
		if (skill_id >= core_style_grid.size())
			return;

		//---------- LOGIC PHASE 1 : 현재 가치 비교
		auto [value_until_now, enhanced_slot_case] = evalute_core_combination_with_optimal_enhancement(core_style_combination);


		auto check = [&](size_t core_id, auto f, auto s, auto t)->bool
		{
			return
				core_style_combination.size() > core_id &&
				core_style_combination[core_id].get_first() == f &&
				core_style_combination[core_id].get_second() == s &&
				core_style_combination[core_id].get_third() == t;
		};

		if (check(0, 0, 1, 3))
		{
			int asdf = 5;
			if (check(1, 1, 2, 4))
			{
				int asdf = 5;
				if (check(2, 2, 0, 5))
				{
					int asdf = 5;
					if (check(3, 3, 4, 5))
					{
						int asdf = 5;
						if (check(4, 4, 3, 5))
						{
							int asdf = 5;
							if (check(5, 4, 3, 4))
							{
								int asdf = 5;
							}
						}
					}
				}
			}
		}

		// Q. 기존 가치를 뛰어 넘었는가?
		if (best_value < value_until_now)
		{
			// 갱신
			best_value = value_until_now;
			best_combination = core_style_combination;
			best_enhanced_slot_case = enhanced_slot_case;

			// DEBUG
			if constexpr (true)
			{
				std::cout << best_value << "\n";
				for (const core_style_t& core_style : best_combination)
					std::cout << "\t" << core_style << "\n";
				for (auto v : best_enhanced_slot_case)
					std::cout << v << " ";
				std::cout << "\n";
			}
		}

		//---------- LOGIC PHASE 2 : 미래 예상 가치 비교 후 실제 가치 평가로의 진행
		// Q. 코어를 장착할 칸이 남아 있는가?
		if (empty_core_slot_quantity == 0)
			return;

		value_t expected_maximum_future_value = get_maximum_expected_value_of_remaining_slot_quantity(empty_core_slot_quantity, skill_id);
		value_t expected_maximum_future_enhancement_value = get_maximum_expected_value_of_remaining_enhancement_slot_quantity(slot_quantity_list, skill_id);
		value_t expected_maximum_future_total_value = expected_maximum_future_value + expected_maximum_future_enhancement_value;

		// Q. N부터 M개를 선택해서 가치가 더 높을 수 있는가?
		//		== 시뮬레이션을 지속할 경우 더 높은 가치가 나올 수 있는가?
		if (false == (best_value < value_until_now + expected_maximum_future_total_value))
			// 기대 가치에 미달 시 진행 중단
			return;

		// 이 코어를 포함하고 시뮬레이션 지속
		for (const core_style_t& core_style : core_style_grid[skill_id])
		{
			core_style_combination.push_back(core_style);
			simulate_core_combination(skill_id + 1, empty_core_slot_quantity - 1);
			core_style_combination.pop_back();
		}
		
		// 이 코어를 제외하고 시뮬레이션 지속
		simulate_core_combination(skill_id + 1, empty_core_slot_quantity);
	};

	skill_id_t begin_skill_id = 0;
	simulate_core_combination(begin_skill_id, total_slot_quantity);

	// 결과 가공
	std::vector<std::pair<core_style_t, value_t>> result;
	result.reserve(best_combination.size());
	for (size_t i = 0; i < best_combination.size(); i++)
		result.push_back(std::make_pair(best_combination[i], best_enhanced_slot_case[i]));
	return result;
};