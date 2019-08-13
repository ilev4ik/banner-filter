#pragma once
#ifndef RAMBLER_BANNERS_HASH_BACK_INSERTER_H
#define RAMBLER_BANNERS_HASH_BACK_INSERTER_H

#include <iterator>

template <typename C>
class hash_back_insert_iterator {
protected:
    C* container;
public:
    typedef C                   container_type;
    typedef std::output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;
    using mapped_type = typename C::value_type::second_type::value_type;

    explicit hash_back_insert_iterator(C& __x): container(&__x) {}

    hash_back_insert_iterator<C>& operator=(const mapped_type& val) {
        (*container)[val.adv_id].push_back(val);
        return *this;
    }

    hash_back_insert_iterator<C>& operator*() {
        return *this;
    }

    hash_back_insert_iterator<C>& operator++() {
        return *this;
    }

    hash_back_insert_iterator<C>& operator++(int) {
        return *this;
    }
};

template <class Container>
hash_back_insert_iterator<Container> hash_back_inserter(Container& c)
{
    return hash_back_insert_iterator<Container>(c);
}

#endif //RAMBLER_BANNERS_HASH_BACK_INSERTER_H
