/*
 * updatable.h
 *
 *  Created on: Dec 18, 2014
 *      Author: bogdan
 */

#ifndef UPDATABLE_H_
#define UPDATABLE_H_

#include <memory>

template<typename T> void update(T* t, float dt);

class updatable_wrap {
public:
	template<typename T>
	updatable_wrap(T* t)
		: self_(new model_t<T>(t)) {
	}

	updatable_wrap(const updatable_wrap& w) : self_(w.self_->copy()) {}
	updatable_wrap(updatable_wrap&& w) : self_(std::move(w.self_)) {}
	updatable_wrap& operator = (updatable_wrap const &w) { self_ = decltype(self_)(w.self_->copy()); return *this; }
	updatable_wrap& operator = (updatable_wrap &&w) { self_ = std::move(w.self_); return *this; }

	bool equal_value(updatable_wrap const& w) const {
		return self_->getRawPtr() == w.self_->getRawPtr();
	}

	bool equal_raw(void* ptr) const {
		return self_->getRawPtr() == ptr;
	}

	void update(float dt) {
		self_->update_(dt);
	}

	void* debugGetPtr() const { return self_->getRawPtr(); }

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void update_(float dt) = 0;
		virtual concept_t* copy()=0;
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		~model_t() noexcept override {};
		model_t(T* x) : obj_(x) {}
		void update_(float dt) override {
			updateImpl(obj_, dt, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() override {
			return obj_;
		}

		template<typename T1>
		static decltype(&T1::update) updateImpl(T1* t, float dt, bool dummyToUseMember) {
			t->update(dt);
			return nullptr;
		}
		template<typename T1>
		static void updateImpl(T1* t, float dt, ...) {
			::update(t, dt);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* UPDATABLE_H_ */
