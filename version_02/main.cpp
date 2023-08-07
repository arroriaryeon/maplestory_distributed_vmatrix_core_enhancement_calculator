#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <exception>
#include <functional>
#include <numeric>

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

constexpr level_t skill_max_level_by_core = 50;
constexpr level_t skill_max_level_by_slot = 60;


value_t evaluete_core_style(const core_style_t& core_style, const std::vector<value_t>& skill_value_list)
{
	return
		skill_value_list[core_style.get_first()] +
		skill_value_list[core_style.get_second()] +
		skill_value_list[core_style.get_third()];
}


std::pair<value_t, std::vector<size_t>> calculate_optimal_slot_enhancement(
	const std::vector<level_t>& core_level_list,
	const std::vector<value_t>& skill_value_list,
	const std::vector<size_t>& enhanced_slot_quantity_list,
	const std::vector<core_style_t>& core_style_combination)
{
	auto for_each_of_all_enhanced_slot_assignment_order = [](std::vector<size_t> enhanced_slot_quantity_list, size_t assignable_slot_quantity, std::function<void(const std::vector<size_t>&)> enhanced_slot_assignment_order_event)
	{



		std::vector<size_t> enhanced_slot_assigned_order;
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

			for (size_t enhancement_level = 0; enhancement_level < enhanced_slot_quantity_list.size(); enhancement_level++)
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
	static std::vector<level_t> _skill_level_list;
	_skill_level_list.clear();
	_skill_level_list.resize(id_max + 1, 0);

	for (const core_style_t& core_style : core_style_combination)
	{
		skill_id_t first_id = core_style.get_first();
		skill_id_t second_id = core_style.get_second();
		skill_id_t third_id = core_style.get_third();
		level_t core_level = core_level_list[first_id];

		_skill_level_list[first_id] += core_level;
		_skill_level_list[second_id] += core_level;
		_skill_level_list[third_id] += core_level;
	}

	for (skill_id_t skill_id = 0; skill_id < _skill_level_list.size(); skill_id++)
	{
		level_t& skill_level = _skill_level_list[skill_id];
		skill_level = std::min(skill_level, skill_max_level_by_core);
	}

	value_t best_total_value = 0;
	std::vector<size_t> best_slot_enhancement_assignment;
	for_each_of_all_enhanced_slot_assignment_order(enhanced_slot_quantity_list, core_style_combination.size(), [&](const std::vector<size_t>& slot_enhancement_assignment)
		{
			static std::vector<level_t> _enhanced_skill_level_list;
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
				value_t skill_value = skill_value_list[skill_id];
				level_t skill_level = _enhanced_skill_level_list[skill_id];
				level_t limited_skill_level = std::min(skill_level, skill_max_level_by_slot);
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





struct Input_t
{
	size_t usable_slot_quantity;
	std::vector<size_t> enhanced_slot_quantity_list;
	std::vector<size_t> core_level_list;
	std::vector<size_t> skill_value_list;
	std::vector<std::vector<core_style_t>> core_style_grid;
} inputData;

std::pair<std::vector<core_style_t>, std::vector<size_t>> main_in_try();
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
				excel_list.push_back(std::stoull(value));
		}

		if (!excel_list.empty())
			excel_grid.emplace_back(std::move(excel_list));
	}

	if (excel_grid.size() < 3)
	{
		std::cout << "no core\n";
		return 0;
	}
	fileStream.close();

	// excel to unprocessed input data
	auto enhanced_slot_quantity_list = std::move(excel_grid[0]);
	auto core_level_list = std::move(excel_grid[1]);
	auto skill_value_list = std::move(excel_grid[2]);
	std::vector<std::vector<size_t>> raw_core_style_list;
	raw_core_style_list.insert(raw_core_style_list.end(), std::next(excel_grid.begin(), 3), excel_grid.end());


	//// input data optimizing
	//while (enhanced_slot_quantity_list.size() && enhanced_slot_quantity_list.back() == 0)
	//	enhanced_slot_quantity_list.pop_back();
	//size_t usable_slot_quantity = std::accumulate(enhanced_slot_quantity_list.begin(), enhanced_slot_quantity_list.end(), 0);
	//
	//std::vector<bool> skill_id_used_list;
	//for (const std::vector<size_t>& raw_core_style : raw_core_style_list)
	//{
	//	if (raw_core_style[0] == 0)
	//		continue;
	//
	//	size_t maximum_raw_core_id = *std::max_element(raw_core_style.begin(), raw_core_style.end());
	//	if (skill_id_used_list.size() < maximum_raw_core_id)
	//		skill_id_used_list.resize(maximum_raw_core_id);
	//	skill_id_used_list[maximum_raw_core_id - 1] = true;
	//}
	//
	//{
	//	size_t push_index = 0;
	//	for (size_t ref_index = 0; ref_index < skill_id_used_list.size(); ref_index++)
	//	{
	//		if (skill_id_used_list[ref_index])
	//		{
	//			core_level_list[push_index] = ref_index;
	//			skill_value_list[push_index] = ref_index;
	//			push_index++;
	//		}
	//	}
	//}

	size_t usable_slot_quantity = std::accumulate(enhanced_slot_quantity_list.begin(), enhanced_slot_quantity_list.end(), 0);
	core_level_list.resize(17, 0);
	skill_value_list.resize(17, 0);

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
			return l.is_virtually_less(r);
		};
		auto is_core_style_virtually_equal = [](const core_style_t& l, const core_style_t& r)
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

		std::cout << "Start calculation..." << std::endl;
		auto [best_combination, best_enhanced_slot_case] = main_in_try();
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
	}
	catch (const std::exception& exception)
	{
		std::cout << "EXCEPTION : " << exception.what() << std::endl;
	}


	// 리팩토링 및 최적화 과정에서의 로직 손상에 대비한 간단한 결과론적 유효성 검사
	{
		std::cout << "\n";
		std::fstream outFile(output_file_name);
		std::fstream integrityFile("output_example.txt");
		while (!integrityFile.eof() && !outFile.eof())
		{
			std::string iV, oV;
			integrityFile >> iV;
			outFile >> oV;
			if (iV != oV)
				throw std::exception("비유효");
		}
		if (!(integrityFile.eof() && outFile.eof()))
			throw std::exception("비유효");
		std::cout << "유효!" << "\n";
	}
}

std::pair<std::vector<core_style_t>, std::vector<size_t>> main_in_try()
{
	// 두 변수가 실제 의미하는 것은 결과적으로 동일함
	const size_t skill_quantity = inputData.core_style_grid.size();
	const size_t core_type_quantity = skill_quantity;

	std::vector<std::vector<value_t>> theoretical_max_core_combination_value_table; //이름지어줘//
	{
		theoretical_max_core_combination_value_table.reserve(core_type_quantity);

		// 각 코어 타입별 최대 "core" 가치
		std::vector<value_t> max_core_value_of_each_core_type;
		max_core_value_of_each_core_type.reserve(core_type_quantity);
		for (auto& core_style_list : inputData.core_style_grid)
		{
			value_t max_value = 0;
			for (auto& core_style : core_style_list)
			{
				value_t core_style_value = evaluete_core_style(core_style, inputData.skill_value_list);
				level_t core_level = inputData.core_level_list[core_style.get_first()];
				value_t core_value = core_style_value * core_level;
				max_value = std::max(max_value, core_value);
			}
			max_core_value_of_each_core_type.push_back(max_value);
		}

		std::vector<value_t> max_core_value_partition;
		max_core_value_partition.reserve(core_type_quantity);
		for (auto iter = max_core_value_of_each_core_type.begin(); iter != max_core_value_of_each_core_type.end(); iter++)
		{
			max_core_value_partition.assign(iter, max_core_value_of_each_core_type.end());
			std::sort(max_core_value_partition.begin(), max_core_value_partition.end(), std::greater<value_t>());

			std::vector<value_t> theoretical_max_value_list;
			theoretical_max_value_list.reserve(max_core_value_partition.size());

			value_t theoretical_max_value = 0;
			for (value_t core_value : max_core_value_partition)
			{
				if (core_value == 0)
					break;
				theoretical_max_value += core_value;
				theoretical_max_value_list.push_back(theoretical_max_value);
			}

			theoretical_max_core_combination_value_table.emplace_back(std::move(theoretical_max_value_list));
		}
	}
	auto get_theoretical_max_core_combination_value = [&theoretical_max_core_combination_value_table](skill_id_t begin_skill_id, size_t remain_slot_quantity)->value_t
	{
		const auto& theoretical_max_value_list = theoretical_max_core_combination_value_table[begin_skill_id];
		if (remain_slot_quantity == 0 || theoretical_max_value_list.empty())
			return 0;

		if (remain_slot_quantity > theoretical_max_value_list.size())
			remain_slot_quantity = theoretical_max_value_list.size();
		size_t index = remain_slot_quantity - 1;
		return theoretical_max_value_list[index];
	};


	// complicated variable
	std::vector<std::vector<value_t>> ordered_max_core_type_value_grid;
	{
		ordered_max_core_type_value_grid.reserve(core_type_quantity);

		// 각 코어 타입별 최대 "core_style" 가치
		std::vector<value_t> max_core_style_value_of_each_core_type;
		max_core_style_value_of_each_core_type.reserve(core_type_quantity);
		for (auto& core_style_list : inputData.core_style_grid)
		{
			value_t max_value = 0;
			for (auto& core_style : core_style_list)
			{
				value_t core_style_value = evaluete_core_style(core_style, inputData.skill_value_list);
				max_value = std::max(max_value, core_style_value);
			}
			max_core_style_value_of_each_core_type.push_back(max_value);
		}

		for (auto iter = max_core_style_value_of_each_core_type.begin(); iter != max_core_style_value_of_each_core_type.end(); iter++)
		{
			std::vector<value_t> max_core_style_value_partition;
			max_core_style_value_partition.assign(iter, max_core_style_value_of_each_core_type.end());
			std::sort(max_core_style_value_partition.begin(), max_core_style_value_partition.end(), std::greater<value_t>());
			
			ordered_max_core_type_value_grid.emplace_back(std::move(max_core_style_value_partition));
		}
	}
	auto get_theoretical_max_slot_enhancement_value = [&ordered_max_core_type_value_grid](skill_id_t begin_skill_id, size_t remaining_slot_quantity)->value_t
	{
		const std::vector<size_t>& enhanced_slot_quantity_list = inputData.enhanced_slot_quantity_list;

		const auto& sorted_core_style_value_list = ordered_max_core_type_value_grid[begin_skill_id];
		remaining_slot_quantity = std::min(remaining_slot_quantity, sorted_core_style_value_list.size());
		if (remaining_slot_quantity == 0)
			return 0;

		size_t slot_index = 0;
		value_t total_value = 0;
		const size_t enhancement_maximum = enhanced_slot_quantity_list.size() - 1;
		for (size_t enhancement = enhancement_maximum; enhancement > 0; enhancement--)
		{
			size_t local_enhanced_slot_quantity = enhanced_slot_quantity_list[enhancement];
			for (size_t i = 0; i < local_enhanced_slot_quantity; i++)
			{
				if (!(slot_index < remaining_slot_quantity))
					goto escape_loop;

				value_t core_style_value = sorted_core_style_value_list[slot_index];
				value_t enhancement_value = core_style_value * enhancement;
				total_value += enhancement_value;

				slot_index++;
			}
		}
		escape_loop:
		return total_value;
	};


	auto calculate_best_vmatrix = [skill_quantity, core_type_quantity, &get_theoretical_max_core_combination_value, &get_theoretical_max_slot_enhancement_value](const std::vector<std::vector<core_style_t>>& core_style_grid, size_t slot_quantity)-> std::pair<std::vector<core_style_t>, std::vector<size_t>>
	{
		std::vector<core_style_t> core_style_combination;
		core_style_combination.reserve(slot_quantity);

		size_t best_value = 0;
		std::vector<core_style_t> best_combination;
		std::vector<size_t> best_enhanced_slot_case;

		std::function<void(skill_id_t)> simulate_core_combination;
		simulate_core_combination = [&](skill_id_t skill_id)
		{
			// 장착된 코어 조합에 대해 최적화 된 슬롯 강화 방법과 그에 따른 가치 계산
			auto [value_until_now, enhanced_slot_case]
				= calculate_optimal_slot_enhancement(inputData.core_level_list,
					inputData.skill_value_list, 
					inputData.enhanced_slot_quantity_list, 
					core_style_combination);
			

			if (best_value < value_until_now)
			{
				best_value = value_until_now;
				best_combination = core_style_combination;
				best_enhanced_slot_case = enhanced_slot_case;

				if constexpr (false)
				{
					std::cout << best_value << "\n";
					for (const core_style_t& core_style : best_combination)
						std::cout << "\t" << core_style << "\n";
					for (auto v : best_enhanced_slot_case)
						std::cout << v << " ";
					std::cout << "\n";
				}
			}

			if (!(core_style_combination.size() < slot_quantity))
				return;
			const size_t empty_core_slot_quantity = slot_quantity - core_style_combination.size();

			// 더 이상 장착 가능한 코어 타입이 없는지 확인
			if (!(skill_id < core_type_quantity))
				return;

			// 남은 슬롯으로 만들어 낼 수 있는 이론상의 최대 가치 계산
			value_t theoretical_max_future_core_value = get_theoretical_max_core_combination_value(skill_id, empty_core_slot_quantity);
			value_t theoretical_max_future_slot_value = get_theoretical_max_slot_enhancement_value(skill_id, empty_core_slot_quantity);
			value_t theoretical_max_future_value = theoretical_max_future_core_value + theoretical_max_future_slot_value;

			if (value_until_now + theoretical_max_future_value <= best_value)
				return;

			// 현재 코어 타입의 코어를 장착한 각각의 시뮬레이션 진행
			for (const core_style_t& core_style : core_style_grid[skill_id])
			{
				// 본인을 포함하고 시뮬레이션 지속
				core_style_combination.push_back(core_style);
				simulate_core_combination(skill_id + 1);
				core_style_combination.pop_back();
			}

			// 현재 코어 타입의 코어를 장착하지 않은 시뮬레이션으로 진행
			simulate_core_combination(skill_id + 1);
		};

		const skill_id_t begin_skill_id = 0;
		simulate_core_combination(begin_skill_id);
		
		return std::make_pair(best_combination, best_enhanced_slot_case);
	};

	return calculate_best_vmatrix(inputData.core_style_grid, inputData.usable_slot_quantity);
}
