#pragma once
#ifndef RAMBLER_BANNERS_FILTER_H
#define RAMBLER_BANNERS_FILTER_H

#include <algorithm>
#include <functional>
#include <vector>

template <typename T>
class filter
{
public:
    using pred_type = std::function<bool(const T& obj)>;
    void add(pred_type&& p) {
        m_preds.emplace_back(std::move(p));
    }

    bool operator()(const T& obj) const {
        return apply(obj);
    }

    bool has_preds() const {
        return !m_preds.empty();
    }
private:
    bool apply(const T& obj) const
    {
        // проверка объекта на условия всех предикатов фильтра
        auto check_pred = [obj](const pred_type& pred) { return !pred(obj); };
        return std::all_of(m_preds.begin(), m_preds.end(), check_pred);
    }

private:
    std::vector<pred_type> m_preds;
};

#endif //RAMBLER_BANNERS_FILTER_H
