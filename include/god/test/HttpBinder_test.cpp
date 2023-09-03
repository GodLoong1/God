#include "god/http/HttpBinder.h"
#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"
#include "god/utils/Logger.h"
#include <deque>
#include <iostream>
#include <type_traits>

using namespace god;

void testFunc1(const HttpRequestPtr&, HttpResponseHandler&&, const std::string& name, double age)
{
    std::cout << "testFunc1: {" << name << ":" << age << "}" << std::endl;
}

[[maybe_unused]]
auto testFunc2 = [](const HttpRequestPtr&, HttpResponseHandler&&, const std::string& name, int age)
{
    std::cout << "testFunc2: {" << name << ":" << age << "}" << std::endl;
};

std::function<void(const HttpRequestPtr&, HttpResponseHandler&&, const std::string&, int)> testFunc3 = testFunc2;

void test(const HttpBinderBasePtr& binder, std::deque<std::string>& params)
{
    // std::cout << binder->handlerName() << std::endl;
    std::cout << binder->paramCount() << std::endl;
    binder->handleHttpRequest(nullptr, nullptr, params);
}

int main()
{
    std::deque<std::string> params{"小龙", "18.123", "爸爸", "29", "卧槽", "88"};

    auto binder1 = makeHttpBinder(&testFunc1);
    auto binder2 = makeHttpBinder(std::move(testFunc2));
    auto binder3 = makeHttpBinder(std::move(testFunc3));

    test(binder1, params);
    test(binder2, params);
    test(binder3, params);
}