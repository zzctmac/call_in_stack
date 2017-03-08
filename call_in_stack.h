#ifndef _CALLINSTACK_H_
#define _CALLINSTACK_H_

#include "call_in_stack_x64_sysv.h"
#include "call_in_stack_x86_sysv.h"


//after arguments passing and save ip pointer, the stack pointer should be at 16x + WORDSIZE bytes(then sp - wordsize must be 16x)
//GET_ADDRESS_ALIENED is bad implemented because it does not know cost is const.
#define GET_ADDRESS_ALIENED(prev_stack_base, cost)   ((prev_stack_base) - ((((word_int_t)(prev_stack_base)-(cost))&(0x10-1)) ^ WORDSIZE))
template <int Cost>
inline char* get_stack_base(char* prev_stack_base){
	//We can help compiler to optimize when (((Cost + 2) * WORDSIZE) & (STACK_ALIGNMENT_SIZE-1) == 0).
	//The judge is finished in compile time as it is a constant.
	if((((Cost + 2) * WORDSIZE) & (STACK_ALIGNMENT_SIZE-1)) == 0){
		return (char*)(((word_int_t)prev_stack_base) & (~(STACK_ALIGNMENT_SIZE - 1)));
	}else{
		return prev_stack_base - (((word_int_t)prev_stack_base & (STACK_ALIGNMENT_SIZE-1)) ^ (((Cost + 2)  * WORDSIZE) & (STACK_ALIGNMENT_SIZE-1)));
	}
}

//1.We have arguments passing stack cost and previous sp pointer saving cost in stack before calling. 
//  So we use get_stack_base<cost + 1>(stack_buffer + length) as stack_base.
//2.Some versions of GCC has a bug that we can not use "static_asserter< i == function_property<T>::arguments_count > *p = 0" to break overload ambiguous(ambiguous error has higher priority than direct SFINAE?). So we use inner type of static_asserter instead. 

#define jump_stack_call_define(i, j) \
template <MACRO_JOIN(RECURSIVE_FUNC_, j)(define_typenames_ex_begin, define_typenames_ex, define_typenames_ex) typename T > \
inline typename function_property<T>::return_type jump_stack_call(unsigned int stack_length, T dest_func  \
MACRO_JOIN(RECURSIVE_FUNC_,i)(define_typeargs_begin, define_typeargs, define_typeargs) \
MACRO_JOIN(RECURSIVE_FUNC_,j)(define_type_args_ex_begin, define_type_args_ex, define_type_args_ex_end) \
, typename static_asserter< i == function_property<T>::arguments_count >::type *p = 0 ){\
	struct auto_stack_buffer{\
		char* stack_buffer;\
		auto_stack_buffer(unsigned int the_size):stack_buffer(new char[the_size]){}\
		~auto_stack_buffer(){delete []stack_buffer;}\
	}z(stack_length);\
		return call_with_stack_class<typename function_property<T>::return_type>::template call_with_stack< \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_rtypes_begin, define_rtypes, define_rtypes) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_rtype_ex_begin, define_rtype_ex, define_rtype_ex) \
		T>( \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_begin, define_args_ex, define_args_ex) \
		dest_func, get_stack_base< STACK_COST(T) >(z.stack_buffer + stack_length) );\
}\
template <MACRO_JOIN(RECURSIVE_FUNC_, j)(define_typenames_ex_begin, define_typenames_ex, define_typenames_ex) typename T > \
inline typename function_property<T>::return_type jump_stack_call(char* stack_buffer, unsigned int stack_length, T dest_func \
MACRO_JOIN(RECURSIVE_FUNC_,i)(define_typeargs_begin, define_typeargs, define_typeargs) \
MACRO_JOIN(RECURSIVE_FUNC_,j)(define_type_args_ex_begin, define_type_args_ex, define_type_args_ex_end) \
, typename static_asserter< i == function_property<T>::arguments_count >::type *p = 0 ){\
		return call_with_stack_class<typename function_property<T>::return_type>::template call_with_stack< \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_rtypes_begin, define_rtypes, define_rtypes) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_rtype_ex_begin, define_rtype_ex, define_rtype_ex) \
		T>( \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_begin, define_args_ex, define_args_ex) \
		dest_func, get_stack_base< STACK_COST(T) >(stack_buffer + stack_length) );\
}\
template <MACRO_JOIN(RECURSIVE_FUNC_, j)(define_typenames_ex_begin, define_typenames_ex, define_typenames_ex) typename T, typename B, int N > \
inline typename function_property<T>::return_type jump_stack_call(B (&stack_buffer)[N], T dest_func  \
MACRO_JOIN(RECURSIVE_FUNC_,i)(define_typeargs_begin, define_typeargs, define_typeargs) \
MACRO_JOIN(RECURSIVE_FUNC_,j)(define_type_args_ex_begin, define_type_args_ex, define_type_args_ex_end) \
, typename static_asserter< i == function_property<T>::arguments_count >::type *p = 0 ){\
		return call_with_stack_class<typename function_property<T>::return_type>::template call_with_stack< \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_rtypes_begin, define_rtypes, define_rtypes) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_rtype_ex_begin, define_rtype_ex, define_rtype_ex) \
		T>( \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_begin, define_args_ex, define_args_ex) \
		dest_func, get_stack_base< STACK_COST(T) >((char*)(&(stack_buffer[N])) ));\
}
BI_TWO_BATCH_FUNC1(10, jump_stack_call_define)
jump_stack_call_define(0,0)


//jump_stack_call_safe is safe for recursively jump_stack_call with same stack_buffer(maybe it is a global variable?) as stack.
#define jump_stack_call_safe_define(i, j) \
template <MACRO_JOIN(RECURSIVE_FUNC_, j)(define_typenames_ex_begin, define_typenames_ex, define_typenames_ex) typename T > \
inline typename function_property<T>::return_type jump_stack_call_safe( char* stack_buffer, unsigned int stack_length, T dest_func \
MACRO_JOIN(RECURSIVE_FUNC_,i)(define_typeargs_begin, define_typeargs, define_typeargs) \
MACRO_JOIN(RECURSIVE_FUNC_,j)(define_type_args_ex_begin, define_type_args_ex, define_type_args_ex_end) \
, typename static_asserter< i == function_property<T>::arguments_count >::type *p = 0 ){\
	char* sp_value;\
	GET_SP(sp_value);\
	if((word_int_t)((((word_int_t)sp_value - (word_int_t)stack_buffer) ^ ((word_int_t)stack_buffer + stack_length - (word_int_t)sp_value))) > 0ll){ \
		return dest_func(\
			MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args_end) \
			MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_c_begin, define_args_ex_c, define_args_ex_c_end) \
		);\
	} \
	else{ \
		return call_with_stack_class<typename function_property<T>::return_type>::template call_with_stack< \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_rtypes_begin, define_rtypes, define_rtypes) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_rtype_ex_begin, define_rtype_ex, define_rtype_ex) \
		T>( \
			MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args) \
			MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_begin, define_args_ex, define_args_ex) \
			dest_func, get_stack_base< STACK_COST(T) >(stack_buffer + stack_length) );\
	} \
} \
template <MACRO_JOIN(RECURSIVE_FUNC_, j)(define_typenames_ex_begin, define_typenames_ex, define_typenames_ex) typename T, typename B, int N > \
inline typename function_property<T>::return_type jump_stack_call_safe(B (&stack_buffer)[N], T dest_func \
MACRO_JOIN(RECURSIVE_FUNC_,i)(define_typeargs_begin, define_typeargs, define_typeargs) \
MACRO_JOIN(RECURSIVE_FUNC_,j)(define_type_args_ex_begin, define_type_args_ex, define_type_args_ex_end) \
, typename static_asserter< i == function_property<T>::arguments_count >::type *p = 0 ){\
	char* sp_value;\
	GET_SP(sp_value);\
	if(((word_int_t)(((word_int_t)sp_value - (word_int_t)(&stack_buffer)) ^ ((word_int_t)(&(stack_buffer[N])) - (word_int_t)sp_value))) > 0ll){ \
		return dest_func(\
			MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args_end) \
			MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_c_begin, define_args_ex_c, define_args_ex_c_end) \
		);\
	} \
	else{ \
		return call_with_stack_class<typename function_property<T>::return_type>::template call_with_stack< \
		MACRO_JOIN(RECURSIVE_FUNC_,i)(define_rtypes_begin, define_rtypes, define_rtypes) \
		MACRO_JOIN(RECURSIVE_FUNC_,j)(define_rtype_ex_begin, define_rtype_ex, define_rtype_ex) \
		T>( \
			MACRO_JOIN(RECURSIVE_FUNC_,i)(define_args_begin, define_args, define_args) \
			MACRO_JOIN(RECURSIVE_FUNC_,j)(define_args_ex_begin, define_args_ex, define_args_ex) \
			dest_func, get_stack_base< STACK_COST(T) >((char*)(&(stack_buffer[N])) ));\
	}\
}
BI_TWO_BATCH_FUNC1(10, jump_stack_call_safe_define)
jump_stack_call_safe_define(0,0)

#endif
