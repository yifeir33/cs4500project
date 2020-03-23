#include "data/rower.h"
#include "data/fielder.h"

class TestSumRower : public Rower {
private:
    size_t _sum;
    class TestSumFielder : public Fielder {
    private:
        size_t _sum;
    public:
        TestSumFielder() : _sum(0) {}

        size_t get_sum() {
            return _sum;
        }

        void accept(std::optional<bool> b) override {
            if(b) _sum += *b;
        }

        void accept(std::optional<double> d) override {
            if(d) _sum += *d;
        }

        void accept(std::optional<int> i) override {
            if(i) _sum += *i;
        }

        void accept(std::optional<std::string> s) override {
            if(s) {
                _sum += s->length();
            }
        }

        size_t hash() const override {
            return 169 + _sum;
        }

        Object *clone() const override {
            return new TestSumRower::TestSumFielder();
        }

        bool equals(const Object* other) const override {
            return dynamic_cast<const TestSumRower::TestSumFielder *>(other);
        }
    };
public:
    TestSumRower() : _sum(0) {}

    bool accept(Row& r) override {
        TestSumFielder tsf;
        r.visit(r.get_index(), tsf);
        _sum += tsf.get_sum();
        return true;
    }

    Object *clone() const override {
        return new TestSumRower();
    }

    void join_delete(Rower *other) override {
        TestSumRower *tsr = dynamic_cast<TestSumRower*>(other);
        assert(tsr);
        _sum += tsr->get_sum();
        delete tsr;
    }

    size_t get_sum() {
        return _sum;
    }

    size_t hash() const override {
        return 159 + _sum;
    }

    bool equals(const Object* other) const override {
        return dynamic_cast<const TestSumRower *>(other);
    }
};
