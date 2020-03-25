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

        std::shared_ptr<Object> clone() const override {
            return std::make_shared<TestSumRower::TestSumFielder>();
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

    std::shared_ptr<Object> clone() const override {
        return std::make_shared<TestSumRower>();
    }

    void join(std::shared_ptr<Rower> other) override {
        auto tsr = std::dynamic_pointer_cast<TestSumRower>(other);
        assert(tsr);
        _sum += tsr->get_sum();
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
