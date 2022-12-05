//#include <iostream>


#define C_RETURN_ERRNO_TEST(ret_type, test) \
    static_cast<bool (*)(ret_type ret, int err)>([](ret_type ret, int err){ retrun test; })
#define C_RETURN_TEST(ret_type, test) \
    static_cast<bool (*)(ret_type ret, int /*err*/)>([](ret_type ret, int /*err*/){ return test; })
#define C_ERRNO_TEST(test) \
    static_cast<bool (*)(int err)>([](int err){ return test; })

#define MACRO_STR(macro) (#macro)


int main() {
    C_RETURN_ERRNO_TEST(clock_t, ret == -1);
    C_RETURN_TEST(clock_t, (ret == -1));
    C_ERRNO_TEST(errno != EINTR);
}
