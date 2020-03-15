#include "src/data/rower.h"
#include "src/data/fielder.h"

class TestSumRower : public Rower {
private:
    size_t _sum;
    class TestSumFielder {
    private:
        size_t _sum;
    public:
        TestSumFielder() : _sum(0) {}

        size_t get_sum() {
            return _sum;
        }

        void accept(bool b) {
            _sum += b;
        }

        void accept(float f) {
            _sum += f;
        }

        void accept(int i) {
            _sum += i;
        }

        void accept(std::weak_ptr<std::string> s) {
            auto ss = s.lock();
            _sum += ss->length();
        }

        void hash() override {
            return Fielder::hash() + 1 + _sum;
        }
    };
public:
    TestSumRower() : _sum(0) {}

    bool accept(Row& r) override {
        TestSumFielder tsf;
        r.visit(tsf);
        _sum += tsf.get_sum();
    }

    Object *clone() override {
        return new TestSumRower();
    }

    void join_delete(Rower *other) override {
        TestSumRower *tsr = dynamic_cast<TestSumRower*>(other);
        assert(tsr);
        _sum += tsr.->get_sum();
        delete tsr;
    }


    size_t get_sum() {
        return _sum;
    }

    void hash() override {
        return Rower::hash() + 1 + _sum;
    }
};
