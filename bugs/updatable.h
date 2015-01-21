/*
 * updatable.h
 *
 *  Created on: Dec 18, 2014
 *      Author: bogdan
 */

#ifndef UPDATABLE_H_
#define UPDATABLE_H_

#include <memory>

class updatable_wrap {
public:
	template<typename T>
	updatable_wrap(std::weak_ptr<T> t)
		: self_(new model_t<T>(t)) {
	}

	updatable_wrap(const updatable_wrap& w) : self_(w.self_->copy()) {}
	updatable_wrap operator = (updatable_wrap const &w) { return updatable_wrap(w); }

	bool isAlive() {
		return self_->isAlive();
	}

	void update(float dt) {
		self_->update_(dt);
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void update_(float dt) = 0;
		virtual concept_t* copy()=0;
		virtual bool isAlive() const = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		std::weak_ptr<T> obj_;
		~model_t() noexcept override {};
		model_t(std::weak_ptr<T> x) : obj_(x) {}
		void update_(float dt) override {
			if (auto sptr = obj_.lock())
				sptr->update(dt);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		bool isAlive() const override {
			return !obj_.expired();
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* UPDATABLE_H_ */
