/*
 * updatable.h
 *
 *  Created on: Dec 18, 2014
 *      Author: bogdan
 */

#ifndef UPDATABLE_H_
#define UPDATABLE_H_

#include <memory>
#include "Event.h"

template<typename T> class Event;

template <typename T>
void update(T *t, float dt);

template <typename T>
void setOnDestroyCallback(T* t, std::function<void(T*)> callback);

class updatable_wrap {
public:
	template<typename T>
	decltype(T::onDestroy)* subscribe(T* t) {
		subscriptionId = t->onDestroy.add([this](T* t) {
			onDestroy.trigger(this);
		});
		return nullptr;
	}
	void subscribe(...) {
	}

	template<typename T>
	updatable_wrap(T* t)
		: self_(new model_t<T>(t)) {
		subscribe(t);
	}

	~updatable_wrap() {
		must unsubscribe...
	}

	updatable_wrap(const updatable_wrap& w) : self_(w.self_->copy()) {}
	updatable_wrap(updatable_wrap &&w) : self_(std::move(w.self_)) {}
	updatable_wrap operator = (updatable_wrap &&w) { return updatable_wrap(w); }

	bool equal_value(updatable_wrap const& w) const {
		return self_->equal_value(w.self_.get());
	}

	void update(float dt) {
		self_->update_(dt);
	}

	Event<void(updatable_wrap*)> onDestroy;
	int subscriptionId;

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void update_(float dt) = 0;
		virtual concept_t* copy()=0;
		virtual bool equal_value(const concept_t* x) const = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* data_;
		~model_t() noexcept override {};
		model_t(T* x) : data_(x) {}
		void update_(float dt) override {
			::update(data_, dt);
		}
		concept_t* copy() override {
			return new model_t<T>(data_);
		}
		bool equal_value(const concept_t* x) const {
			const model_t<T>* ptr = dynamic_cast<const model_t<T>*>(x);
			if (ptr)
				return ptr->data_ == data_;
			else
				return false;
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* UPDATABLE_H_ */
