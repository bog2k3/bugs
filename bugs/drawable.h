/*
 * drawable.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bogdan
 */

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <memory>

class RenderContext;

class drawable_wrap {
public:
	template<typename T>
	drawable_wrap(T* t)
		: self_(new model_t<T>(t)) {
	}

	drawable_wrap(const drawable_wrap& w) : self_(w.self_->copy()) {}
	drawable_wrap(drawable_wrap &&w) : self_(std::move(w.self_)) {}
	drawable_wrap operator = (drawable_wrap const &w) { return drawable_wrap(w); }

	bool equal_value(drawable_wrap const& w) const {
		return self_->getRawPtr() == w.self_->getRawPtr();
	}

	void draw(RenderContext const &ctx) {
		self_->draw_(ctx);
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void draw_(RenderContext const& ctx) = 0;
		virtual concept_t* copy()=0;
		virtual bool equal_value(const concept_t* x) const = 0;
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		model_t(T x) : obj_(std::move(x)) {}
		~model_t() noexcept {};
		void draw_(RenderContext const& ctx) override {
			obj_->draw(ctx);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() {
			return obj_;
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* DRAWABLE_H_ */
