// citizengine source file

#ifndef __CompileTimeAssert_h__
#define __CompileTimeAssert_h__

#if defined (__cplusplus)

#define COMPILE_TIME_ASSERT(expr)	static_assert(expr, #expr)

#endif


#endif // #ifndef __CompileTimeAssert_h__