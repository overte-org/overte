//
//  Varying.h
//  render/src/task
//
//  Created by Zach Pomerantz on 1/6/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_task_Varying_h
#define hifi_task_Varying_h

#include <type_traits>
#include <tuple>
#include <array>

namespace task {
class Varying;


// A varying piece of data, to be used as Job/Task I/O
class Varying {
public:
    Varying() {}
    Varying(const Varying& var) : _concept(var._concept) {}
    Varying& operator=(const Varying& var) {
       _concept = var._concept;
        return (*this);
    }
    template <class T> Varying(const T& data, const std::string& name = "noname") : _concept(std::make_shared<Model<T>>(data, name)) {}

    template <class T> bool canCast() const { return !!std::dynamic_pointer_cast<Model<T>>(_concept); }
    template <class T> const T& get() const {
        Q_ASSERT(std::dynamic_pointer_cast<const Model<T>>(_concept));
        return std::static_pointer_cast<const Model<T>>(_concept)->_data;
    }
    template <class T> T& edit() { return std::static_pointer_cast<Model<T>>(_concept)->_data; }

    const std::string name() const { return _concept->name(); }

    // access potential sub varyings contained in this one.
    Varying operator[] (uint8_t index) const { return (*_concept)[index]; }
    uint8_t length() const { return (*_concept).length(); }

    template <class T> Varying getN (uint8_t index) const { return get<T>()[index]; }
    template <class T> Varying editN (uint8_t index) { return edit<T>()[index]; }

    bool isNull() const { return _concept == nullptr; }

protected:
    class Concept {
    public:
        Concept(const std::string& name) : _name(name) {}

        virtual ~Concept() = default;

        virtual Varying operator[] (uint8_t index) const = 0;
        virtual uint8_t length() const = 0;

        const std::string name() { return _name; }

        const std::string _name;
    };
    template <class T> class Model : public Concept {
    public:
        using Data = T;

        Model(const Data& data, const std::string& name) : Concept(name), _data(data) {}
        virtual ~Model() = default;

        virtual Varying operator[] (uint8_t index) const override {
            return Varying();
        }
        virtual uint8_t length() const override {
            return 0;
        }

        Data _data;
    };

    std::shared_ptr<Concept> _concept;
};

template < typename T0, typename T1 >
class VaryingSet2 : public std::pair<Varying, Varying> {
public:
    using Parent = std::pair<Varying, Varying>;
    typedef void is_proxy_tag;

    VaryingSet2() : Parent(Varying(T0()), Varying(T1())) {}
    VaryingSet2(const VaryingSet2& pair) : Parent(pair.first, pair.second) {}
    VaryingSet2(const Varying& first, const Varying& second) : Parent(first, second) {}

    const T0& get0() const { return first.get<T0>(); }
    T0& edit0() { return first.edit<T0>(); }

    const T1& get1() const { return second.get<T1>(); }
    T1& edit1() { return second.edit<T1>(); }

    virtual Varying operator[] (uint8_t index) const {
        if (index == 1) {
            return std::get<1>((*this));
        } else {
            return std::get<0>((*this));
        }
    }
    virtual uint8_t length() const { return 2; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2>
class VaryingSet3 : public std::tuple<Varying, Varying,Varying>{
public:
    using Parent = std::tuple<Varying, Varying, Varying>;

    VaryingSet3() : Parent(Varying(T0()), Varying(T1()), Varying(T2())) {}
    VaryingSet3(const VaryingSet3& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src)) {}
    VaryingSet3(const Varying& first, const Varying& second, const Varying& third) : Parent(first, second, third) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    virtual Varying operator[] (uint8_t index) const {
        if (index == 2) {
            return std::get<2>((*this));
        } else if (index == 1) {
            return std::get<1>((*this));
        } else {
            return std::get<0>((*this));
        }
    }
    virtual uint8_t length() const { return 3; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3>
class VaryingSet4 : public std::tuple<Varying, Varying, Varying, Varying>{
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying>;

    VaryingSet4() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3())) {}
    VaryingSet4(const VaryingSet4& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src)) {}
    VaryingSet4(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth) : Parent(first, second, third, fourth) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    virtual Varying operator[] (uint8_t index) const {
        if (index == 3) {
            return std::get<3>((*this));
        } else if (index == 2) {
            return std::get<2>((*this));
        } else if (index == 1) {
            return std::get<1>((*this));
        } else {
            return std::get<0>((*this));
        }
    }
    virtual uint8_t length() const { return 4; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3, class T4>
class VaryingSet5 : public std::tuple<Varying, Varying, Varying, Varying, Varying>{
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying>;

    VaryingSet5() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4())) {}
    VaryingSet5(const VaryingSet5& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src)) {}
    VaryingSet5(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth) : Parent(first, second, third, fourth, fifth) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }

    virtual Varying operator[] (uint8_t index) const {
        if (index == 4) {
            return std::get<4>((*this));
        } else if (index == 3) {
            return std::get<3>((*this));
        } else if (index == 2) {
            return std::get<2>((*this));
        } else if (index == 1) {
            return std::get<1>((*this));
        } else {
            return std::get<0>((*this));
        }
    }
    virtual uint8_t length() const { return 5; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3, class T4, class T5>
class VaryingSet6 : public std::tuple<Varying, Varying, Varying, Varying, Varying, Varying>{
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying, Varying>;

    VaryingSet6() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4()), Varying(T5())) {}
    VaryingSet6(const VaryingSet6& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src), std::get<5>(src)) {}
    VaryingSet6(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth, const Varying& sixth) : Parent(first, second, third, fourth, fifth, sixth) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }

    const T5& get5() const { return std::get<5>((*this)).template get<T5>(); }
    T5& edit5() { return std::get<5>((*this)).template edit<T5>(); }

    virtual Varying operator[] (uint8_t index) const {
        switch (index) {
        default:
            return std::get<0>((*this));
        case 1:
            return std::get<1>((*this));
        case 2:
            return std::get<2>((*this));
        case 3:
            return std::get<3>((*this));
        case 4:
            return std::get<4>((*this));
        case 5:
            return std::get<5>((*this));
        };
    }
    virtual uint8_t length() const { return 6; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
class VaryingSet7 : public std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying>{
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying>;
    
    VaryingSet7() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4()), Varying(T5()), Varying(T6())) {}
    VaryingSet7(const VaryingSet7& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src), std::get<5>(src), std::get<6>(src)) {}
    VaryingSet7(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth, const Varying& sixth, const Varying& seventh) : Parent(first, second, third, fourth, fifth, sixth, seventh) {}
    
    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }
    
    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }
    
    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }
    
    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }
    
    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }
    
    const T5& get5() const { return std::get<5>((*this)).template get<T5>(); }
    T5& edit5() { return std::get<5>((*this)).template edit<T5>(); }
    
    const T6& get6() const { return std::get<6>((*this)).template get<T6>(); }
    T6& edit6() { return std::get<6>((*this)).template edit<T6>(); }
    
    virtual Varying operator[] (uint8_t index) const {
        switch (index) {
        default:
            return std::get<0>((*this));
        case 1:
            return std::get<1>((*this));
        case 2:
            return std::get<2>((*this));
        case 3:
            return std::get<3>((*this));
        case 4:
            return std::get<4>((*this));
        case 5:
            return std::get<5>((*this));
        case 6:
            return std::get<6>((*this));
        };
    }
    virtual uint8_t length() const { return 7; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class VaryingSet8 : public std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying> {
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying>;

    VaryingSet8() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4()), Varying(T5()), Varying(T6()), Varying(T7())) {}
    VaryingSet8(const VaryingSet8& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src), std::get<5>(src), std::get<6>(src), std::get<7>(src)) {}
    VaryingSet8(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth, const Varying& sixth, const Varying& seventh, const Varying& eighth) : Parent(first, second, third, fourth, fifth, sixth, seventh, eighth) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }

    const T5& get5() const { return std::get<5>((*this)).template get<T5>(); }
    T5& edit5() { return std::get<5>((*this)).template edit<T5>(); }

    const T6& get6() const { return std::get<6>((*this)).template get<T6>(); }
    T6& edit6() { return std::get<6>((*this)).template edit<T6>(); }

    const T7& get7() const { return std::get<7>((*this)).template get<T7>(); }
    T7& edit7() { return std::get<7>((*this)).template edit<T7>(); }

    virtual Varying operator[] (uint8_t index) const {
        switch (index) {
        default:
            return std::get<0>((*this));
        case 1:
            return std::get<1>((*this));
        case 2:
            return std::get<2>((*this));
        case 3:
            return std::get<3>((*this));
        case 4:
            return std::get<4>((*this));
        case 5:
            return std::get<5>((*this));
        case 6:
            return std::get<6>((*this));
        case 7:
            return std::get<7>((*this));
        };
    }
    virtual uint8_t length() const { return 8; }

    Varying asVarying() const { return Varying((*this)); }
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
class VaryingSet9 : public std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying> {
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying>;

    VaryingSet9() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4()), Varying(T5()), Varying(T6()), Varying(T7()), Varying(T8())) {}
    VaryingSet9(const VaryingSet9& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src), std::get<5>(src), std::get<6>(src), std::get<7>(src), std::get<8>(src)) {}
    VaryingSet9(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth, const Varying& sixth, const Varying& seventh, const Varying& eighth, const Varying& nine) : Parent(first, second, third, fourth, fifth, sixth, seventh, eighth, nine) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }

    const T5& get5() const { return std::get<5>((*this)).template get<T5>(); }
    T5& edit5() { return std::get<5>((*this)).template edit<T5>(); }

    const T6& get6() const { return std::get<6>((*this)).template get<T6>(); }
    T6& edit6() { return std::get<6>((*this)).template edit<T6>(); }

    const T7& get7() const { return std::get<7>((*this)).template get<T7>(); }
    T7& edit7() { return std::get<7>((*this)).template edit<T7>(); }

    const T8& get8() const { return std::get<8>((*this)).template get<T8>(); }
    T8& edit8() { return std::get<8>((*this)).template edit<T8>(); }
    virtual Varying operator[] (uint8_t index) const {
        switch (index) {
        default:
            return std::get<0>((*this));
        case 1:
            return std::get<1>((*this));
        case 2:
            return std::get<2>((*this));
        case 3:
            return std::get<3>((*this));
        case 4:
            return std::get<4>((*this));
        case 5:
            return std::get<5>((*this));
        case 6:
            return std::get<6>((*this));
        case 7:
            return std::get<7>((*this));
        case 8:
            return std::get<8>((*this));
        };
    }
    virtual uint8_t length() const { return 9; }

    Varying asVarying() const { return Varying((*this)); }
};


template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
class VaryingSet10 : public std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying> {
public:
    using Parent = std::tuple<Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying, Varying>;

    VaryingSet10() : Parent(Varying(T0()), Varying(T1()), Varying(T2()), Varying(T3()), Varying(T4()), Varying(T5()), Varying(T6()), Varying(T7()), Varying(T8()), Varying(T9())) {}
    VaryingSet10(const VaryingSet10& src) : Parent(std::get<0>(src), std::get<1>(src), std::get<2>(src), std::get<3>(src), std::get<4>(src), std::get<5>(src), std::get<6>(src), std::get<7>(src), std::get<8>(src), std::get<9>(src)) {}
    VaryingSet10(const Varying& first, const Varying& second, const Varying& third, const Varying& fourth, const Varying& fifth, const Varying& sixth, const Varying& seventh, const Varying& eighth, const Varying& nine, const Varying& ten) : Parent(first, second, third, fourth, fifth, sixth, seventh, eighth, nine, ten) {}

    const T0& get0() const { return std::get<0>((*this)).template get<T0>(); }
    T0& edit0() { return std::get<0>((*this)).template edit<T0>(); }

    const T1& get1() const { return std::get<1>((*this)).template get<T1>(); }
    T1& edit1() { return std::get<1>((*this)).template edit<T1>(); }

    const T2& get2() const { return std::get<2>((*this)).template get<T2>(); }
    T2& edit2() { return std::get<2>((*this)).template edit<T2>(); }

    const T3& get3() const { return std::get<3>((*this)).template get<T3>(); }
    T3& edit3() { return std::get<3>((*this)).template edit<T3>(); }

    const T4& get4() const { return std::get<4>((*this)).template get<T4>(); }
    T4& edit4() { return std::get<4>((*this)).template edit<T4>(); }

    const T5& get5() const { return std::get<5>((*this)).template get<T5>(); }
    T5& edit5() { return std::get<5>((*this)).template edit<T5>(); }

    const T6& get6() const { return std::get<6>((*this)).template get<T6>(); }
    T6& edit6() { return std::get<6>((*this)).template edit<T6>(); }

    const T7& get7() const { return std::get<7>((*this)).template get<T7>(); }
    T7& edit7() { return std::get<7>((*this)).template edit<T7>(); }

    const T8& get8() const { return std::get<8>((*this)).template get<T8>(); }
    T8& edit8() { return std::get<8>((*this)).template edit<T8>(); }

    const T9& get9() const { return std::get<9>((*this)).template get<T9>(); }
    T9& edit9() { return std::get<9>((*this)).template edit<T9>(); }

    virtual Varying operator[] (uint8_t index) const {
        switch (index) {
        default:
            return std::get<0>((*this));
        case 1:
            return std::get<1>((*this));
        case 2:
            return std::get<2>((*this));
        case 3:
            return std::get<3>((*this));
        case 4:
            return std::get<4>((*this));
        case 5:
            return std::get<5>((*this));
        case 6:
            return std::get<6>((*this));
        case 7:
            return std::get<7>((*this));
        case 8:
            return std::get<8>((*this));
        case 9:
            return std::get<9>((*this));
        };
    }
    virtual uint8_t length() const { return 10; }

    Varying asVarying() const { return Varying((*this)); }
};

template < class T, int NUM >
class VaryingArray : public std::array<Varying, NUM> {
public:
    VaryingArray() {
        for (size_t i = 0; i < NUM; i++) {
            (*this)[i] = Varying(T());
        }
    }

    VaryingArray(std::initializer_list<Varying> list) {
        assert(list.size() == NUM);
        std::copy(list.begin(), list.end(), std::array<Varying, NUM>::begin());
    }
};

}

#endif // hifi_task_Varying_h
