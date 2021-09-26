#include "../include/minmsg.h"
#include <string>
#include <iostream>

struct TestStruct
{
    int a, b, c;
    bool d;
    std::string e;

    MINMSG_ADD_FIELDS(&a, &b, &c, &d, &e);
};

struct NestedTestStruct
{
    std::string a;
    TestStruct b;

    MINMSG_ADD_FIELDS(&a, &b);
};

bool test_pack_unpack() {
    // Pack struct
    NestedTestStruct test_struct_in{"Test String", {5, 10, 15, true, "Test String 2"}};

    char buffer[sizeof(NestedTestStruct)];
    minmsg::bbuf_write bbuf_write(buffer);
    minmsg::pack(&bbuf_write, &test_struct_in);

    // Unpack
    NestedTestStruct test_struct_out{};

    minmsg::bbuf_read bbuf_read(buffer);
    minmsg::unpack(&bbuf_read, &test_struct_out);

    return 
    test_struct_out.a == test_struct_in.a
    && test_struct_out.b.e == test_struct_in.b.e;
}

int main() {
    if(!test_pack_unpack()) {
        std::cout << "Failed to pack/unpack struct!" << std::endl;
        return 1;
    }
        
    // Test passes
    return 0;
}