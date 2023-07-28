#pragma once
#include <set>

template<typename value_t, typename comparator = std::less<value_t>>
class double_priority_queue
{
private:
	using inner_container_t = std::multiset<value_t, comparator>;
public:
	using iterator_t = inner_container_t::iterator;
	using const_iterator_t = inner_container_t::const_iterator;

public:
	double_priority_queue() = default;
	~double_priority_queue() = default;

public:
	bool empty() const;
	size_t size() const;

	const value_t& top() const;
	const value_t& bottom() const;

	void push(const value_t& value);
	void push(value_t&& value);

	void pop_top();
	void pop_bottom();

	inline iterator_t begin() { return _data_set.begin(); }
	inline iterator_t end() { return _data_set.end(); }
	inline const_iterator_t cbegin() const { return _data_set.cbegin(); }
	inline const_iterator_t cend() const { return _data_set.cend(); }

private:
	inner_container_t _data_set;
};

template<typename value_t, typename comparator>
inline bool double_priority_queue<value_t, comparator>::empty() const
{
	return _data_set.empty();
}

template<typename value_t, typename comparator>
inline size_t double_priority_queue<value_t, comparator>::size() const
{
	return _data_set.size();
}

template<typename value_t, typename comparator>
inline const value_t& double_priority_queue<value_t, comparator>::top() const
{
	return (*_data_set.begin());
}

template<typename value_t, typename comparator>
inline const value_t& double_priority_queue<value_t, comparator>::bottom() const
{
	return (*_data_set.rbegin());
}

template<typename value_t, typename comparator>
inline void double_priority_queue<value_t, comparator>::push(const value_t& value)
{
	_data_set.insert(value);
}

template<typename value_t, typename comparator>
inline void double_priority_queue<value_t, comparator>::push(value_t&& value)
{
	_data_set.insert(std::move(value));
}

template<typename value_t, typename comparator>
inline void double_priority_queue<value_t, comparator>::pop_top()
{
	_data_set.erase(_data_set.begin());
}

template<typename value_t, typename comparator>
inline void double_priority_queue<value_t, comparator>::pop_bottom()
{
	_data_set.erase(std::prev(_data_set.end(), 1));
}
