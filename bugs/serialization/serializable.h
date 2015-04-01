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
enum class SerializationObjectTypes;

// serialize a T object into the given stream
template<typename T> void serialize(T* t, BinaryStream &stream);

template<typename T> SerializationObjectTypes getSerializationType(T* t);

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

	SerializationObjectTypes getType() {
		return self_->getType_();
	}

private:
	struct concept_t {
		virtual ~concept_t() noexcept = default;
		virtual void serialize_(BinaryStream &stream) = 0;
		virtual SerializationObjectTypes getType_() = 0;
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
		SerializationObjectTypes getType_() override {
			return getTypeImpl(obj_, true);
		}
		concept_t* copy() override {
			return new model_t<T>(obj_);
		}
		void* getRawPtr() override {
			return obj_;
		}

		template<typename T1, typename T2 = decltype(&T1::serialize)>
		static void serializeImpl(T1* t, BinaryStream &stream, bool dummyToUseMember) {
			t->serialize(stream);
		}
		template<typename T1>
		static void serializeImpl(T1* t, BinaryStream &stream, ...) {
			::serialize(t, stream);
		}

		template<typename T1, typename T2 = decltype(&T1::getSerializationType)>
		static SerializationObjectTypes getTypeImpl(T1* t, bool dummyToUseMember) {
			return t->getSerializationType();
		}
		template<typename T1>
		static SerializationObjectTypes getTypeImpl(T1* t, ...) {
			return ::getSerializationType(t);
		}
	};

	std::unique_ptr<concept_t> self_;
};

#endif /* SERIALIZATION_SERIALIZABLE_H_ */
