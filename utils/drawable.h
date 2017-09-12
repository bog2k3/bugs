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

template<typename T> void draw(T* t, RenderContext const& ctx);

class drawable_wrap {
public:
	template<typename T>
	drawable_wrap(T* t)
		: self_(new model_t<T>(t)) {
	}

	drawable_wrap(const drawable_wrap& w) : self_(w.self_->copy()) {}
	drawable_wrap(drawable_wrap &&w) : self_(std::move(w.self_)) {}
	drawable_wrap& operator = (drawable_wrap const &w) { self_ = decltype(self_)(w.self_->copy()); return *this; }
	drawable_wrap& operator = (drawable_wrap &&w) { self_ = std::move(w.self_); return *this; }

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
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		model_t(T* x) : obj_(x) {}
		~model_t() noexcept {};
		void draw_(RenderContext const& ctx) override {
			drawImpl(obj_, ctx, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() {
			return obj_;
		}

		template<typename T1>
		static decltype(&T1::draw) drawImpl(T1* t, RenderContext const& ctx, bool dummyToUseMember) {
			t->draw(ctx);
			return nullptr;
		}
		template<typename T1>
		static void drawImpl(T1* t, RenderContext const& ctx, ...) {
			::draw(t, ctx);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* DRAWABLE_H_ */
