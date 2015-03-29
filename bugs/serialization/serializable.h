/*
 * serializable.h
 *
 *  Created on: Mar 29, 2015
 *      Author: bog
 */

#ifndef SERIALIZATION_SERIALIZABLE_H_
#define SERIALIZATION_SERIALIZABLE_H_

#include <memory>

class BinaryStream;

// serialize a T object into the given stream
template<typename T> void serialize(T* t, BinaryStream &stream);

class serializable_wrap {
public:
	template<typename T>
	serializable_wrap(T* t)
		: self_(new model_t<T>(t)) {
	}

	serializable_wrap(const serializable_wrap& w) : self_(w.self_->copy()) {}
	serializable_wrap(serializable_wrap&& w) : self_(std::move(w.self_)) {}
	serializable_wrap& operator = (serializable_wrap const &w) { self_ = decltype(self_)(w.self_->copy()); return *this; }
	serializable_wrap& operator = (serializable_wrap &&w) { self_ = std::move(w.self_); return *this; }

	bool equal_raw(void* ptr) const {
		return self_->getRawPtr() == ptr;
	}

	void serialize(BinaryStream &stream) {
		self_->serialize_(stream);
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void serialize_(BinaryStream &stream) = 0;
		virtual concept_t* copy()=0;
		virtual void* getRawPtr() = 0;
	};
	template<typename T>
	struct model_t : concept_t {
		T* obj_;
		~model_t() noexcept override {};
		model_t(T* x) : obj_(x) {}
		void serialize_(BinaryStream &stream) override {
			serializeImpl(obj_, stream, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() override {
			return obj_;
		}

		template<typename T1>
		static decltype(&T1::serialize) serializeImpl(T1* t, BinaryStream &stream, bool dummyToUseMember) {
			t->serialize(stream);
			return nullptr;
		}
		template<typename T1>
		static void serializeImpl(T1* t, BinaryStream &stream, ...) {
			::serialize(t, stream);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* SERIALIZATION_SERIALIZABLE_H_ */
