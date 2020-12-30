
#if __has_include(<coroutine>)
#include <coroutine>
#define MH_COROUTINES_SUPPORTED 1
namespace mh::detail
{
	namespace coro = std;
}
#elif __has_include(<experimental/coroutine>)
#define MH_COROUTINES_SUPPORTED 1
namespace mh::detail
{
	namespace coro = std::experimental;
}
#endif

int main(int argc, char** argv)
{
	mh::detail::coro::coroutine_handle<> handle;
	return 0;
}
