/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "tokenize.h"
#include "checkautovariables.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestAutoVariables : public TestFixture {
public:
    TestAutoVariables() : TestFixture("TestAutoVariables")
    { }

private:



    void check(const char code[], bool inconclusive=false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckAutoVariables checkAutoVariables(&tokenizer, &settings, this);
        checkAutoVariables.returnReference();

        tokenizer.simplifyTokenList();

        // Check auto variables
        checkAutoVariables.autoVariables();
        checkAutoVariables.returnPointerToLocalArray();
        checkAutoVariables.returncstr();
    }

    void run() {
        TEST_CASE(testautovar1);
        TEST_CASE(testautovar2);
        TEST_CASE(testautovar3); // ticket #2925
        TEST_CASE(testautovar4); // ticket #2928
        TEST_CASE(testautovar5); // ticket #2926
        TEST_CASE(testautovar6); // ticket #2931
        TEST_CASE(testautovar7); // ticket #3066
        TEST_CASE(testautovar8);
        TEST_CASE(testautovar9);
        TEST_CASE(testautovar_array1);
        TEST_CASE(testautovar_array2);
        TEST_CASE(testautovar_return1);
        TEST_CASE(testautovar_return2);
        TEST_CASE(testautovar_return3);
        TEST_CASE(testautovar_return4); // ticket #3030
        TEST_CASE(testautovar_extern);
        TEST_CASE(testinvaliddealloc);
        TEST_CASE(testassign1);  // Ticket #1819
        TEST_CASE(testassign2);  // Ticket #2765

        TEST_CASE(returnLocalVariable1);
        TEST_CASE(returnLocalVariable2);

        // return reference..
        TEST_CASE(returnReference1);
        TEST_CASE(returnReference2);
        TEST_CASE(returnReference3);
        TEST_CASE(returnReference4);
        TEST_CASE(returnReference5);

        // return c_str()..
        TEST_CASE(returncstr);

        // global namespace
        TEST_CASE(testglobalnamespace);

        TEST_CASE(returnParameterAddress);
    }



    void testautovar1() {
        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar2() {
        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    *res = &num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    void func1(int **res);\n"
              "}\n"
              "void Fred::func1(int **res)\n"
              "{\n"
              "    int num = 2;\n"
              "    foo.res = &num;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar3() { // ticket #2925
        check("void foo(int **p)\n"
              "{\n"
              "    int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar4() { // ticket #2928
        check("void foo(int **p)\n"
              "{\n"
              "    static int x[100];\n"
              "    *p = x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar5() { // ticket #2926
        check("void foo(struct AB *ab)\n"
              "{\n"
              "    char a;\n"
              "    ab->a = &a;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct AB *ab)\n"
              "{\n"
              "    char a;\n"
              "    ab->a = &a;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Inconclusive: Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar6() { // ticket #2931
        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());

        check("void foo(struct X *x)\n"
              "{\n"
              "    char a[10];\n"
              "    x->str = a;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Inconclusive: Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar7() { // ticket #3066
        check("struct txt_scrollpane_s * TXT_NewScrollPane(struct txt_widget_s * target)\n"
              "{\n"
              "    struct txt_scrollpane_s * scrollpane;\n"
              "    target->parent = &scrollpane->widget;\n"
              "    return scrollpane;\n"
              "}", false);
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar8() {
        check("void foo(int*& p) {\n"
              "    int i = 0;\n"
              "    p = &i;\n"
              "}", false);
        ASSERT_EQUALS("[test.cpp:3]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar9() {
        check("struct FN {int i;};\n"
              "struct FP {FN* f};\n"
              "void foo(int*& p, FN* p_fp) {\n"
              "    FN fn;\n"
              "    FP fp;\n"
              "    p = &fn.i;\n"
              "    p = &p_fp->i;\n"
              "    p = &fp.f->i;\n"
              "}", false);
        ASSERT_EQUALS("[test.cpp:6]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar_array1() {
        check("void func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar_array2() {
        check("class Fred {\n"
              "    void func1(int* arr[2]);\n"
              "}\n"
              "void Fred::func1(int* arr[2])\n"
              "{\n"
              "    int num=2;"
              "    arr[0]=&num;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Assigning address of local auto-variable to a function parameter.\n", errout.str());
    }

    void testautovar_return1() {
        check("int* func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return of the address of an auto-variable\n", errout.str());
    }

    void testautovar_return2() {
        check("class Fred {\n"
              "    int* func1()\n"
              "}\n"
              "int* Fred::func1()\n"
              "{\n"
              "    int num=2;"
              "    return &num;"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Return of the address of an auto-variable\n", errout.str());
    }

    void testautovar_return3() {
        // #2975 - FP
        check("void** f()\n"
              "{\n"
              "    void *&value = tls[id];"
              "    return &value;"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar_return4() {
        // #3030
        check("char *foo()\n"
              "{\n"
              "    char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Return of the address of an auto-variable\n", errout.str());

        check("char *foo()\n"
              "{\n"
              "    static char q[] = \"AAAAAAAAAAAA\";\n"
              "    return &q[1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void testautovar_extern() {
        check("struct foo *f()\n"
              "{\n"
              "    extern struct foo f;\n"
              "    return &f;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testinvaliddealloc() {
        check("int* func1()\n"
              "{\n"
              "int a;\n"
              "char tmp[256];\n"
              "free (tmp);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (error) Deallocating auto-variable is invalid\n"), errout.str());

        check("void f()\n"
              "{\n"
              "    char psz_title[10];\n"
              "    {\n"
              "        char *psz_title = 0;\n"
              "        abc(0, psz_title);\n"
              "        free(psz_title);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void testassign1() { // Ticket #1819
        check("void f(EventPtr *eventP, ActionPtr **actionsP) {\n"
              "    EventPtr event = *eventP;\n"
              "    *actionsP = &event->actions;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testassign2() { // Ticket #2765
        check("static void function(unsigned long **datap) {\n"
              "    struct my_s *mr = global_structure_pointer;\n"
              "    *datap = &mr->value;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void returnLocalVariable1() {
        check("char *foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Returning pointer to local array variable\n", errout.str());

        check("class Fred {\n"
              "    char *foo();\n"
              "};\n"
              "char *Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Returning pointer to local array variable\n", errout.str());
    }

    void returnLocalVariable2() {
        check("std::string foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    std::string foo();\n"
              "};\n"
              "std::string Fred::foo()\n"
              "{\n"
              "    char str[100] = {0};\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference1() {
        check("std::string &foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Returning reference to auto variable\n", errout.str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Returning reference to auto variable\n", errout.str());

        check("std::vector<int> &foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "std::string &f()\n"
              "{\n"
              "    return hello();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Returning reference to temporary\n", errout.str());
    }

    void returnReference2() {
        check("class Fred {\n"
              "    std::string &foo();\n"
              "}\n"
              "std::string &Fred::foo()\n"
              "{\n"
              "    std::string s;\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Returning reference to auto variable\n", errout.str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Returning reference to auto variable\n", errout.str());

        check("class Fred {\n"
              "    std::vector<int> &foo();\n"
              "};\n"
              "std::vector<int> &Fred::foo()\n"
              "{\n"
              "    static std::vector<int> v;\n"
              "    return v;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("class Fred {\n"
              "    std::string &f();\n"
              "};\n"
              "std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Returning reference to temporary\n", errout.str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    std::string &f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "std::string &Fred::f()\n"
              "{\n"
              "    return hello();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (error) Returning reference to temporary\n", errout.str());
    }

    void returnReference3() {
        check("double & f(double & rd) {\n"
              "    double ret = getValue();\n"
              "    rd = ret;\n"
              "    return rd;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    // Returning reference to global variable
    void returnReference4() {
        check("double a;\n"
              "double & f() {\n"
              "    double & ref = a;\n"
              "    return ref;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void returnReference5() {
        check("struct A {\n"
              "    int i;\n"
              "};\n"

              "struct B {\n"
              "    A a;\n"
              "};\n"

              "struct C {\n"
              "    B *b;\n"
              "    const A& a() const {\n"
              "        const B *pb = b;\n"
              "        const A &ra = pb->a;\n"
              "        return ra;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void returncstr() {
        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "const char *f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (error) Returning pointer to temporary\n", errout.str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    const char *f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "const char *Fred::f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (error) Returning pointer to temporary\n", errout.str());
    }


    void testglobalnamespace() {
        check("class SharedPtrHolder\n"
              "{\n"
              "   ::std::tr1::shared_ptr<int> pNum;\n"
              "public:\n"
              "   void SetNum(const ::std::tr1::shared_ptr<int> & apNum)\n"
              "   {\n"
              "      pNum = apNum;\n"
              "   }\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void returnParameterAddress() {
        check("int* foo(int y)\n"
              "{\n"
              "  return &y;\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:3]: (error) Return the address of function parameter 'y'\n", errout.str());

        check("int ** foo(int * y)\n"
              "{\n"
              "  return &y;\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:3]: (error) Return the address of function parameter 'y'\n", errout.str());

        check("const int * foo(const int & y)\n"
              "{\n"
              "  return &y;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestAutoVariables)


