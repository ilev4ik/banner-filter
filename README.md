# Рекламный аукцион

[![Build Status](https://travis-ci.com/ilev4ik/banner-filter.svg?branch=master)](https://travis-ci.com/ilev4ik/banner-filter)
[![codecov](https://codecov.io/gh/ilev4ik/banner-filter/branch/master/graph/badge.svg)](https://codecov.io/gh/ilev4ik/banner-filter)


Реализована функция рекламного аукциона с дополнительными возможностями:

* добавление дополнительных фильтров
* добавление дополнительных полей для приоритезации баннеров

## Определение класса баннера
Класс `banner` должен удовлетворять опредённым условиям, чтобы работали доп. возможности.

 ```cpp
class banner : public eq_mixin<banner>
{
    using rank_t = int;
public:
    rank_t rank() const { return price; }
    friend class banner_traits<banner>;
    ... 
};

```

Необходимо объявить тип, используемый для приоретизации и объявить дружественным класс, который пользуется этой информацией.
Класс `banner_traits<>` (`banner_traits.h`) содержит в себе необходимые функторы, используемые в алгоритмах при проведении аукциона.

Чтобы добавить возможность приоретизации следует в классе кастомного баннера (например, `banner_ex`) определить тип`rank_t` и реализовать
 функцию `rank()`, возвращающую поля в порядке приоритета.
`rank_t` может быть объявлен как `std::tuple`, так как из коробки он имеет возможность лексикографического сравнения. В базовом варианте
`rank()` возвращается поле стоимости `price` по условию задачи. То есть есть возможность указать, что такое цена для баннера в более общих понятиях.

Никаких дополнительных действий для включения возможности фильтрации не требуется, так как предикаты фильтрации определяются динамически, например:
```cpp
filter<banner_t> banner_filter;
banner_filter.add([](const banner_t& b) -> bool
{
    return b.countries.empty() ? true : b.countries.count("russia");
});
```

В данном случае из исходной выборки баннеров останутся те, которые предназначены только для страны `"russia"`.

## Ход решения
Ход решения прямо следует из реализации метода `lyciator::run_auction`:
1) Проверка на количество лотов `> 0`
2) Фильтрация исходных баннеров
3) Для оставшихся -- разбить их на группы по `id` рекламной кампаниии
4) Для каждой группы параллельно найти наиболее удовлетворяющий список баннеров по `lots` лотам
5) Выбрать из всех лучших списков баннеров для каждого `id` лучшие
6) Равновероятно выбрать 1 список, если таких "лучших" оказалось более одного
7) Вернуть результат

## Прочее
* Параллельный поиск производится через пул потоков с размером `std::thread::hardware_concurrency` по средством помещения
задач расчётов в очередь задач пула (`simple_thread_pool.h`)
* Для аккумуляции "цен" (для разных типов баннеров определение цены разное) используется объект баннера
того же типа, что и в списке баннеров, но с аккумулироваными полями. Так как в `c++11` нет возможности поэлементого суммирования `std::tuple`
из коробки, это реализовано в `tuple_utils.h` с доп. реализацией `index_sequence`
* Для улучшения читабельности реализован `hash_back_inserter` -- простейший `output_iterator` для группировки
баннеров по `id` рекламной кампании 


