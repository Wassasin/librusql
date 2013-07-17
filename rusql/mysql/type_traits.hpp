#pragma once

#include <string>
#include <functional>

#include <boost/optional.hpp>

namespace rusql { namespace mysql {
	template <typename T>
	struct type_traits;

	typedef std::function<void(MYSQL_BIND&, Statement&, unsigned int)> OutputProcessor;
	
	//! A collection a functions that map C++ types in one way or another to what MySQL wants (buffer, is_null, field length, etc.)
	namespace field {
		namespace length {
			//! For fields with a fixed length, such as int, double, etc.
			struct Fixed {
				template <typename T>
				static size_t get(T const&){
					return 0;
				}
			};

			struct String {
				static size_t get(std::string x){
					return x.size();
				}
			};

			struct Optional {
				template <typename T>
				static size_t get(boost::optional<T> const& x){
					if(x){
						return type_traits<T>::length::get(*x);
					} else {
						return 0;
					}
				}
			};
		}

		//! A collection of functors that get the char const* to any type of variable, to pass to MYSQL_BIND for example.
		namespace buffer {
			//! All primitve types (int, double, etc.) can be simply cast to a char* and be done with it.
			struct Primitive {
				template <typename T>
				static char* get(T& x){
					return reinterpret_cast<char*>(&x);
				}
			};
			
			struct String {
				static char* get(std::string& x){
					return &x[0];
				}
			};
			
			struct CharPointer {
				static char* get(char* x){
					return x;
				}
			};
			
			//! For boost::optional, returning the pointer only if the object was set.
			struct Optional {
				template <typename T>
				static char* get(boost::optional<T> &x) {
					if(x) {
						return type_traits<T>::data::get(x.get());
					} else {
						return nullptr;
					}
				}
			};

			//! For boost::optional, we only return a buffer for fixed-size types
			struct OptionalOutput {
				template <typename T>
				static char* get(boost::optional<T> &x) {
					typename type_traits<T>::length Length;
					return get_internal(x, Length);
				}

				template <typename T>
				static char* get_internal(boost::optional<T> &x, length::Fixed){
					if(!x) {
						x = T();
					}
					return type_traits<T>::output_data::get(x.get());
				}

				template <typename T>
				static char* get_internal(boost::optional<T> &, length::String){
					return nullptr;
				}

				template <typename T>
				static char* get_internal(boost::optional<T> &x, length::Optional){
					return get_internal((x = T()).get(), type_traits<T>::length);
				}
			};
			
			//! For those types without data, such as boost::none_t
			struct Null {
				template <typename T>
				static char* get(T&) {
					return nullptr;
				}
			};
		}
		
		namespace type {
			template <enum_field_types type> 
			struct Tag {
				template <typename T>
				static constexpr enum_field_types get(T const&){
					return type;
				}
			};
			
			typedef Tag<MYSQL_TYPE_TINY> Tiny;
			typedef Tag<MYSQL_TYPE_SHORT> Short;
			typedef Tag<MYSQL_TYPE_LONG> Long;
			typedef Tag<MYSQL_TYPE_LONGLONG> LongLong;
			typedef Tag<MYSQL_TYPE_STRING> String;
			typedef Tag<MYSQL_TYPE_NULL> Null;
			
			struct Optional {
				template <typename T>
				static enum_field_types get(boost::optional<T> const& x){
					if(x){
						return type_traits<T>::type::get(*x);
					} else {
						return MYSQL_TYPE_NULL;
					}
				}
			};
			
			struct Pointer {
				template <typename T>
				static enum_field_types get(T const * const x){
					if(x){
						return type_traits<T>::type::get(*x);
					} else {
						return MYSQL_TYPE_NULL;
					}
				}
			};
		}

		// Need this as well because boost::optional as "output" (will be written to) is not a MYSQL_TYPE_NULL but simply a T.
		namespace output_type {
			struct Optional {
				template <typename T>
				static enum_field_types get(boost::optional<T> const& x){
					return type_traits<T>::output_type::get(x);
				}
			};
		}
		
		namespace is_unsigned {
			struct No {
				template <typename T>
				static constexpr bool get(T const&){
					return false;
				}
			};
			
			struct Yes {
				template <typename T>
				static constexpr bool get(T const&){
					return true;
				}
			};
			
			struct Optional {
				template <typename T>
				static bool get(boost::optional<T> const& x){
					if(x){
						return type_traits<T>::is_unsigned::get(*x);
					} else {
						return false;
					}
				}
			};
		}

		namespace post_processors {
			struct Optional;
			struct String;
			struct NoPostProcessing;
		}
	}
	
	struct NoProcessing { typedef field::post_processors::NoPostProcessing output_processor; };
	struct Primitive : NoProcessing
	                    { typedef field::buffer::Primitive data;
	                      typedef data output_data; };
	struct Fixed        { typedef field::length::Fixed length; };
	struct Unsigned     { typedef field::is_unsigned::Yes is_unsigned; };
	struct Signed       { typedef field::is_unsigned::No is_unsigned; };
	
	template <>
	struct type_traits<uint16_t> : Primitive, Fixed, Unsigned {
		typedef field::type::Short type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<uint32_t> : Primitive, Fixed, Unsigned {
		typedef field::type::Long type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<uint64_t> : Primitive, Fixed, Unsigned {
		typedef field::type::LongLong type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<int16_t> : Primitive, Fixed, Signed {
		typedef field::type::Short type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<int32_t> : Primitive, Fixed, Signed {
		typedef field::type::Long type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<int64_t> : Primitive, Fixed, Signed {
		typedef field::type::LongLong type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<bool> : Primitive, Fixed, Unsigned {
		typedef field::type::Tiny type;
		typedef type output_type;
	};
	
	template <>
	struct type_traits<std::string> : Unsigned {
		typedef field::type::String type;
		typedef type output_type;
		typedef field::buffer::String data;
		// set to null to just get the length; we set the buffer later
		typedef field::buffer::Null output_data;
		typedef field::length::String length;
		typedef field::post_processors::String output_processor;
	};
	
	template <typename T>
	struct type_traits<boost::optional<T>> {
		typedef field::type::Optional type;
		typedef field::output_type::Optional output_type;
		typedef field::buffer::Optional data;
		// set to null to just get the length; we set the buffer later
		typedef field::buffer::OptionalOutput output_data;
		typedef field::length::Optional length;
		typedef field::is_unsigned::Optional is_unsigned;
		typedef field::post_processors::Optional output_processor;
	};
	
	template <size_t size>
	struct type_traits<char[size]> : Unsigned, NoProcessing {
		typedef field::type::String type;
		typedef type output_type;
		typedef field::buffer::CharPointer data;
		typedef data output_data;
		typedef field::length::String length;
	};
	
	template <>
	struct type_traits<char*> : Unsigned, NoProcessing {
		typedef field::type::String type;
		typedef type output_type;
		typedef field::buffer::CharPointer data;
		typedef data output_data;
		typedef field::length::String length;
	};
	
	template <>
	struct type_traits<char const*> : Unsigned, NoProcessing {
		typedef field::type::String type;
		typedef type output_type;
		typedef field::buffer::CharPointer data;
		typedef data output_data;
		typedef field::length::String length;
	};
	
	template <>
	struct type_traits<boost::none_t> : Unsigned, NoProcessing {
		typedef field::type::Null type;
		typedef type output_type;
		typedef field::buffer::Null data;
		typedef data output_data;
		typedef field::length::Fixed length;
	};
}}
